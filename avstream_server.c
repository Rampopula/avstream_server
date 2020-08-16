#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
 #include <fcntl.h> 

#include <getopt.h>
#include "udpsock.h"
#include "demuxer.h"

#define PACKET_DATA_OFFFSET			(8)
#define PACKET_BUF_SIZE				(1024*256)	//(32768)
#define PACKET_TYPE_AVC				"AVC"
#define PACKET_TYPE_AAC				"AAC"

// #define FILE_DEBUG
#ifdef FILE_DEBUG
	#define MP4_IN_PATH					"/home/rampopula/vscode/avstream_server/build/demo.mp4"
	#define H264_OUT_PATH				"test.h264"
	#define AAC_OUT_PATH				"test.aac"
	
	// #define DUMP_KEYFRAME
	#ifdef DUMP_KEYFRAME
		#define KEYFRAME_PATH			"keyframe"
	#endif
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

int32_t vframe_proc(uint8_t *frame, int32_t size, uint8_t keyframe) {

	#ifdef FILE_DEBUG
		#ifdef DUMP_KEYFRAME
			if (keyframe) {
				FILE *file = fopen(KEYFRAME_PATH, "a+b");
				fwrite(frame, 1, size, file);
				fclose(file);
                LOG("keyframe extracted: %s\n", KEYFRAME_PATH);
				while(1);
			}
		#else
			FILE *file = fopen(H264_OUT_PATH, "a+b");
			fwrite(frame, 1, size, file);
			fclose(file);
		#endif
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
		FILE *file = fopen(AAC_OUT_PATH, "a+b");
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


#define FF_JPEG_MAX_SIZE    (64 * 1024)     // 64 KB

typedef uint8_t*    keyframe_t;

keyframe_t open_keyframe(char *path, int32_t *size) {

    FILE *file = NULL;
    keyframe_t keyframe = NULL;

    // Open file
    file = fopen(path, "r+b");
    if (file == NULL) {
        LOG("Null pointer!\n");
        return NULL;
    }

    // Get file size
    fseek(file, 0L, SEEK_END);
    *size = ftell(file);
    fseek(file, 0L, SEEK_SET);

    // Allocate memory
    keyframe = malloc(*size);
    if (keyframe == NULL) {
        LOG("Null pointer!\n");
        fclose(file);
        return NULL;
    }

    // Read keyframe
    if (fread(keyframe, 1, *size, file) != *size) {
        LOG("Read error!\n");
        fclose(file);
        free(keyframe);
        return NULL;
    }

    return keyframe;
}

int32_t destroy_keyframe(keyframe_t keyframe) {

    if (keyframe != NULL) {
        free(keyframe);
    }

    return EXIT_SUCCESS;
}


typedef struct ImageJpeg_s {
    uint32_t width;          // if width & height > 0 then resulting image will be rescaled
    uint32_t height;
    uint32_t quality;        // quality factor from 2 to 31 (worst)
    int32_t size;
    uint8_t *data;
} ImageJpeg_t;

static uint8_t jpeg[FF_JPEG_MAX_SIZE];

int32_t ffmpeg_keyframe2jpeg(uint8_t *in_frame, int32_t in_size, ImageJpeg_t *jpeg_out, int32_t quiet_flag) {

    int ret;
    char pipe_name[32];
    char rescale[32];
    char quality[16];
    char command[128];
    FILE *ffmpeg = NULL;
    FILE *pipe = NULL;
    
    if (jpeg_out == NULL) {
        LOG("output buffer is null!\n");
        return EXIT_FAILURE;
    }

    // Clear output buffers
    jpeg_out->size = 0;
    jpeg_out->data = NULL;

    // Define pipe name
    snprintf(pipe_name, 32, "/tmp/ffpipe");

    // Create pipe
    remove(pipe_name);
    ret = mkfifo(pipe_name, O_RDWR | O_CREAT | 0666);
    if (ret == -1) {
        LOG("mkfifo error: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    // Open ffmpeg
    if (jpeg_out->width && jpeg_out->height) {
        LOG("JPEG rescale: %d x %d\n", jpeg_out->width, jpeg_out->height);
        snprintf(rescale, 32, "-vf scale=%d:%d", jpeg_out->width, jpeg_out->height);
    }
    else {
        snprintf(rescale, 32, " ");
    }

    if (jpeg_out->quality >= 2 && jpeg_out->quality <= 31) {
        LOG("JPEG quality factor: %d\n", jpeg_out->quality);
        snprintf(quality, 16, "-q:v %d", jpeg_out->quality);
    }
    else {
        snprintf(rescale, 16, " ");
    }
    
    snprintf(command, 128, "/usr/local/bin/ffmpeg -i pipe:0 %s -f image2 %s %s pipe: > %s", quiet_flag ? "-loglevel quiet" : "", rescale, quality, pipe_name);
    ffmpeg = popen(command, "w");
    if (ffmpeg == NULL) {
        LOG("popen() error: %s\n", strerror(errno));
        goto exit;
    }

    // Write keyframe to ffmpeg
    if (fwrite(in_frame, sizeof(uint8_t), in_size, ffmpeg) != in_size) {
        LOG("fwrite() error: %s\n", strerror(errno));
        goto exit;
    }
    fflush(ffmpeg);

    // Open pipe
    pipe = fopen(pipe_name, "r");
    if (pipe == NULL) {
        LOG("fopen() error: %s\n", strerror(errno));
        goto exit;
    }

    // Release ffmpeg pipe
    fclose(ffmpeg);
    ffmpeg = NULL;

    // Read result from pipe
    while(fread(jpeg + jpeg_out->size, sizeof(uint8_t), 1, pipe) > 0) {
        jpeg_out->size++;
    }
    jpeg_out->data = jpeg;

    fclose(pipe);
    return EXIT_SUCCESS;

exit:
    if (ffmpeg) fclose(ffmpeg);
    if (pipe) fclose(pipe);
    remove(pipe_name);
    return EXIT_FAILURE;
}

int32_t jpeg_from_keyframe() {

    // Open keyframe
    int32_t keyframe_size = 0;
    keyframe_t keyframe = open_keyframe("/home/rampopula/vscode/avstream_server/build/keyframe_annexb", &keyframe_size);
    if (keyframe == NULL) {
        return EXIT_FAILURE;
    }

    ImageJpeg_t jpeg;
    jpeg.size = 0;
    jpeg.width = 256;
    jpeg.height = 144;
    jpeg.quality = 15;
    jpeg.data = NULL;

    if (ffmpeg_keyframe2jpeg(keyframe, keyframe_size, &jpeg, 1) != 0) {
        LOG("ffmpeg_keyframe2jpeg() failed!\n");
        return EXIT_FAILURE;
    }
    LOG("ffmpeg_keyframe2jpeg() success! jpeg size: %d\n", jpeg.size);

    // write jpeg
    // remove("output.jpeg");
    FILE *jpg = fopen("output.jpeg", "a+b");
    fwrite(jpeg.data, 1, jpeg.size, jpg);
	fclose(jpg);

    destroy_keyframe(keyframe);
    return EXIT_SUCCESS;
}


int main(int argc, char *argv[]) {

    int32_t ret;
    char filepath[128];
	int32_t udp_delay = 10;

    // Get input file name
    int32_t opt = getopt(argc, argv, "i:");
	if (opt != -1 && opt == 'i') {
        snprintf(filepath, 32, "%s", optarg);
	}
    else {
		#ifdef FILE_DEBUG
			snprintf(filepath, 128, "%s", MP4_IN_PATH);
		#else
			printf("Usage: avserver -i sample.mp4 -d 100\n");
			return EXIT_FAILURE;
		#endif
    }
	
	opt = getopt(argc, argv, "d:");
	if (opt != -1 && opt == 'd') {
		udp_delay = atoi(optarg);
	}
	LOG("Set UDP send delay to %d ms\n", udp_delay);

	// Open udp socket
	if (udpsock_open(udp_delay) != 0) {
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
