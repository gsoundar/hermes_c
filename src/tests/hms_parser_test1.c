/**
 * HERMES - Test
 * -------------
 * by Gokul Soundararajan
 *
 * Message parsing tests for Hermes C edition
 *
 **/

#include <hermes.h>
#include <hermes_internal.h>
#include <assert.h>

int main(int argc, char **argv) {

  int fd = 0; /* 0 is stdin */
  int msgs_recvd = 0;
  int max_hdr_size = 1024;

  while( msgs_recvd < 1000000 ) {

    hms_msg *msg = hms_msg_parse( fd, max_hdr_size );
    if(!msg) break;

    char *buffer = malloc( max_hdr_size );
    int hdr_sz = hms_msg_get_header_size(msg);
    hms_msg_print_header( msg, buffer, max_hdr_size );

    /* printf("msg: hdr:%d body:%d\n", 
	  hms_msg_get_header_size(msg),
	  hms_msg_get_body_size(msg) ); */
    buffer[hdr_sz]='\0';
    printf("%s", buffer );

    char *body = NULL; int body_sz;
    hms_msg_get_body( msg, &body, &body_sz );
    printf("%s", body);


    if(buffer) { free(buffer); buffer = NULL;}
    if(body) { free(body); body = NULL; }
    hms_msg_destroy( msg );
    
    msgs_recvd++; /* printf("recvd: %d\n", msgs_recvd); */

  } /* end while(1) */



  return 0;

} /* end main() */
