set(NAME demuxer)
set(SRC ${NAME})

add_library(${NAME} STATIC ${SRC})

### FFMPEG
find_path(AVFORMAT_INCLUDE_DIR libavformat/avformat.h)
find_library(AVFORMAT_LIBRARY avformat)

find_path(AVCODEC_INCLUDE_DIR libavcodec/avcodec.h)
find_library(AVCODEC_LIBRARY avcodec)

find_path(SWRESAMPLE_INCLUDE_DIR libswresample/swresample.h)
find_library(SWRESAMPLE_LIBRARY swresample)

find_path(AVUTIL_INCLUDE_DIR libavutil/avutil.h)
find_library(AVUTIL_LIBRARY avutil)

target_include_directories(${NAME} PRIVATE ${AVFORMAT_INCLUDE_DIR} ${AVCODEC_INCLUDE_DIR} ${SWRESAMPLE_INCLUDE_DIR} ${AVUTIL_INCLUDE_DIR})
target_link_libraries(${NAME} PRIVATE ${AVFORMAT_LIBRARY} ${AVCODEC_LIBRARY} ${SWRESAMPLE_LIBRARY} ${AVUTIL_LIBRARY})	
#################################################################################

target_include_directories(${NAME} PRIVATE ${CMAKE_SOURCE_DIR}/common)
target_link_libraries(${NAME} PRIVATE pthread m z)
