#include <stdio.h> 
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <stdlib.h>
#include <string.h>

#define SIZE 1024 
char buf[SIZE];
#define PORT 13000
int main(int argc, char *argv[]) { 
    int sockfd; 
    int nread; 
    struct sockaddr_in serv_addr; 
    if (argc != 2) { 
	fprintf(stderr, "usage: %s IPaddr\n", argv[0]); 
	exit(1); 
    } 


    while (fgets(buf, SIZE , stdin) != NULL) {
	/* create endpoint */ 
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
	    perror(NULL); exit(2); 
	} 
	/* connect to server */ 
	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]); 
	serv_addr.sin_port = htons(PORT);
 
	while (connect(sockfd, 
		       (struct sockaddr *) &serv_addr, 
		       sizeof(serv_addr)) < 0) {
	    /* allow for timesouts etc */
	    perror(NULL);
	    sleep(1);
	}
	
	printf("%s", buf);
	nread = strlen(buf);
	/* transfer data and quit */ 
	write(sockfd, buf, nread);
	close(sockfd); 
    }
} 
