set(NAME udpsock)
set(SRC ${NAME})

add_library(${NAME} STATIC ${SRC})

target_include_directories(${NAME} PRIVATE ${CMAKE_SOURCE_DIR}/common)
