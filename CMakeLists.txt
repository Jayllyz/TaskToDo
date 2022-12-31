cmake_minimum_required(VERSION 3.0.0)
project(TaskTodo VERSION 0.1.0)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-Wall")

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER})

find_package(PostgreSQL REQUIRED)
include_directories (${PostgreSQL_INCLUDE_DIRS})
link_directories(${PostgreSQL_LIBRARY_DIRS})
add_definitions(${PostgreSQL_DEFINITIONS})

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIR})
link_directories(${CURL_LIBRARY_DIRS})
add_definitions(${CURL_DEFINITIONS})

file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY settings DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

find_program(CLANG_FORMAT "clang-format")
if(CLANG_FORMAT)
    add_custom_target(
        format
        COMMENT "Running clang-format to change files"
        COMMAND ${CLANG_FORMAT} -i -style=file ${CMAKE_CURRENT_SOURCE_DIR}/*.c
        COMMAND ${CLANG_FORMAT} -i -style=file ${CMAKE_CURRENT_SOURCE_DIR}/*.h
        COMMAND ${CLANG_FORMAT} -i -style=file ${CMAKE_CURRENT_SOURCE_DIR}/settings/*.c
    )
endif()

add_executable(TaskTodo main.c calculator.c functions.c settings/bdd.c)
target_link_libraries(TaskTodo ${GTK3_LIBRARIES} m ${PostgreSQL_LIBRARIES} m ${CURL_LIBRARIES})
