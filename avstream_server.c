#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "udpsock.h"
#include "demuxer.h"

#define PACKET_DATA_OFFFSET			(8)
#define PACKET_BUF_SIZE				(1024*256)	//(32768)
#define PACKET_TYPE_AVC				"AVC"
#define PACKET_TYPE_AAC				"AAC"

// #define FILE_DEBUG
#ifdef FILE_DEBUG
	#define MP4_IN_PATH					"/home/rampopula/vscode/avstream_server/build/artem.mp4"
	#define H264_OUT_PATH				"test.h264"
	#define AAC_OUT_PATH				"test.aac"
#endif


static uint8_t packet_buf[PACKET_BUF_SIZE];


int32_t wrap_packet(uint8_t *packet, char *type, uint8_t chn_id, int32_t size) {

	uint8_t *ptr = packet_buf;
	
	// Packet structure
	// char head[3]; 		AVC/AAC																					
	memcpy((void*)ptr, (void*)type, strlen(type));
	ptr += strlen(type);

	// uint8_t chn_id;		0/1												
	memcpy((void*)ptr, (void*)&chn_id, sizeof(uint8_t));
	ptr += sizeof(uint8_t);

	// int32_t size;
	memcpy((void*)ptr, (void*)&size, sizeof(int32_t));	

	// uint8_t data[size];
	// Already in packet

	return EXIT_SUCCESS;
}

int32_t vframe_proc(uint8_t *frame, int32_t size) {

	#ifdef FILE_DEBUG
		FILE *file = fopen("test.h264", "a+b");
		fwrite(frame, 1, size, file);
		fclose(file);
	#else

		LOG("callback ---> send video frame, size: %d b\n", size);

		uint8_t chn_id = 0;
		char *type = PACKET_TYPE_AVC;

		memcpy((void*)packet_buf + PACKET_DATA_OFFFSET, (void*)frame, size);
		wrap_packet(packet_buf, type, chn_id, size);
		udpsock_send(packet_buf, size + PACKET_DATA_OFFFSET);	
	#endif

    return EXIT_SUCCESS;
}

int32_t aframe_proc(uint8_t *frame, int32_t size) {

	uint8_t adts_header[7];

	#ifndef FILE_DEBUG
		uint8_t chn_id = 0;
		char *type = PACKET_TYPE_AAC;
	#endif


	size += 7;

	/* Sync point over a full byte */
	adts_header[0] = 0xFF;

	/* Sync point continued over first 4 bits + static 4 bits * (ID, layer, protection)*/
	adts_header[1] = 0xF1;

	/* Object type over first 2 bits */	
	uint8_t obj_type = 0x01;
	adts_header[2] = obj_type << 6;

	/* rate index over next 4 bits */
	uint8_t rate_idx = 0x08;
	adts_header[2] |= (rate_idx << 2);

	/* channels over last 2 bits */
	uint8_t channels = 0x02;
	adts_header[2] |= (channels & 0x4) >> 2;

	/* channels continued over next 2 bits + 4 bits at zero */
	adts_header[3] = (channels & 0x3) << 6;

	/* frame size over last 2 bits */
	uint32_t frame_length = size ;
	adts_header[3] |= (frame_length & 0x1800) >> 11;

	/* frame size continued over full byte */
	adts_header[4] = (frame_length & 0x1FF8) >> 3;

	/* frame size continued first 3 bits */
	adts_header[5] = (frame_length & 0x7) << 5;

	/* buffer fullness (0x7FF for VBR) over 5 last bits*/
	adts_header[5] |= 0x1F;

	/* buffer fullness (0x7FF for VBR) continued over 6 first bits + 2 zeros * number of raw data blocks */
	adts_header[6] = 0xFC;//one raw data blocks.

	//Set raw Data blocks.
	// adts_header[6] |= num_data_block & 0x03;

	#ifdef FILE_DEBUG
		FILE *file = fopen("test.aac", "a+b");
		fwrite(adts_header, 1, 7, file);
		fwrite(frame, 1, size - 7, file);
		fclose(file);
	#else
		LOG("callback ---> send audio frame, size: %d b\n", size);

		memcpy((void*)packet_buf + PACKET_DATA_OFFFSET, (void*)adts_header, sizeof(adts_header));
		memcpy((void*)packet_buf + PACKET_DATA_OFFFSET + 7, (void*)frame, size - 7);
		wrap_packet(packet_buf, type, chn_id, size);
		udpsock_send(packet_buf, size + PACKET_DATA_OFFFSET);		
	#endif

    return EXIT_SUCCESS;
}


int main(int argc, char *argv[]) {

    int32_t ret;
    char filepath[128];

    // Get input file name
    int32_t opt = getopt(argc, argv, "i:");
	if (opt != -1 && opt == 'i') {
        snprintf(filepath, 32, "%s", optarg);
	}
    else {
		#ifdef FILE_DEBUG
			snprintf(filepath, 128, "%s", MP4_IN_PATH);
		#else
			printf("Usage: avserver -i sample.mp4\n");
			return EXIT_FAILURE;
		#endif
    }

	// Open udp socket
	if (udpsock_open() != 0) {
		LOG("Can't open socket!\n");
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
