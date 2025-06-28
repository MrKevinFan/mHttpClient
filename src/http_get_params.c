//
// Created by fanmin on 2025-06-28.
//
# include "http_get_params.h"

HttpParams* create_params() {
    HttpParams* params = malloc(sizeof(HttpParams));
    params->params = NULL;
    params->count = 0;
    return params;
}

void add_param(HttpParams* params, const char* key, const char* value) {
    params->params = realloc(params->params,
                             (params->count + 1) * sizeof(HttpParam));
    params->params[params->count].key = strdup(key);
    params->params[params->count].value = strdup(value);
    params->count++;
}

void free_params(HttpParams* params) {
    for(int i = 0; i < params->count; i++) {
        free(params->params[i].key);
        free(params->params[i].value);
    }
    free(params->params);
    free(params);
}

// URL编码辅助函数
char* url_encode(const char* str) {
    if(!str) return NULL;
    const char hex[] = "0123456789ABCDEF";
    size_t len = strlen(str);
    char* encoded = malloc(len * 3 + 1); // 最坏情况下每个字符变成%XX

    if(!encoded) return NULL;

    char* p = encoded;
    for(size_t i = 0; i < len; i++) {
        unsigned char c = str[i];

        // 保留字符不编码 (RFC 3986)
        if(isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            *p++ = c;
        }
            // 空格转为+
        else if(c == ' ') {
            *p++ = '+';
        }
        // 其他字符转为%XX形式
        else {
            *p++ = '%';
            *p++ = hex[c >> 4];
            *p++ = hex[c & 0x0F];
        }
    }

    *p = '\0';
    return encoded;
}
/**
 * 通过对象构建
 * @param base_url
 * @param params
 * @return
 */
char* build_object_params_get_url(const char* base_url, HttpParams* params) {
    if(!base_url) base_url = "";
    // 计算基础URL长度
    size_t url_len = strlen(base_url);
    // 计算查询参数部分长度
    size_t query_len = 0;
    if(params && params->count > 0) {
        for(int i = 0; i < params->count; i++) {
            char* encoded_key = url_encode(params->params[i].key);
            char* encoded_val = url_encode(params->params[i].value);
            query_len += strlen(encoded_key) + strlen(encoded_val) + 1;
            free(encoded_key);
            free(encoded_val);
        }
        query_len += params->count - 1; // for '&' between params
    }
    // 分配内存并构建完整URL
    char* full_url = malloc(url_len + query_len + 2); // +2 for '?' and '\0'
    strcpy(full_url, base_url);
    if(params && params->count > 0) {
        char* p = full_url + url_len;
        *p++ = '?';
        for(int i = 0; i < params->count; i++) {
            if(i > 0) *p++ = '&';
            char* encoded_key = url_encode(params->params[i].key);
            char* encoded_val = url_encode(params->params[i].value);
            strcpy(p, encoded_key);
            p += strlen(encoded_key);
            *p++ = '=';
            strcpy(p, encoded_val);
            p += strlen(encoded_val);
            free(encoded_key);
            free(encoded_val);
        }
        *p = '\0';
    }
    return full_url;
}

// 构造带参数的GET请求URL
char* build_array_params_get_url(const char* base_url, const char** params, int param_count) {
    if(!base_url || !params || param_count <= 0) return NULL;

    // 计算总长度
    size_t total_len = strlen(base_url) + 1; // 基础URL + '?'
    for(int i = 0; i < param_count; i += 2) {
        if(params[i] && params[i+1]) {
            total_len += strlen(params[i]) + strlen(params[i+1]) + 3; // key=value&
        }
    }

    char* url = malloc(total_len);
    if(!url) return NULL;

    strcpy(url, base_url);
    strcat(url, "?");

    for(int i = 0; i < param_count; i += 2) {
        if(params[i] && params[i+1]) {
            char* encoded_val = url_encode(params[i+1]);
            if(encoded_val) {
                if(i > 0) strcat(url, "&");
                strcat(url, params[i]);
                strcat(url, "=");
                strcat(url, encoded_val);
                free(encoded_val);
            }
        }
    }

    return url;
}