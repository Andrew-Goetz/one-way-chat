#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>

/***** Constant Definitions *****/
#define ARG_NUM 2 + 1
#define BUF 1000

struct args { 
	int port;
	char *username;
};

enum ARGS {
	PORT = 1,
	USERNAME
};

/***** Function Definitions *****/
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
		fprintf(stderr, "WARNING (%s): could not find tcp protocol number, defaulting to 0.\n", __func__);
		protoP->p_proto = 0;
	}

	/* Open socket */
	int receive_fd;
	if((receive_fd = socket(AF_INET, SOCK_STREAM, protoP->p_proto)) < 0) {
		fprintf(stderr, "ERROR (%s): failure to open socket.\n", __func__);
		exit_status = EXIT_FAILURE;
		goto ERROR;
	}
	
	/* Bind socket */
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(arguments->port);
	if(bind(receive_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		fprintf(stderr, "ERROR (%s): failure to bind socket. Try choosing a different port.\n", __func__);
		exit_status = EXIT_FAILURE;
		goto ERROR;
	}

	/* Start listening on socket */
	printf("Listening on socket at port %d...\n", arguments->port);
	if(listen(receive_fd, 1) > 0) {
		fprintf(stderr, "ERROR (%s): failure to listen on receive socket.\n", __func__);
		exit_status = EXIT_FAILURE;
		goto ERROR;
	}

	socklen_t slen = sizeof(addr);
	int rs = accept(receive_fd, (struct sockaddr *)&addr, &slen);
	if(rs < 0) {
		fprintf(stderr, "ERROR (%s): error accepting connection at port %d.\n", __func__, arguments->port);
		exit_status = EXIT_FAILURE;
		goto ERROR;
	}

	/* Enter receive loop */
	char receive_buf[BUF];
	printf("Connection accepted on port %d! Listening for new messages...\n", arguments->port);
	for(;;) {
		memset(&receive_buf, 0, BUF);
		read(rs, receive_buf, BUF);
		printf("Received message: %s", receive_buf);
	}

	/* Clean-up and return with exit_status */
	ERROR:
	if(close(receive_fd) == -1 && errno != EBADF) {
		fprintf(stderr, "WARNING: (%s): error closing file descriptor %d.\n.", __func__, receive_fd);
	}
	free(arguments);
	return exit_status;
}

void *malloc_chk(size_t size) {
	void *p = malloc(size);
	if(!p) {
		fprintf(stderr, "ERROR: malloc failure for size %zu:\n%s", size, strerror(errno));
	}
	return p;
}

/* strtol() that returns an int, with basic error checking from https://stackoverflow.com/a/26083517 */
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
	arguments->username = argv[USERNAME];
	return arguments;
}

void print_usage(char *prog_name) {
	fprintf(stderr, "Usage: %s [PORT] [USERNAME]\n", prog_name);
}
