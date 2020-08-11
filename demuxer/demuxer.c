#include "demuxer.h"

#define TRUE                (1)
#define FALSE               (0)
#define LOG(fmt, args...)   do {\
                                fprintf(stdout, "%s-%d: " fmt, __FUNCTION__, __LINE__, ##args);\
                            } while(0)


static AVPacket pkt;
static AVFormatContext *fmt_ctx = NULL;
static int32_t vstream_idx = -1, astream_idx = -1;
static int32_t inited = FALSE;
static DemuxerCallbacks_t callbacks;


static int32_t get_vstream_idx(void) {

    vstream_idx = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (vstream_idx < 0) {
        LOG("Could not find video stream in input file\n");
        return EXIT_FAILURE;
    } 

    return EXIT_SUCCESS;
}

static int32_t get_astream_idx(void) {

    astream_idx = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (astream_idx < 0) {
        LOG("Could not find video stream in input file\n");
        return EXIT_FAILURE;
    } 

    return EXIT_SUCCESS;
}


int32_t demuxer_init(char *filepath, DemuxerCallbacks_t *user_callbacks) {

    if (filepath == NULL) {
        LOG("File path is null!\n");
        return EXIT_FAILURE;
    }

    if (user_callbacks == NULL) {
        LOG("Callbacks is null!\n");
        return EXIT_FAILURE;
    }

    if (inited) {
        LOG("Already inited!\n");
        return EXIT_SUCCESS;
    }

    // save callbacks
    memcpy((void*)&callbacks, (void*)user_callbacks, sizeof(DemuxerCallbacks_t));

    // register all formats and codecs
    av_register_all();

    // open input file, and allocate format context
    if (avformat_open_input(&fmt_ctx, filepath, NULL, NULL) < 0) {
        LOG("Could not open source file %s\n", filepath);
        return EXIT_FAILURE;
    }

    // retrieve stream information
    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
        LOG("Could not find stream information\n");
        return EXIT_FAILURE;
    }

    if (get_vstream_idx() || get_astream_idx()) {
        LOG("Can't get stream index!\n");
        return EXIT_FAILURE;
    }

    // initialize packet, set data to NULL, let the demuxer fill it
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    inited = TRUE;
    return EXIT_SUCCESS;
}

int32_t demuxer_start(void) {

    if (!inited){
        LOG("Not inited!\n");
        return EXIT_FAILURE;
    }

    while (av_read_frame(fmt_ctx, &pkt) >= 0) {
        AVPacket orig_pkt = pkt;

        if (pkt.stream_index == vstream_idx) {
            // Video packet
            if (callbacks.vframe_proc) {
                callbacks.vframe_proc(pkt.data, pkt.size);
            }
        } 
        else if (pkt.stream_index == astream_idx) {
            // Audio packet
            if (callbacks.aframe_proc) {
                callbacks.aframe_proc(pkt.data, pkt.size);
            }
        }
        av_packet_unref(&orig_pkt);
    }

    return EXIT_SUCCESS;
}
