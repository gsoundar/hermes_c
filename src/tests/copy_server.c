/**
 * HERMES - COPY
 * -------------
 * by Gokul Soundararajan
 *
 * Building a COPY server using Hermes
 * - Handles the COPY command
 * - COPY filename offset length
 * - returns OK or ERROR
 *
 **/

#include <stdio.h>
#include <stdlib.h>
#include <hermes.h>

/* function headers */
static int __copy_validate( hms_endpoint *endpoint, hms_msg *msg );
static int __copy_accepts( hms_endpoint *endpoint, hms_msg *msg );
static int __copy_handle( hms_endpoint *endpoint, hms_msg *msg );

int main(int argc, char **argv) {

  /* Initialize hermes with 10 threads */
  fprintf(stdout, "Copy server 1.0\n"); fflush(stdout);
  hms_ops ops;
  ops.hms_handle = __copy_handle;
  ops.hms_validate = __copy_validate;
  ops.hms_accepts = __copy_accepts;
  hms* manager = hermes_init(1, 61182, ops);

  /* Press key to shutdown */
  char end;
  fscanf(stdin, "%c", &end);

  fprintf(stdout, "Requesting shutdown\n"); fflush(stdout);
  hermes_shutdown(manager, HMS_TRUE);
  fprintf(stdout, "Done\n"); fflush(stdout);

  return 0;

} /* end main() */


static int __copy_validate( hms_endpoint *endpoint, hms_msg *msg ) {

  /* a valid copy message must have
     - 0 un-named headers
     - content-length > 0
  */

  int is_valid = HMS_TRUE;

  char *verb; int is_copy = HMS_FALSE;
  if( hms_msg_get_verb( msg, &verb) == 0) { 
    if(strcasecmp(verb, "COPY") != 0) {
      is_copy = HMS_FALSE;
    }
    free(verb); verb = NULL;
  }

  if( is_copy == HMS_TRUE && hms_msg_num_headers( msg ) <= 0 ) { 
    is_valid = HMS_FALSE; 
  }
  else if( is_copy == HMS_TRUE && hms_msg_num_named_headers( msg ) != 3 ) { 
    is_valid = HMS_FALSE; 
  }
  else if( is_copy == HMS_TRUE && hms_msg_get_body_size( msg ) <= 0 ) {
    is_valid = HMS_FALSE;
  }

  return (is_valid == HMS_TRUE) ? 0 : -1;

}

static int __copy_accepts( hms_endpoint *endpoint, hms_msg *msg ) {

  char *verb;
  int will_accept = HMS_FALSE;

  hms_msg_get_verb( msg, &verb );
  if(!verb) { return -1; }
  else if( strcasecmp(verb, "COPY") == 0) {will_accept = HMS_TRUE;}
  else { will_accept = HMS_FALSE; }

  free(verb);
		      
  return (will_accept == HMS_TRUE) ? 0 : -1;

}

static int __copy_handle( hms_endpoint *endpoint, hms_msg *msg ) {

  int fd = -1;
  int erred = HMS_FALSE;

  /* Extract arguments */
  /* Assuming msg is valid since it went through the validate phase */
  char *filename, *offset_str;
  hms_msg_get_named_header( msg, "Filename", &filename );
  hms_msg_get_named_header( msg, "Offset", &offset_str );
  if(!filename) { erred = HMS_TRUE; goto copy_handle_done; }
  if(!offset_str) { erred = HMS_TRUE; goto copy_handle_done; }

  /* open file */
  fd = open( filename, O_WRONLY | O_CREAT , S_IRUSR | S_IWUSR );
  if(fd == -1) { erred = HMS_TRUE; goto copy_handle_done; }

  /* find off and len */
  int offset = atoi( offset_str );
  int length = hms_msg_get_body_size( msg );
  
  /* extract the data */
  char *data = NULL; int data_len;
  hms_msg_get_body( msg, &data, &data_len );
  if(!data) { erred = HMS_TRUE; goto copy_handle_done; }
  if( data_len != length) { erred = HMS_TRUE; goto copy_handle_done; }

  //fprintf(stdout, "writing to file: |%s| off:|%d| len:|%d| \n",
  //filename, offset, length );
  
  /* write to file */
  if( pwrite( fd, data, length, offset ) != length ) {
    erred = HMS_TRUE;
    goto copy_handle_done;
  }

  /* clean up */
 copy_handle_done:
  if(fd >= 0) close(fd);
  if(filename) free(filename);
  if(offset_str) free(offset_str);
  if(data) free(data);

  return (erred == HMS_FALSE) ? 0 : -1 ;

}
