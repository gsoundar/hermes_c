/**
 * HERMES
 * ------
 * by Gokul Soundararajan
 *
 * A C implementation of the Hermes protocol
 *
 **/

#ifndef __HERMES_H__
#define __HERMES_H__

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
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
#include <signal.h>
#include <sys/time.h>
#include <time.h>

#include <hermes_internal.h>
#include <md5.h>
#include <tpool.h>
#include <hashtable.h>
#include <hms_list.h>

#define HERMES_MAX_HDR_SIZE 1024
#undef HERMES_ENABLE_CHECKSUMS 

/* Restricted headers */
#define HMS_CONTENT_LENGTH    "Content-Length"
#define HMS_CONTENT_CHECKSUM  "Content-Checksum"

typedef struct hashtab hashtab;

enum hms_bool { HMS_FALSE=0, HMS_TRUE=1 };
enum hms_type { HERMES_SERVER=0, HERMES_CLIENT=1 };

struct hms_endpoint;
struct hms_msg;
struct hms;
struct hms_msg_uheader;
struct hms_msg_nheader;

typedef struct hms_msg_uheader {
  char *val;
  struct hms_list_head lh;
} hms_msg_uheader;

typedef struct hms_msg_nheader {
  char *key;
  char *val;
  struct hms_list_head lh;
} hms_msg_nheader;

typedef struct hms_msg {

  /* request id - md5 has 32 chars, so size is 33 */
  char *id; /* optional field: request-id */
  
  /* verb */
  char *verb;

  /* unnamed headers */
  struct hms_msg_uheader headers;
  int num_headers;

  /* named headers */
  struct hms_msg_nheader named_headers;
  int num_named_headers;

  /* content */
  int content_len;
  char *content;

} hms_msg;

typedef struct hms_ops {
  int (*hms_validate) ( struct hms_endpoint *endpoint, hms_msg *msg );
  int (*hms_accepts)  ( struct hms_endpoint *endpoint, hms_msg *msg );
  int (*hms_handle)   ( struct hms_endpoint *endpoint, hms_msg *msg );
  /* TODO: provide logging function */
} hms_ops;

typedef struct hms {
  /* server socket */
  int server_socket;
  int server_port;

  /* variables */
  int status;
  int shutdown;
  int num_threads;

  /* connection handlers */
  tpool_t pool;

  /* functions */
  hms_ops dops;
  hms_ops ops;

  /* mutexes */
  pthread_mutex_t manager_lock;
  
} hms;

enum hms_endpoint_state { HMS_ENDPOINT_FREE=0, HMS_ENDPOINT_USED=1 };

typedef struct hms_endpoint {
  /* connected socket */
  int socket;
  /* start time */
  struct timeval start;
  /* status */
  int status;
  /* functions */
  hms_ops ops;
  /* mutexes */
  pthread_mutex_t meta_lock;
} hms_endpoint;

typedef struct hms_connector {
  /* connected socket */
  int socket;
  /* start time */
  struct timeval start;
  /* status */
  int status;
  /* mutexes */
  pthread_mutex_t meta_lock;
} hms_connector;


/* Functions */
/* ----------------------------------------------------- */

/* Manager */
/* ----------------------------------------------------- */
hms*           hermes_init( int num_threads , int server_port, hms_ops ops );
int            hermes_shutdown( hms *manager, int force );

/* Endpoint */
/* ----------------------------------------------------- */
hms_endpoint*  hms_endpoint_init( int fd, hms_ops ops );
int            hms_endpoint_recv_msg( hms_endpoint *endpoint, hms_msg **msg );
int            hms_endpoint_send_msg( hms_endpoint *endpoint, hms_msg *msg );
int            hms_endpoint_destroy( hms_endpoint *endpoint );

/* Connector */
/* ----------------------------------------------------- */
hms_connector* hms_connector_init( char *hostname, int port );
int            hms_connector_recv_msg( hms_connector *connector, hms_msg **msg );
int            hms_connector_send_msg( hms_connector *connector, hms_msg *msg );
int            hms_connector_destroy( hms_connector *connector );

/* Message */
/* ----------------------------------------------------- */
hms_msg *      hms_msg_create();
int            hms_msg_destroy( hms_msg *msg );
int            hms_msg_get_header_size( hms_msg *msg );
int            hms_msg_print_header( hms_msg *msg, char *buffer, int len);

int            hms_msg_set_verb(hms_msg *msg, char *verb );
int            hms_msg_get_verb(hms_msg *msg, char **verb );

int            hms_msg_add_header(hms_msg *msg, char *header );
int            hms_msg_get_header(hms_msg *msg, int index, char **value );
int            hms_msg_del_header(hms_msg *msg, int index );
int            hms_msg_num_headers( hms_msg *msg );

int            hms_msg_add_named_header( hms_msg *msg, char *key, char *value );
int            hms_msg_get_named_header( hms_msg *msg, char *key, char **value );
int            hms_msg_del_named_header( hms_msg *msg, char *key );
int            hms_msg_num_named_headers( hms_msg *msg );

int            hms_msg_get_body_size( hms_msg *msg );
int            hms_msg_get_body( hms_msg *msg, char **data, int *len );
int            hms_msg_set_body( hms_msg *msg, char *data, int len );
int            hms_msg_del_body( hms_msg *msg );


/* Util */


#endif /* end __HERMES_H__ */
