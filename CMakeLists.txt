cmake_minimum_required(VERSION 3.0.0)
project(todolist)

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

add_executable(todolist main.c calculator.c functions.c settings/bdd.c)
target_link_libraries(todolist ${GTK3_LIBRARIES} m ${PostgreSQL_LIBRARIES} m ${CURL_LIBRARIES})
