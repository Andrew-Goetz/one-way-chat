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

struct args {
	int port;
	char *username;
	struct in_addr addr;
};

enum ARGS {
	ADDR = 1,
	PORT,
	USERNAME
};

struct args *parse_args(int argc, char *argv[]);
void *malloc_chk(size_t size);
int strtoi_chk(const char *restrict nptr, char *endptr, int base);
void print_usage(char *prog_name);

int main(int argc, char *argv[]) {
	struct args *arguments = parse_args(argc, argv);

	/* Setup sending socket */
	int send_fd;
	if((send_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "ERROR (%s): failure to create send socket.", __func__);
		free(arguments);
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	if((addr.sin_addr.s_addr = inet_addr("127.0.0.1")) == INADDR_NONE) {
		fprintf(stderr, "ERROR (%s): failure to create send socket.", __func__);
	}
	addr.sin_port = htons(arguments->port);

	if(connect(send_fd, (const struct sockaddr *)&addr, sizeof(addr))) {
		fprintf(stderr, "ERROR (%s): failure to connect to socket at http://127.0.0.1:%d.", __func__, arguments->port);
		//TODO goto cleanup and exit
	}

	printf("Socket connection established to http://127.0.0.1:%d. Press enter to continue to messaging...", arguments->port);
	while(getchar() != '\n');
	char send_buf[BUF];
	printf("Connection accepted! Messaging is ready.\n");
	for(;;) {
		printf(" [%s]:- ", arguments->username);
		fgets(send_buf, sizeof(send_buf), stdin);
		write(send_fd, send_buf, BUF);
		memset(&send_buf, 0, BUF);
	}

	ERROR:
	//pthread_exit(NULL);
	if(close(send_fd) == -1 && errno != EBADF)
		fprintf(stderr, "WARNING: (%s): error closing file descriptor %d\n", __func__, send_fd);

	free(arguments);
}

void *malloc_chk(size_t size) {
	void *p = malloc(size);
	if(!p)
		fprintf(stderr, "Error: malloc failure for size %zu:\n%s", size, strerror(errno));
	return p;
}

/* strtol() that returns an int, with basic error checking from https://stackoverflow.com/a/26083517 */
int strtoi_chk(const char *restrict nptr, char *endptr, int base) {
	const long result = strtol(nptr, &endptr, base);
	if(nptr == endptr)
		fprintf(stderr, "WARNING (%s): invalid argument to strtoi_chk, '%s', continuing with result=%d\n", __func__, nptr, (int)result);
	else if(result > INT_MAX || result < INT_MIN)
		fprintf(stderr, "WARNING (%s): value '%s' outside range of int, continuing with result=%d\n", __func__, nptr, (int)result);
	else if(errno != 0)
		fprintf(stderr, "WARNING (%s): strtoi_chk set errno=%d, continuing with result=%d\n", __func__, errno, (int)result);
	return (int)result;
}

struct args *parse_args(int argc, char *argv[]) {
	/* Verify arg num*/
	if(argc != ARG_NUM) {
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	/* Verify IP address */
	struct in_addr addr;
	if(!inet_aton(argv[ADDR], &addr)) {
		fprintf(stderr, "ERROR (%s): invalid address", __func__);
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}
	

	/* Verify ports */
	int port = strtoi_chk(argv[PORT], NULL, 10);
	if(port <= 1023 || port >= 49152) {
		fprintf(stderr, "ERROR (%s): port must be greater than 1023 and less than 49152\n", __func__);
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}
	
	/* Initialize and return args struct */
	struct args *arguments = malloc_chk(sizeof(struct args));
	arguments->port = port;
	arguments->username = argv[USERNAME];
	return arguments;
}

void print_usage(char *prog_name) {
	fprintf(stderr, "Usage: %s [ADDR] [PORT] [USERNAME]\n", prog_name);
}
