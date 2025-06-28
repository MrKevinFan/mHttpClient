cmake_minimum_required(VERSION 3.19)
project(mHttpClient VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lws2_32")
endif()

set(MHTTPCLIENT_SOURCES
  src/http_client.c
)

# 主库
add_library(mHttpClient SHARED src/http_get_params.c)
# 主程序
#add_executable(mJson)
target_sources(mHttpClient PRIVATE ${MHTTPCLIENT_SOURCES})
target_include_directories(mHttpClient PRIVATE include src)
if(WIN32)
    target_link_libraries(mHttpClient ws2_32)
endif()
# 示例程序
add_executable(example example/main.c include/http_get_params.h)
target_include_directories(example PRIVATE include src)
target_link_libraries(example mHttpClient)

# 安装配置
install(TARGETS mHttpClient
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        )

install(DIRECTORY include/
        DESTINATION include
        FILES_MATCHING PATTERN "*.h"
        )

# 测试配置
if(BUILD_TESTING)
    enable_testing()
    add_executable(test_http_client tests/test_http.c)
    target_link_libraries(test_http_client mHttpClient)
    add_test(NAME http_client_test COMMAND test_http_client)
endif()

# 打包配置
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
