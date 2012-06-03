/**
 * HERMES - bench
 * ------
 * by Gokul Soundararajan
 *
 **/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <netinet/tcp.h>
#include <sys/socket.h> 
#include <assert.h>

int main(int argc, char **argv) {

  int msg_size = 1024;
  char *buffer = (char *) malloc( msg_size + 1);
  
  int i = 0; 
  for(i=0; i < (1024*1024); i++) {
    
    srand(i);

    /* make message */
    {
      int i = 0;
      char *alphabet = "abcdefghijklmnopqrstuvwxyz";
      int len = strlen(alphabet);
      for( i=0; i < (msg_size - 1); i++) {
	buffer[i] = alphabet[ random() % len ];
      }
      buffer[i++] = '\n';
      buffer[i++] = '\0';
    }
    assert( strlen(buffer) == msg_size );

    fprintf(stdout, "PING\n");
    //fprintf(stdout, "a:b\n");
    //fprintf(stdout, "\n");
    //fprintf(stdout, "  c  : d \n");
    fprintf(stdout, "Content-Length:%d\n", msg_size);
    fprintf(stdout, ".\n");
    fprintf(stdout, "%s", buffer );
    
  } /* end for loop */

  free(buffer);

  return 0;

} /* end main() */
