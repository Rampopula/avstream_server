#ifndef __DEMUXER_H__
#define __DEMUXER_H__

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "libavutil/imgutils.h"
#include "libavutil/samplefmt.h"
#include "libavutil/timestamp.h"
#include "libavformat/avformat.h"


typedef int32_t(*VFRAME_Proc)(uint8_t *frame, int32_t size);
typedef int32_t(*AFRAME_Proc)(uint8_t *frame, int32_t size);

typedef struct DemuxerCallbacks_s {
    VFRAME_Proc vframe_proc;
    AFRAME_Proc aframe_proc;
} DemuxerCallbacks_t;


int32_t demuxer_init(char *filepath, DemuxerCallbacks_t *user_callbacks);

int32_t demuxer_start(void);


#endif /* __DEMUXER_H__ */
