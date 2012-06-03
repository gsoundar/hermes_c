/**
 * HERMES - C edition
 * ------------------
 * by Gokul Soundararajan
 *
 * This utility copies a file to the server
 *
 **/

#include <hermes.h>
#define __DEBUG_PRINT_MSG_HEADER__

static int __send_chunk( hms_connector *connector, char *src, char *dst, int offset, int len );

int main(int argc, char **argv) {

  int erred = HMS_FALSE;
  char *src_file, *dst_file;
  char *hostname;
  int port;
  int chunk_size;

  if(argc == 6 ) {
    src_file = argv[1];
    hostname = argv[2];
    port = atoi( argv[3] );
    dst_file = argv[4];
    chunk_size = atoi(argv[5]);
  } else {
    fprintf(stderr, "USAGE: %s [src_file] [server] [port] [file_on_server] [chunk_size]\n",
	    argv[0]);
    fflush(stderr);
    return -1;
  }

  hms_connector *connector = NULL;

  /* check file */
  FILE *src = NULL;
  int file_len = -1, offset = 0, left = -1;
  src = fopen(src_file, "r"); 
  if(src == NULL) { 
    fprintf(stderr, "cannot open source file\n"); fflush(stderr);
    erred = HMS_TRUE; goto done_main; 
  }

  /* find length of file */
  fseek( src, 0, SEEK_END ); file_len = ftell(src);
  fclose( src );
  fprintf(stdout, "file len: |%d|\n", file_len );

  /* open connection */
  connector = hms_connector_init( hostname, port );

  /* send chunk by chunk */
  left = file_len; offset = 0;
  while( left > 0 ) {
    int status;
    if( left > chunk_size ) {
      status = __send_chunk( connector, src_file, dst_file, offset, chunk_size );
      offset += chunk_size; left -= chunk_size;
    } else {
      status = __send_chunk( connector, src_file, dst_file, offset, left );
      offset += left; left = 0;
    }
    if(status == -1) { 
      fprintf(stderr, "send chunk failed\n"); fflush(stderr);
      erred = HMS_TRUE; goto done_main; 
    }
  }

  /* destroy connector */
  hms_connector_destroy( connector );

 done_main:
  return (erred == HMS_FALSE) ? 0 : -1;

} /* end main() */


static int __send_chunk( hms_connector *connector, char *src, char *dst, int offset, int len ) {

  int erred = HMS_FALSE;
  int ret = 0;
  char *data=NULL, offset_str[32];
  sprintf(offset_str, "%d", offset);
  
  /*
    fprintf(stdout, "[%s][%s] chunk: off: |%d| len:|%d| \n",
	  src, dst, offset, len );
	  fflush(stdout);
  */

  /* build message */
  hms_msg *request = NULL;
  request = hms_msg_create(); 
  hms_msg_set_verb( request, "COPY" );
  hms_msg_add_named_header( request, "Filename" , dst );
  hms_msg_add_named_header( request, "Offset" , offset_str );

  /* Open src file */
  int src_fd = -1; src_fd = open(src, O_RDONLY, 0 );
  if(src_fd == -1)  { 
    fprintf(stderr, "cannot open source file\n"); fflush(stderr);
    erred = HMS_TRUE; goto done_send_chunk; 
  }

  /* Read chunk */
  data = malloc( len + 1 );
  if(data == NULL)  { 
    fprintf(stderr, "cannot malloc data\n"); fflush(stderr);
    erred = HMS_TRUE; goto done_send_chunk; 
  }
  ret = pread( src_fd, data, len, offset );
  if(ret != len)  { 
    fprintf(stderr, "cannot pread\n"); fflush(stderr);
    erred = HMS_TRUE; goto done_send_chunk; 
  }

  /* Add to message */
  ret = hms_msg_set_body( request, data, len );
  if(ret)  { 
    fprintf(stderr, "cannot add body\n"); fflush(stderr);
    erred = HMS_TRUE; goto done_send_chunk; 
  }

  /* Send request */
  hms_connector_send_msg( connector, request );
  if(ret)  { 
    fprintf(stderr, "cannot send chunk\n"); fflush(stderr);
    erred = HMS_TRUE; goto done_send_chunk; 
  }

#ifdef __DEBUG_PRINT_MSG_HEADER__
  {
    unsigned int debug_buf_size = 0;
    char *debug_buf = NULL;
    debug_buf_size = hms_msg_get_header_size(request);
    debug_buf = (char *) malloc(debug_buf_size);
    hms_msg_print_header(request, debug_buf, debug_buf_size);
    fprintf(stdout, "%s", debug_buf);
    free(debug_buf); debug_buf = NULL; debug_buf_size = 0;
  }
#endif

 done_send_chunk:
  if(src_fd != -1) close(src_fd);
  if(data) free(data);
  if(request) hms_msg_destroy( request );

  return ( erred == HMS_FALSE ) ? 0 : -1;

} /* end __send_chunk() */
