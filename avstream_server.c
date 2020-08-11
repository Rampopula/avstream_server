#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "udpsock.h"
#include "demuxer.h"


int32_t vframe_proc(uint8_t *frame, int32_t size) {
    
    printf("callback ---> video frame, size: %3.2f kb\n", size / 1024.f);
 
    return EXIT_SUCCESS;
}

int32_t aframe_proc(uint8_t *frame, int32_t size) {

    printf("callback ---> audio frame, size: %3.2f kb\n", size / 1024.f);

    return EXIT_SUCCESS;
}


int main(int argc, char *argv[]) {

    int32_t ret;
    char filepath[32];

    // Get input file name
    int32_t opt = getopt(argc, argv, "i:");
	if (opt != -1 && opt == 'i') {
        snprintf(filepath, 32, "%s", optarg);
	}
    else {
        printf("Usage: avserver -i sample.mp4\n");
        return EXIT_FAILURE;
    }

    // Initialize demuxer
    DemuxerCallbacks_t callbacks;
    callbacks.vframe_proc = &vframe_proc;
    callbacks.aframe_proc = &aframe_proc;

    ret = demuxer_init(filepath, &callbacks);
    if (ret != 0) {
        return EXIT_FAILURE;
    }

    // Start demuxer
    demuxer_start();

    return EXIT_SUCCESS;
}
