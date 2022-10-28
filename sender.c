/* MIT License, by Andrew Goetz */

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>

#define ARG_NUM 3 + 1
#define BUF 1000

struct args {
	u_int16_t port;
	char addr[20];
	char *username;
};

enum ARGS {
	PORT = 1,
	ADDRESS,
	USERNAME
};

struct args *parse_args(int argc, char *argv[]);
void *malloc_chk(size_t size);
int strtoi_chk(const char *restrict nptr, char *endptr, int base);
void print_usage(char *prog_name);

int main(int argc, char *argv[]) {
	struct args *arguments = parse_args(argc, argv);
	int exit_status = EXIT_SUCCESS;

	/* Get protocol number */
	struct protoent *protoP;
	if(!(protoP= getprotobyname("tcp"))) {
		fprintf(stderr, "WARNING (%s): could not find TCP protocol number, defaulting to 0.\n", __func__);
		protoP->p_proto = 0;
	}

	/* Open socket */
	int send_fd;
	if((send_fd = socket(AF_INET, SOCK_STREAM, protoP->p_proto)) < 0) {
		fprintf(stderr, "ERROR (%s): failure to create send socket.\n", __func__);
		exit_status = EXIT_FAILURE;
		goto ERROR;
	}

	/* Connect to socket */
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	if((addr.sin_addr.s_addr = inet_addr(arguments->addr)) == INADDR_NONE) {
		fprintf(stderr, "ERROR (%s): invalid IPv4 address provided.\n", __func__);
		printf("%d\n", addr.sin_addr.s_addr);
		exit_status = EXIT_FAILURE;
		goto ERROR;
	}
	addr.sin_port = htons(arguments->port);
	if(connect(send_fd, (const struct sockaddr *)&addr, sizeof(addr))) {
		fprintf(stderr, "ERROR (%s): failure to connect to socket running at port %d. Try starting the receiver program first.\n", __func__, arguments->port);
		exit_status = EXIT_FAILURE;
		goto ERROR;
	}
	printf("Connection accepted! Ready to send messages.\n");
	
	/* Enter sending loop, currently no checks for buffer overflow */
	char send_buf[BUF];
	for(;;) {
		memset(&send_buf, 0, BUF);
		printf(" [%s]:- ", arguments->username);
		fgets(send_buf, sizeof(send_buf), stdin);
		if(write(send_fd, send_buf, BUF) < 0) {
			fprintf(stderr, "ERROR (%s): unable to write to socket, try again. If errors continue, restart the program.\n", __func__);
		}
	}

	/* Clean-up and return with exit_status */
	ERROR:
	if(close(send_fd) == -1 && errno != EBADF) {
		fprintf(stderr, "WARNING: (%s): error closing file descriptor %d.\n", __func__, send_fd);
	}
	free(arguments);
	return exit_status;
}

void *malloc_chk(size_t size) {
	void *p = malloc(size);
	if(!p) {
		fprintf(stderr, "Error: failure to malloc %zu bytes:\n%s", size, strerror(errno));
	}
	return p;
}

/* strtol() that returns an int, with basic error checking adapted from https://stackoverflow.com/a/26083517 */
int strtoi_chk(const char *restrict nptr, char *endptr, int base) {
	const long result = strtol(nptr, &endptr, base);
	if(nptr == endptr) {
		fprintf(stderr, "WARNING (%s): invalid argument to strtoi_chk, '%s', continuing with result=%d.\n", __func__, nptr, (int)result);
	} else if(result > INT_MAX || result < INT_MIN) {
		fprintf(stderr, "WARNING (%s): value '%s' outside range of int, continuing with result=%d.\n", __func__, nptr, (int)result);
	} else if(errno != 0) {
		fprintf(stderr, "WARNING (%s): strtoi_chk set errno=%d, continuing with result=%d.\n", __func__, errno, (int)result);
	}
	return (int)result;
}

struct args *parse_args(int argc, char *argv[]) {
	/* Verify arg num*/
	if(argc != ARG_NUM) {
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	/* Verify ports */
	int port = strtoi_chk(argv[PORT], NULL, 10);
	if(port <= 1023 || port >= 65536) {
		fprintf(stderr, "ERROR (%s): port must be greater than 1023 and less than 65536.\n", __func__);
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	/* Initialize and return args struct */
	struct args *arguments = malloc_chk(sizeof(struct args));
	arguments->port = port;
	strncpy(arguments->addr, argv[ADDRESS], sizeof(arguments->addr));
	arguments->username = argv[USERNAME];
	return arguments;
}

void print_usage(char *prog_name) {
	fprintf(stderr, "Usage: %s [PORT] [ADDRESS] [USERNAME]\n", prog_name);
}
