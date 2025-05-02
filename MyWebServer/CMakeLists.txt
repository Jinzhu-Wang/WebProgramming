
cmake_minimum_required(VERSION 3.10)
project(WebServer)

set(CMAKE_CXX_FLAGS "-g -Wall -Werror -std=c++14")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/)

include_directories(
    base
    tcp
    log
    http
    timer
    CGI
)

# 多线程
find_package(Threads REQUIRED)

#MySQL
execute_process(COMMAND mysql_config --cflags OUTPUT_VARIABLE MYSQL_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND mysql_config --libs OUTPUT_VARIABLE MYSQL_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
separate_arguments(MYSQL_CFLAGS)
separate_arguments(MYSQL_LIBS)
include_directories(${MYSQL_CFLAGS})

# libjsoncpp
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)
include_directories(${JSONCPP_INCLUDE_DIRS})

aux_source_directory(base SRC_LIST1)
aux_source_directory(tcp SRC_LIST2)
aux_source_directory(http SRC_LIST3)
aux_source_directory(timer SRC_LIST4)
aux_source_directory(log SRC_LIST5)
aux_source_directory(CGI SRC_LIST6)


add_executable(webserver test/http_server.cpp ${SRC_LIST1} ${SRC_LIST2} ${SRC_LIST3} ${SRC_LIST4} ${SRC_LIST5} ${SRC_LIST6})
# add_executable(test_logstream test/test_logstream.cpp ${SRC_LIST5})
# add_executable(echoserver test/echo_server.cpp ${SRC_LIST1} ${SRC_LIST2} )
# add_executable(test_context test/test_httpcontext.cpp ${SRC_LIST3} )
# target_link_libraries(echoserver ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(webserver ${CMAKE_THREAD_LIBS_INIT})

# 设置libjson连接
target_link_libraries(webserver ${JSONCPP_LIBRARIES})
target_link_libraries(webserver ${MYSQL_LIBS})
