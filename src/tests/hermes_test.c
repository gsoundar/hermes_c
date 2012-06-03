/**
 * HERMES - TEST
 * -------------
 * by Gokul Soundararajan
 *
 * Simple testing for HERMES C edition
 *
 **/

#include <stdio.h>
#include <stdlib.h>
#include <hermes.h>

/* function headers */
static int __my_validate( hms_endpoint *endpoint, hms_msg *msg );
static int __my_accepts( hms_endpoint *endpoint, hms_msg *msg );
static int __my_handle( hms_endpoint *endpoint, hms_msg *msg );

int main(int argc, char **argv) {

  /* Initialize hermes with 10 threads */
  fprintf(stdout, "Starting hermes\n"); fflush(stdout);
  hms_ops ops;
  ops.hms_handle = __my_handle;
  ops.hms_validate = __my_validate;
  ops.hms_accepts = __my_accepts;
  hms* manager = hermes_init(10, 61182, ops);

  /* Press key to shutdown */
  char end;
  fscanf(stdin, "%c", &end);

  fprintf(stdout, "Requesting shutdown\n"); fflush(stdout);
  hermes_shutdown(manager, HMS_TRUE);
  fprintf(stdout, "Done\n"); fflush(stdout);

  return 0;

} /* end main() */


static int __my_validate( hms_endpoint *endpoint, hms_msg *msg ) {
  return 0;
}

static int __my_accepts( hms_endpoint *endpoint, hms_msg *msg ) {

  char *verb;
  int will_accept = HMS_FALSE;

  hms_msg_get_verb( msg, &verb );
  if(!verb) { return -1; }
  else if( strcasecmp(verb, "TEST") == 0) {will_accept = HMS_TRUE;}
  else { will_accept = HMS_FALSE; }

  free(verb);
		      
  return (will_accept == HMS_TRUE) ? 0 : -1;

}

static int __my_handle( hms_endpoint *endpoint, hms_msg *msg ) {

  printf("handled\n"); fflush(stdout);

  return 0;

}
