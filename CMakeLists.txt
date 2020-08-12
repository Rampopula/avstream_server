cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

# Application versino
set(PROJECT_NAME AVStreamServer)
project(${PROJECT_NAME} VERSION "1.0.0")

# Compiler
add_compile_options(-c -fmessage-length=0 -Wall -Wcomment)

# Libraries
add_subdirectory(udpsock)
add_subdirectory(demuxer)

# Build application
set(NAME avstream_server)
set(SRC avstream_server)
add_executable(${NAME} ${SRC})

target_include_directories(${NAME} PRIVATE common udpsock demuxer)
target_link_directories(${NAME} PRIVATE udpsock demuxer)
target_link_libraries(${NAME} PRIVATE udpsock demuxer)
