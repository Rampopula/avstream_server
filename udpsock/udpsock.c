#include "udpsock.h"


static int32_t sock_fd = -1;
static struct sockaddr_un svaddr, claddr;
static int32_t udpsock_opened = FALSE;


int32_t udpsock_open(void) {

	if (udpsock_opened) {
		LOG("Already opened!\n");
		return EXIT_SUCCESS;
	}

	sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (sock_fd == -1) {
		LOG("Can't create socket: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	if (remove(UDPSOCK_SERVER_PATH) == -1 && errno != ENOENT) {
		LOG("Remove %s\n", UDPSOCK_SERVER_PATH);
		close(sock_fd);
		return EXIT_FAILURE;
	}

	memset((void*)&svaddr, 0x00, sizeof(struct sockaddr_un));
	svaddr.sun_family = AF_UNIX;
	strncpy(svaddr.sun_path, UDPSOCK_SERVER_PATH, sizeof(svaddr.sun_path) - 1);

	if (bind(sock_fd, (struct sockaddr*)&svaddr, sizeof(struct sockaddr_un)) == -1) {
		LOG("Bind error: %s\n", strerror(errno));
		close(sock_fd);
		return EXIT_FAILURE;
	}

	memset(&claddr, 0x00, sizeof(struct sockaddr_un));
	claddr.sun_family = AF_UNIX;
	snprintf(claddr.sun_path, sizeof(claddr.sun_path), UDPSOCK_CLIENT_PATH);

	udpsock_opened = TRUE;
	return EXIT_SUCCESS;
}

int32_t udpsock_close(void) {

	if (!udpsock_opened) {
		LOG("Already closed!");
		EXIT_SUCCESS;
	}

	close(sock_fd);
	return EXIT_SUCCESS;
}


int32_t udpsock_send(uint8_t *data, int32_t size) {
	
	int32_t ret;
	
	if (!udpsock_opened) {
		LOG("Socket is closed!\n");
		EXIT_FAILURE;
	}

	do {
		errno = 0;
		ret = sendto(sock_fd, data, size, 0, (struct sockaddr*)&claddr, sizeof(struct sockaddr_un));
		// LOG("%s\n", strerror(errno));
		 usleep(1000 * 50);
	} while(errno == ECONNREFUSED);

	return ret;
}
