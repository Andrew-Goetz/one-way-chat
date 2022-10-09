#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>

#define ARG_NUM 2 + 1
#define BUF 1000

struct args { int port;
	char *username;
	int addr;
};

enum ARGS {
	ADDR = 1,
	PORT,
	USERNAME
};

struct args *parse_args(int argc, char *argv[]);
void *malloc_chk(size_t size);
int strtoi_chk(const char *restrict nptr, char *endptr, int base);

int main(int argc, char *argv[]) {
	struct args *arguments = parse_args(argc, argv);
	
	/* Setup listener socket */
	int receive_fd;
	if((receive_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "ERROR (%s): failure to create receive socket.", __func__);
		//TODO goto cleanup and exit
	}
	
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(arguments->port);

	if(bind(receive_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		fprintf(stderr, "ERROR (%s): failure to bind socket to receive address.", __func__);
		//TODO goto cleanup and exit
	}
	if(listen(receive_fd, 3) > 0) {
		fprintf(stderr, "ERROR (%s): failure to listen on receive socket.", __func__);
		//TODO goto cleanup and exit
	}
	printf("Socket listening at http://127.0.0.1:%d. Press enter to continue...", arguments->port);
	while(getchar() != '\n');

	socklen_t slen = sizeof(addr);
	int rs = accept(receive_fd, (struct sockaddr *)&addr, &slen);
	if(rs < 0) {
		fprintf(stderr, "ERROR (%s): failure to connect to socket at http://127.0.0.1:%d.", __func__, arguments->port);
		//TODO goto cleanup and exit
	}
	char receive_buf[BUF];
	printf("Listening for new messages\n");
	for(;;) {
		read(rs, receive_buf, BUF);
		printf("Received message: %s\n", receive_buf);
	}
	close(rs);
	close(receive_fd);
}

void *malloc_chk(size_t size) {
	void *p = malloc(size);
	if(!p)
		fprintf(stderr, "ERROR: malloc failure for size %zu:\n%s", size, strerror(errno));
	return p;
}

/* strtol() that returns an int, with basic error checking from https://stackoverflow.com/a/26083517 */
int strtoi_chk(const char *restrict nptr, char *endptr, int base) {
	const long result = strtol(nptr, &endptr, base);
	if(nptr == endptr)
		fprintf(stderr, "WARNING (%s): invalid argument to strtoi_chk, '%s', continuing with result=%ld\n", __func__, nptr, result);
	else if(result > INT_MAX || result < INT_MIN)
		fprintf(stderr, "WARNING (%s): value '%s' outside range of int, continuing with result=%d\n", __func__, nptr, (int)result);
	else if(errno != 0)
		fprintf(stderr, "WARNING (%s): strtoi_chk set errno=%d, continuing with result=%ld\n", __func__, errno, result);
	return (int)result;
}

struct args *parse_args(int argc, char *argv[]) {
	/* Verify arg num*/
	if(argc != ARG_NUM) {
		fprintf(stderr, "Usage: %s [ADDR] [PORT] [USERNAME]\n", argv[0]);
		exit(1);
	}

	/* Verify ports */
	int port = strtoi_chk(argv[PORT], NULL, 10);
	if(port <= 1023 || port >= 49152) {
		fprintf(stderr, "ERROR (%s): port must be greater than 1023 and less than 49152\n", __func__);
		fprintf(stderr, "Usage: %s [ADDR] [PORT] [USERNAME]\n", argv[0]);
		exit(1);
	}
	
	/* Initialize and return args struct */
	struct args *arguments = malloc_chk(sizeof(struct args));
	arguments->port = port;
	arguments->username = argv[USERNAME];
	return arguments;
}
