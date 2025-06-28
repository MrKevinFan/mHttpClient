//
// Created by fanmin on 2025-06-28.
//

#ifndef MHTTPCLIENT_HTTP_GET_PARAMS_H
#define MHTTPCLIENT_HTTP_GET_PARAMS_H
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
typedef struct {
    char* key;
    char* value;
} HttpParam;

typedef struct {
    HttpParam* params;
    int count;
} HttpParams;

HttpParams* create_params();
void add_param(HttpParams* params, const char* key, const char* value);
void free_params(HttpParams* params);

char* build_object_params_get_url(const char* base_url, HttpParams* params);
char* build_array_params_get_url(const char* base_url, const char** params, int param_count);

#endif //MHTTPCLIENT_HTTP_GET_PARAMS_H
