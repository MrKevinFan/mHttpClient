
#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "http_get_params.h"
#ifdef _WIN32
#include <windows.h>
#define strncasecmp _strnicmp
#else
#include <unistd.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define closesocket close
#endif

typedef enum {
    HTTP_GET,
    HTTP_POST,
    HTTP_PUT,
    HTTP_DELETE
} http_method_t;

typedef struct {
    int status_code;
    char* headers;
    char* body;
    size_t body_len;
} http_response_t;

typedef void (*http_callback_t)(http_response_t*, void*);

#ifdef _WIN32
#include <winsock2.h>
typedef SOCKET socket_t;
#else
typedef int socket_t;
#endif
http_response_t* http_get_json_sync(const char* url);
http_response_t* http_get_object_params_json_sync(const char* url,HttpParams* params);
http_response_t* http_get_array_params_json_sync(const char* url,const char** params, int param_count);
http_response_t* http_get_sync(const char* url, const char* headers);


http_response_t* http_post_form_sync(const char* url,const char* body);
http_response_t* http_post_json_sync(const char* url,const char* body);
http_response_t* http_post_sync(const char* url, const char* headers, const char* body);

// 同步请求
http_response_t* http_request_sync(const char* url, http_method_t method,const char* headers, const char* body);

// GET请求封装
void http_get_async(const char* url,http_callback_t callback,void* user_data) ;

// POST请求封装(JSON格式)
void http_post_json_async(const char* url,const char* json_body,http_callback_t callback,void* user_data) ;

// POST请求封装(表单格式)
void http_post_form_async(const char* url,const char* form_data,http_callback_t callback,void* user_data);

// 异步请求
void http_request_async(const char* url, http_method_t method,
                        const char* headers, const char* body,
                        http_callback_t callback, void* user_data);

// 全局初始化/清理
int http_client_init();
void http_client_cleanup();
//释放响应资源
void http_free_response(http_response_t* res);
#endif
