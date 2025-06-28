#include "http_client.h"


#define DEFAULT_PORT 80
#define BUFFER_SIZE 4096
#define MAX_REDIRECTS 5

typedef struct {
    char* host;
    char* path;
    int port;
} url_components_t;

// 使用GCC的构造函数特性,代码会在其他函数之前执行，适用于初始化
__attribute__((constructor))
int http_client_init() {
#ifdef _WIN32
    WSADATA wsa;
    return WSAStartup(MAKEWORD(2,2), &wsa);
#else
    return 0;
#endif
}

// 使用GCC的析构函数特性,代码会在其他函数之后执行，适用于释放资源
__attribute__((destructor))
void http_client_cleanup() {
#ifdef _WIN32
    WSACleanup();
#endif
}

int parse_url(const char* url, url_components_t* comp) {
    const char *p = url;
    char *host_start;

    if (strncasecmp(url, "http://", 7) == 0) {
        p += 7;
    } else if (strncasecmp(url, "https://", 8) == 0) {
        return -1; // HTTPS not supported
    }

    host_start = (char*)p;
    while (*p && *p != ':' && *p != '/' && *p != '?') p++;

    size_t host_len = p - host_start;
    comp->host = malloc(host_len + 1);
    strncpy(comp->host, host_start, host_len);
    comp->host[host_len] = '\0';

    if (*p == ':') {
        p++;
        comp->port = atoi(p);
        while (isdigit(*p)) p++;
    } else {
        comp->port = DEFAULT_PORT;
    }

    if (*p == '\0') {
        comp->path = strdup("/");
    } else {
        comp->path = strdup(p);
    }

    return 0;
}

socket_t connect_to_host(const char* host, int port) {
    struct sockaddr_in server_addr;
    struct hostent *he;
    socket_t sockfd;

    if ((he = gethostbyname(host)) == NULL) {
        return -1;
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr = *((struct in_addr*)he->h_addr);
    memset(&(server_addr.sin_zero), '\0', 8);

    if (connect(sockfd, (struct sockaddr*)&server_addr,
                sizeof(struct sockaddr)) == -1) {
        closesocket(sockfd);
        return -1;
    }

    return sockfd;
}

http_response_t* parse_response(const char* response, size_t len) {
    http_response_t* res = malloc(sizeof(http_response_t));
    memset(res, 0, sizeof(http_response_t));

    const char* p = response;
    const char* end = response + len;

    // Parse status line
    if (strncmp(p, "HTTP/", 5) != 0) {
        free(res);
        return NULL;
    }
    p += 5;

    // Skip HTTP version
    while (*p && *p != ' ') p++;
    if (*p == ' ') p++;

    res->status_code = atoi(p);
    while (*p && *p != '\r' && *p != '\n') p++;

    // Parse headers
    const char* headers_start = p;
    while (*p && p < end) {
        if (*p == '\r' && *(p+1) == '\n' && (*(p+2) == '\r' || *(p+2) == '\n')) {
            break;
        }
        p++;
    }

    size_t headers_len = p - headers_start;
    res->headers = malloc(headers_len + 1);
    strncpy(res->headers, headers_start, headers_len);
    res->headers[headers_len] = '\0';

    // Skip CRLF
    while (*p && (*p == '\r' || *p == '\n')) p++;

    // Body
    res->body_len = end - p;
    res->body = malloc(res->body_len + 1);
    memcpy(res->body, p, res->body_len);
    res->body[res->body_len] = '\0';

    return res;
}

void http_free_response(http_response_t* res) {
    if (res) {
        free(res->headers);
        free(res->body);
        free(res);
    }
}

const char* method_to_str(http_method_t method) {
    switch (method) {
        case HTTP_GET: return "GET";
        case HTTP_POST: return "POST";
        case HTTP_PUT: return "PUT";
        case HTTP_DELETE: return "DELETE";
        default: return "GET";
    }
}

http_response_t* http_get_object_params_json_sync(const char* url,HttpParams* params) {
    char* full_url = build_object_params_get_url(url, params);
    return http_get_sync(full_url, "Accept: application/json");
}

http_response_t* http_get_array_params_json_sync(const char* url,const char** params, int param_count) {
    char* full_url = build_array_params_get_url(url, params, param_count);
    return http_get_sync(full_url, "Accept: application/json");
}

http_response_t* http_get_json_sync(const char* url) {
    return http_get_sync(url, "Accept: application/json");
}

http_response_t* http_get_sync(const char* url, const char* headers) {
    // GET请求无需body和headers，POST请求自动设置正确的Content-Type。
    return http_request_sync(url, HTTP_GET, headers, NULL);
}

http_response_t* http_post_form_sync(const char* url,const char* body) {
    return http_post_sync(url, "Content-Type: application/x-www-form-urlencoded",body);
}

http_response_t* http_post_json_sync(const char* url,const char* body) {
    return http_post_sync(url, "Content-Type: application/json",body);
}

http_response_t* http_post_sync(const char* url, const char* headers, const char* body) {
    return http_request_sync(url, HTTP_POST, headers, body);
}

http_response_t* http_request_sync(const char* url, http_method_t method,
                                   const char* headers, const char* body) {
    url_components_t comp;
    if (parse_url(url, &comp) != 0) {
        return NULL;
    }

    socket_t sockfd = connect_to_host(comp.host, comp.port);
    if (sockfd == -1) {
        free(comp.host);
        free(comp.path);
        return NULL;
    }

    char request[BUFFER_SIZE];
    int len = snprintf(request, BUFFER_SIZE,
                       "%s %s HTTP/1.1\r\n"
                       "Host: %s\r\n"
                       "Connection: close\r\n",
                       method_to_str(method), comp.path, comp.host);

    if (headers) {
        len += snprintf(request + len, BUFFER_SIZE - len, "%s\r\n", headers);
    }

    if (body) {
        len += snprintf(request + len, BUFFER_SIZE - len,
                        "Content-Length: %zu\r\n\r\n%s",
                        strlen(body), body);
    } else {
        len += snprintf(request + len, BUFFER_SIZE - len, "\r\n");
    }

    send(sockfd, request, len, 0);

    char response[BUFFER_SIZE];
    int total = 0;
    int n;

    while ((n = recv(sockfd, response + total, BUFFER_SIZE - total - 1, 0)) > 0) {
        total += n;
    }

    closesocket(sockfd);
    free(comp.host);
    free(comp.path);

    if (total == 0) {
        return NULL;
    }

    response[total] = '\0';
    return parse_response(response, total);
}

typedef struct {
    http_callback_t callback;
    void* user_data;
    char* url;
    http_method_t method;
    char* headers;
    char* body;
} async_request_t;

#ifdef _WIN32
DWORD WINAPI async_thread(LPVOID arg) {
#else
void* async_thread(void* arg) {
#endif
    async_request_t* req = (async_request_t*)arg;
    http_response_t* res = http_request_sync(req->url, req->method,req->headers, req->body);
    if (req->callback) {
    req->callback(res, req->user_data);
    }
    http_free_response(res);
    free(req->url);
    free(req->headers);
    free(req->body);
    free(req);
    return 0;
}



// GET请求封装
void http_get_async(const char* url,
                    http_callback_t callback,
                    void* user_data) {
    http_request_async(url, HTTP_GET, NULL, NULL, callback, user_data);
}

// POST请求封装(JSON格式)
void http_post_json_async(const char* url,
                          const char* json_body,
                          http_callback_t callback,
                          void* user_data) {
    const char* headers = "Content-Type: application/json";
    http_request_async(url, HTTP_POST, headers, json_body, callback, user_data);
}

// POST请求封装(FORM格式)
void http_post_form_async(const char* url,
                          const char* form_data,
                          http_callback_t callback,
                          void* user_data) {
    const char* headers = "Content-Type: application/x-www-form-urlencoded";
    http_request_async(url, HTTP_POST, headers, form_data, callback, user_data);
}

void http_request_async(const char* url, http_method_t method,
                        const char* headers, const char* body,
                        http_callback_t callback, void* user_data) {
    async_request_t* req = malloc(sizeof(async_request_t));
    req->callback = callback;
    req->user_data = user_data;
    req->url = strdup(url);
    req->method = method;
    req->headers = headers ? strdup(headers) : NULL;
    req->body = body ? strdup(body) : NULL;

#ifdef _WIN32
    CreateThread(NULL, 0, async_thread, req, 0, NULL);
#else
    pthread_t thread;
    pthread_create(&thread, NULL, async_thread, req);
    pthread_detach(thread);
#endif
}
