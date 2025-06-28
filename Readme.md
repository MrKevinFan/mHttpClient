
(本人菜鸡一枚，欢迎大佬么指正！)

### mHttpClient说明

 mHttpClient 是一个轻量级的 http 客户端，用于向 http 服务器发送GET、POST请求。



### mHttpClient使用

http_get_params.h 和 http_get_params.c 主要是用于构建get带参请求的 url，可以参考示例：

```
// 示例2: 带数组参数的GET请求
void http_get_array_example(){
    const char* base_url = "http://192.168.1.32:10085/api/execCmd";
    const char* params[] = {
            "value", "input keyevent 24",
            "timeout", "10",
    };
    http_response_t* resp_get_params = http_get_array_params_json_sync(base_url,params, sizeof(params)/sizeof(params[0]));
    printResult(resp_get_params);
}
// 示例3: 带 HttpParam 对象参数的GET请求
void http_get_object_example(){
    const char* base_url = "http://192.168.1.32:10085/api/execCmd";
    // 初始化参数结构
    HttpParams params = {
            .params = (HttpParam[]){
                    {"value", "input keyevent 24"},
                    {"timeout", "10"},
            },
            .count = 2
    };
    http_response_t* resp_get_params = http_get_object_params_json_sync(base_url,&params);
    printResult(resp_get_params);
}
```

不需要的可以注释掉相应的方法，然后引用http_client.h和http_client.c即可。当前主要提供了同步、异步的GET、POST请求，暂时不支持HPPS请求。







### mHttpClient示例

下面是一些简单的使用示例，包含同步和异步发起GET、POST请求。

```
#include <stdio.h>
#include "http_client.h"

void sleep(unsigned int milliseconds) {
#ifdef _WIN32
    //    Windows使用Sleep()，单位毫秒
    Sleep(milliseconds);
#else
    //    Unix/Linux使用usleep()，单位微秒
    usleep(milliseconds * 1000);  // 将毫秒转换为微秒
#endif
}
void printResult(http_response_t* resp){
    if (resp) {
        printf("GET Response:\nStatus: %d\nBody: %.*s\n",
               resp->status_code,
               resp->body_len,
               resp->body);
        http_free_response(resp);
    }
}
// 示例1: 无参GET请求
void http_get_sync_example(){
    http_response_t* resp_get = http_get_json_sync("http://192.168.1.32:10085/api/home");
    printResult(resp_get);
}
// 示例2: 带数组参数的GET请求
void http_get_array_sync_example(){
    const char* base_url = "http://192.168.1.32:10085/api/execCmd";
    const char* params[] = {
            "value", "input keyevent 24",
            "timeout", "10",
    };
    http_response_t* resp_get_params = http_get_array_params_json_sync(base_url,params, 	sizeof(params)/sizeof(params[0]));
    printResult(resp_get_params);
}
// 示例3: 带 HttpParam 对象参数的GET请求
void http_get_object_sync_example(){
    const char* base_url = "http://192.168.1.32:10085/api/execCmd";
    // 初始化参数结构
    HttpParams params = {
            .params = (HttpParam[]){
                    {"value", "input keyevent 24"},
                    {"timeout", "10"},
            },
            .count = 2
    };
    http_response_t* resp_get_params = 			http_get_object_params_json_sync(base_url,&params);
    printResult(resp_get_params);
}

void http_post_json_sync_example(){
    const char* url = "http://192.168.1.32:10085/api/execCmd";
    const char* json_body = "{\"value\":\"input keyevent 24\",\"timeout\":10}";
    http_response_t* resp_post_params = http_post_json_sync(url,json_body);
    printResult(resp_post_params);
}

void http_post_form_sync_example(){
    const char* url = "http://192.168.1.32:10085/api/swipe";
    const char* form_body = "startX=500&startY=1500&endX=500&endY=500&duration=1000";
    http_response_t* resp_post_params = http_post_form_sync(url,form_body);
    printResult(resp_post_params);
}

// 回调函数处理响应
void response_callback(http_response_t* response, void* user_data) {
    printf("\n--- 请求完成 ---\n");
    printf("用户数据: %s\n", (char*)user_data);
    printf("状态码: %d\n", response->status_code);
    printf("响应体: %s\n", response->body);
    // 记得释放响应内存
    http_free_response(response);
}

void http_get_async_example(){
    http_get_async("http://192.168.1.32:10085/api/home",
                   response_callback,
                   "这是GET请求的用户数据");
}

void http_post_json_async_example(){
    const char* json_data = "{\"value\":\"input keyevent 24\",\"timeout\":10}";
    http_post_json_async("http://192.168.1.32:10085/api/execCmd",
                         json_data,
                         response_callback,
                         "这是JSON POST请求的用户数据");
}
void http_post_form_async_example(){
    const char* form_data = "startX=500&startY=1500&endX=500&endY=500&duration=1000";
    http_post_form_async("http://192.168.157.115:10085/api/swipe",
                         form_data,
                         response_callback,
                         "这是表单POST请求的用户数据");
}
int main() {
    // GET请求示例
    printf("发送请求...\n");
    http_post_form_async_example();

    // 主线程需要保持运行以等待回调
    // printf("\n等待异步请求完成...\n");
    while(1) {
        sleep(1);
    }
    return 0;
}
```



### mHttpClient版本说明

| 版本号       | 更新时间      | 更新描述 |
| ------------ | ------------- | -------- |
| V1.0.0-Alpha | 2025年6月28日 | 初版发布 |

