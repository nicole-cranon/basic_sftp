/*
 *	server.c - is based on server.c demo
 *	nicole cranon <nicole.cranon@ucdenver.edu>
 *  cs4761, spring 2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>


#define MY_PORT 3000	// port to listen on

#define BACKLOG 10		// size of pending connections queue

void sigchld_handler(int s);

int main(void){
	int sockfd,
		new_fd;	// listen on sockfd, new connection on new_fd

	struct sockaddr_in my_addr;	// my address info
	struct sockaddr_in their_addr;	// connector's address information

	int sin_size;

	struct sigaction sa;
	int yes=1;
	char *recvbuf;
	char *caddr;
	int numbytes;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	char hostname[128];

	gethostname(hostname, sizeof(hostname));
	printf("My hostname: %s\n", hostname);

	printf("SOCK_FD =%d\n", sockfd);

	if (setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}

	my_addr.sin_family = AF_INET;					// host byte order
	my_addr.sin_port = htons(MY_PORT);		// short network byte order
	my_addr.sin_addr.s_addr = INADDR_ANY;	// automatically fill with my IP

	memset(&(my_addr.sin_zero), '\0', 8);	// zero the rest of the struct

	if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) == -1) 	{
		perror("bind");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	} 


	sa.sa_handler = sigchld_handler;	// reap all dead processes 
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;

	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	// main loop
	char *invalid_command = "Error: Invalid command.\n";
	while (1) {
		 socklen_t sin_size = sizeof(struct sockaddr_in);

		if ((new_fd = accept (sockfd, (struct sockaddr *) &their_addr, &sin_size)) == -1) {
			perror("accept");
			exit(1);
		}

		printf ("server: got connection from %s\n", (char *) inet_ntoa(their_addr.sin_addr));

		if (!fork()) {	// is a child process
			close(sockfd);	// child does not need the listener
			recvbuf = (char *) calloc(128, sizeof(char));

			for (;;) {
				numbytes = recv(new_fd, recvbuf, 128, 0);
				if (numbytes < 0) {
					perror("recv");
					exit(1);
				} else if (numbytes == 0 || strncmp(recvbuf, "bye", 3) == 0) {
					printf("Client(%s) has been disconnected\n", (char *) inet_ntoa(their_addr.sin_addr));
					close(new_fd);
					exit(0);
				}

				printf("Received from %s: %s\n", inet_ntoa(their_addr.sin_addr), recvbuf);

				char* catalog = NULL;
				char* spwd = NULL;
				// user requested catalog
				if (strcmp(recvbuf, "catalog") == 0) {
					// s_catalog();

					char cmdline[] = "ls . > .lsout"; 
					system(cmdline);

					FILE *fp;

					fp = fopen("./.lsout", "r");

					if( !fp ){
						perror("fopen");
						exit(1);
					}

					fseek( fp , 0L , SEEK_END);
					long lSize = ftell( fp );
					rewind( fp );
					char buff[lSize];

					fread( buff , lSize, 1 , fp);
					fclose(fp);

					catalog = buff;

					if (catalog != NULL) {
						if (send(new_fd, catalog,lSize, 0) == -1) {
							perror("send");
							exit(1);
						}
					}

					catalog = "";
				} else if (strcmp(recvbuf, "spwd") == 0) {

					char cmdline[] = "pwd . > .spwd"; 
					system(cmdline);

					FILE *fp;

					fp = fopen("./.spwd", "r");

					if( !fp ){
						perror("fopen");
						exit(1);
					}

					fseek( fp , 0L , SEEK_END);
					long lSize = ftell( fp );
					rewind( fp );
					char buff[lSize];

					fread( buff , lSize, 1 , fp);
					fclose(fp);

					spwd = buff;

					if (spwd != NULL) {
						if (send(new_fd, spwd,lSize, 0) == -1) {
							perror("send");
							exit(1);
						}
					}

					spwd = "";
				} else if (strncmp(recvbuf, "download", 8) == 0) {
					// TODO: add download functionality
				} else if (strncmp(recvbuf, "upload", 8) == 0) {
					// TODO: add upload functionality
				} else {
					if (send(new_fd, invalid_command,numbytes, 0) == -1) {
						perror("send");
						exit(1);
					}
				}
			}

			close(new_fd);
			exit(0);
		}

		close(new_fd);	// parent doesn't need this
	}

	return 0;
}

// function definitions

void sigchld_handler(int s) {
	while(wait(NULL) > 0);
}