#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include "common.h"


// #define BUF_SIZE 10             /* Maximum size of messages exchanged
//                                    between client and server */

// #define SV_SOCK_PATH 		"/tmp/avudpsock"
// #define SV_CLI_SOCK_PATH	"/tmp/avudpsockcli"	


/* CLIENT */
// int main(int argc, char *argv[]) {

// 	struct sockaddr_un udp_client_addr;
// 	int udp_sock_fd;
// 	int receive_bytes = 0;
// 	char buffer [1024];

// 	udp_sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
// 	if(udp_sock_fd == -1) {
// 		printf("Error creating UDP sock \n");
// 		return -1;
// 	}

// 	if(remove(SV_CLI_SOCK_PATH) == -1 && errno != ENOENT) {
// 		printf("SV_CLI_SOCK_PATH remove error \n");
// 		return -1;
// 	}

// 	memset(&udp_client_addr, 0x00, sizeof(struct sockaddr_un));
// 	udp_client_addr.sun_family = AF_UNIX;
// 	snprintf(udp_client_addr.sun_path, sizeof(udp_client_addr.sun_path), SV_CLI_SOCK_PATH);

// 	if(bind(udp_sock_fd, (struct sockaddr *)&udp_client_addr, sizeof(struct sockaddr_un) - 1) == -1) {
// 		printf("UDP sock bind error\n");
// 		return -1;
// 	}

// 	while(1) {
// 		// length = sizeof(struct sockaddr_un);
// 		receive_bytes = recvfrom(udp_sock_fd, buffer, sizeof(buffer), 0, NULL, NULL);

// 		if(receive_bytes == -1) {
// 			printf("recvfrom sock error\n");
// 			return -1;
// 		}
// 		buffer[receive_bytes] = '\0';
// 		printf("recv %d bytes: %s\n", receive_bytes, buffer);
// 	}
// }

/* SERVER */
// int main(int argc, char *argv[]) {

// 	struct sockaddr_un udp_sock_addr, udp_client_addr;
// 	int udp_sock_fd;
// 	char buffer [1024];

// 	udp_sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
// 	if(udp_sock_fd == -1) {
// 		printf("Error creating UDP sock \n");
// 		return -1;
// 	}

// 	if(remove(SV_SOCK_PATH) == -1 && errno != ENOENT) {
// 		printf("SV_SOCK_PATH remove error \n");
// 		return -1;
// 	}

// 	memset(&udp_sock_addr, 0x00, sizeof(struct sockaddr_un));
// 	udp_sock_addr.sun_family = AF_UNIX;
// 	strncpy(udp_sock_addr.sun_path, SV_SOCK_PATH, sizeof(udp_sock_addr.sun_path) - 1);

// 	if(bind(udp_sock_fd, (struct sockaddr *)&udp_sock_addr, sizeof(struct sockaddr_un) - 1) == -1) {
// 		printf("UDP sock bind error\n");
// 		return -1;
// 	}

// 	memset(&udp_client_addr, 0x00, sizeof(struct sockaddr_un));
// 	udp_client_addr.sun_family = AF_UNIX;
// 	snprintf(udp_client_addr.sun_path, sizeof(udp_client_addr.sun_path), SV_CLI_SOCK_PATH);


	
// 	int ret, i = 0;
// 	while(1) {
// 		sprintf(buffer, "Test message from server #%d", i++);
		
// 		do {
// 			errno = 0;
// 			ret = sendto(udp_sock_fd, buffer, 0, 0, (struct sockaddr*)&udp_client_addr, sizeof(struct sockaddr_un));
// 			usleep(1000 * 10);
// 		} while(errno == ECONNREFUSED);
		
// 		if (ret < 0) {
// 			printf("sendto error(%d): %s\n", errno, strerror(errno));
// 		}

// 		usleep(1000 * 500);
// 	}
// 	remove(SV_SOCK_PATH);
// 	return 0;
// }




#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "udpsock.h"
#include "demuxer.h"

#define PACKET_DATA_OFFFSET			(8)
#define PACKET_BUF_SIZE				(32768)
#define PACKET_TYPE_AVC				"AVC"
#define PACKET_TYPE_AAC				"AAC"


static uint8_t packet_buf[PACKET_BUF_SIZE];
static int i = 0;

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

	uint8_t chn_id = 10;
	char *type = PACKET_TYPE_AVC;

	// return 0;

	// // FILE *file = fopen("test.h264", "a+b");
	// // fwrite(frame, 1, size, file);
	// // fclose(file);

	// // return 0;


	memcpy((void*)packet_buf + PACKET_DATA_OFFFSET, (void*)frame, size);
	wrap_packet(packet_buf, type, chn_id, size);
	udpsock_send(packet_buf, size + PACKET_DATA_OFFFSET);

    LOG("%d) callback ---> send video frame, size: %d b\n", ++i, size);

    return EXIT_SUCCESS;
}

int32_t aframe_proc(uint8_t *frame, int32_t size) {

	uint8_t chn_id = 10;
	char *type = PACKET_TYPE_AAC;
	// static int i = 0;


	size += 7;

	uint8_t adts_header[7] = {0xFF,0xF1,0x60,0x80,0x2F,0x7F,0xFC};



	// FILE *file = fopen("test.aac", "a+b");

	// fwrite(adts, 1, 7, file);
	// fwrite(frame, 1, size, file);
	// fclose(file);

	// // printf("%d\n", ++i);

	// return 0;

	memcpy((void*)packet_buf + PACKET_DATA_OFFFSET, (void*)adts, sizeof(adts));
	memcpy((void*)packet_buf + PACKET_DATA_OFFFSET + 7, (void*)frame, size);
	wrap_packet(packet_buf, type, chn_id, size);
	udpsock_send(packet_buf, size + PACKET_DATA_OFFFSET);

    LOG("%d) callback ---> send audio frame, size: %d b\n", ++i, size);
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
        printf("Usage: avserver -i sample.mp4\n");
		return EXIT_FAILURE;
		// snprintf(filepath, 128, "%s", "/home/rampopula/vscode/avstream_server/build/vid.mp4");
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
