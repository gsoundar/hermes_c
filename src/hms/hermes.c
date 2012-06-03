/**
 * HERMES
 * ------
 * by Gokul Soundararajan
 *
 * A C implementation of the Hermes protocol
 *
 **/

#include <hermes.h>

/* Function prototypes */
/* ---------------------------------------------------- */
/* Manager code */
static void  _hms_listen(hms *manager);
static void _hms_handle_endpoint( hms_endpoint *endpoint );

/* Endpoint code */

/* Default client code */
static int _hms_default_validate(hms_endpoint *endpoint, hms_msg *msg);
static int _hms_default_accepts( struct hms_endpoint *endpoint, hms_msg *msg );
static int _hms_default_handle(hms_endpoint *endpoint, hms_msg *msg);

/* Hermes implementation */
/* ---------------------------------------------------- */

hms* hermes_init( int num_threads , int server_port, hms_ops ops ) {

  hms *manager = NULL;

  /* malloc space */
  manager = (hms *) malloc( sizeof(hms) );
  hms_assert_not_equals( __FILE__, __LINE__ , (uintptr_t) NULL, (uintptr_t) manager);

  /* ignore broken pipe */
  signal( SIGPIPE, SIG_IGN );

  /* initialize mutexes */
  pthread_mutex_init( &(manager->manager_lock), NULL);

  /* set defaults */
  manager->num_threads = num_threads;
  manager->shutdown = HMS_FALSE;
  manager->server_port = server_port;

  manager->dops.hms_validate = _hms_default_validate;
  manager->dops.hms_handle = _hms_default_handle;
  manager->ops = ops;

  /* create the thread pool */
  tpool_init(&manager->pool, (num_threads + 2), 10, HMS_TRUE );
  //fprintf(stdout, "created thread pool\n"); fflush(stdout);
  
  /* starts a thread to listen */
  if( server_port > 0 ) {
    hms_assert_not_equals(__FILE__, __LINE__,  -1, tpool_add_work(manager->pool, (void *) _hms_listen, (void *) manager) );    
  }

  return manager;

} /* end hermes_init */



int  hermes_shutdown( hms *manager, int force ) {

  pthread_mutex_lock( &manager->manager_lock );
  manager->shutdown = HMS_TRUE;

  /* blocks until all threads end */
  tpool_destroy( manager->pool, force );

  /* clean up memory */
  close(manager->server_socket);
  pthread_mutex_unlock( &manager->manager_lock );

  return 0;

} /* end hermes_shutdown() */

/**
 *
 * Runs in a separate thread and spawns new threads to 
 * handle connections 
 *
 **/

static void _hms_listen(hms *manager ) {

  hms_assert_not_equals( __FILE__, __LINE__, (uintptr_t) NULL, (uintptr_t) manager );

  int sockfd;
  struct sockaddr_in my_addr;
  struct sockaddr_in their_addr;
  int new_fd;
  socklen_t sin_size;
  int yes;
  int server_port = manager->server_port;
  unsigned sleep_time = 0;

 retry_listen:

  /* exponential backoff */
  if(sleep_time) sleep(sleep_time);

  if(sleep_time == 0) { sleep_time = 2; }
  else if(sleep_time == 2) { sleep_time = 2*sleep_time; }

  /* open the socket */
  if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket"); goto retry_listen;
  }
  /* set socket options */
  if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int) ) == -1) {
    perror("setsockopt"); goto retry_listen;
  }
  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons(server_port);
  my_addr.sin_addr.s_addr = INADDR_ANY;
  memset(my_addr.sin_zero, '\0', sizeof(my_addr.sin_zero));

  /* bind the socket */
  if( bind(sockfd, (struct sockaddr *) &my_addr, sizeof(my_addr) ) == -1) {
      perror("bind"); 
      goto retry_listen;
  }
  manager->server_socket = sockfd;

  /* listen on the socket */
  if(listen(sockfd, 10) == -1){
    perror("listen"); goto retry_listen;
  }

  /* spawn thread to accept connections */
  //fprintf(stdout, "waiting to accept\n"); fflush(stdout);
  while( HMS_TRUE ) {

    int flag = 1, ret;
    sin_size = sizeof(their_addr);
    if((new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size)) == -1) {
      perror("accept"); return;
    }
    ret = setsockopt( new_fd , IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag) );
    ret = setsockopt( new_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag) );
    //fprintf(stdout, "accepting new connection\n"); fflush(stdout); 

    /* shutdown */
    if(manager->shutdown) break;

    /* allocate an endpoint and add to manager */
    hms_endpoint *endpoint = hms_endpoint_init( new_fd, manager->ops );
    hms_assert_not_equals( __FILE__, __LINE__, (uintptr_t) NULL, (uintptr_t) endpoint);    

    /* spawn a thread to handle the new conn */
    hms_assert_not_equals( __FILE__, __LINE__, -1, tpool_add_work(manager->pool, (void *) _hms_handle_endpoint, (void *) endpoint) );    
    //fprintf(stdout, "spawned handler thread success\n"); fflush(stdout);

  } /* end while() */

  /* should not reach here until shutdown */
  hms_assert_not_equals( __FILE__, __LINE__,  0, manager->shutdown);

  return;

} /* end _hms_listen() */

static void _hms_handle_endpoint( hms_endpoint *endpoint ) {

  pthread_mutex_lock( &endpoint->meta_lock );

  int handler_status = 0;

  while( HMS_TRUE ) {

    /* read hms message */
    hms_msg *msg = NULL;
    int parse_status = hms_endpoint_recv_msg( endpoint, &msg );
    if(parse_status != 0) { /*fprintf(stderr, "parser failed\n");*/ break;}

    /* validate then handle */
    if( !endpoint->ops.hms_validate || endpoint->ops.hms_validate(endpoint,msg) == 0 ) {
      if( endpoint->ops.hms_accepts(endpoint,msg) == 0 ) {
	handler_status = endpoint->ops.hms_handle(endpoint,msg);
      }
      /* call default handler */
      else {
	handler_status = _hms_default_handle( endpoint, msg );
      }
    }

    /* free memory used by message */
    hms_msg_destroy( msg ); msg = NULL;

    if(handler_status != 0 ) break;
    
  } /* end while() */

  /* close the endpoint */
  hms_endpoint_destroy( endpoint );

  pthread_mutex_unlock( &endpoint->meta_lock);

  return;

} /* end _hms_handle_endpoint() */

/* Socket helpers */
/* ----------------------------------------------------- */

static int __send_all( int fd, char *buf, int len) {

  /* Check input */
  hms_assert_not_equals( __FILE__, __LINE__ , (uintptr_t) NULL, (uintptr_t) buf);
  hms_assert_not_equals( __FILE__, __LINE__ , (int) 0, (int) len);

  const char *p = buf;
  int n;
  
  do {
    if((n = send(fd, p, len, 0)) == -1)
      return n;
    len -= n;
    p += n;
  } while(len > 0);

  return p - (const char *) buf;

} /* __send_all() */

static int __recv_all(int fd, char *buf, int len) {

  /* Check input */
  hms_assert_not_equals( __FILE__, __LINE__ , (int) NULL, (int) buf);
  hms_assert_not_equals( __FILE__, __LINE__ , (int) 0, (int) len);

  char *p = buf;
  int n;

  do {
    if(( (n = read(fd, p, len)) == -1) || ( n == 0))
      return n;
    len -= n;
    p += n;
  } while(len > 0);

  return p - (const char *) buf;

}

/* Connector */
/* ----------------------------------------------------- */
hms_connector* hms_connector_init( char *hostname, int port ) {

  /* Check input */
  hms_assert_not_equals( __FILE__, __LINE__ , (uintptr_t) NULL, (uintptr_t) hostname);
  hms_assert_not_equals( __FILE__, __LINE__ , (int) 0, (int) port);

  /* Open a connection */
  int sockfd;
  struct hostent *he;
  struct sockaddr_in their_addr;
  
  /* get host info */
  if((he = gethostbyname(hostname)) == NULL) {
    return NULL;
  }
  
  /* get a socket */
  if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    return NULL;
  }
  
  their_addr.sin_family = AF_INET;
  their_addr.sin_port = htons(port);
  their_addr.sin_addr = *((struct in_addr *) he->h_addr);
  memset(their_addr.sin_zero, '\0', sizeof(their_addr.sin_zero));

  /* connect to server */
  if(connect(sockfd, (struct sockaddr *) &their_addr, sizeof(their_addr) ) == -1) {
    close(sockfd);
    return -1;
  }

  /* disable nagle */
  {
    int flag = 1;
    int ret = setsockopt( sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag) );
    hms_assert_not_equals( __FILE__, __LINE__ , (int) -1, (int) ret);
  }

  /* malloc a connector object */
  hms_connector *connector = NULL;
  connector = calloc( 1, sizeof(hms_connector) );
  hms_assert_not_equals( __FILE__, __LINE__ , (int) NULL, (int) connector);

  /* initialize the connector */
  connector->socket = sockfd;
  gettimeofday( &connector->start, NULL );
  pthread_mutex_init( &connector->meta_lock, NULL );

  return connector;

} /* end hms_connector_init() */

int hms_connector_recv_msg( hms_connector *connector, hms_msg **msg ) {

  /* Check input */
  hms_assert_not_equals( __FILE__, __LINE__ , (int) NULL, (int) connector);

  /* Receive msg */
  hms_msg *tmp_msg = NULL;
  tmp_msg = hms_msg_parse( connector->socket, HERMES_MAX_HDR_SIZE );

  /* Parsing failed */
  if(!tmp_msg) {
    *msg = NULL; return -1;
  }

  *msg = tmp_msg;
  return 0;

} /* end hms_connector_recv_msg() */

int hms_connector_send_msg( hms_connector *connector, hms_msg *msg ) {

  /* Check input */
  hms_assert_not_equals( __FILE__, __LINE__ , (int) NULL, (int) connector);
  hms_assert_not_equals( __FILE__, __LINE__ , (int) NULL, (int) msg);
  
  int erred = HMS_FALSE;

  /* Put header in bufffer to send in one shot */
  int hdr_len = 0; hdr_len = hms_msg_get_header_size( msg );
  hms_assert_not_equals( __FILE__, __LINE__ , (int) 0, (int) hdr_len);
  char *hdr_buf = malloc( hdr_len + 1 );
  hms_assert_not_equals( __FILE__, __LINE__ , (int) NULL, (int) hdr_buf);
  
  /* get header */
  if( hms_msg_print_header( msg, hdr_buf, hdr_len) == 0 ) {
    if( __send_all( connector->socket, hdr_buf, hdr_len ) == -1 ) {
      erred = HMS_TRUE;
    }
  } else { erred = HMS_TRUE; }
  free(hdr_buf); hdr_buf = NULL; hdr_len = 0;

  /* send body */
  if( hms_msg_get_body_size( msg ) > 0 ) {
    char *body = NULL; int body_len = 0;
    if( !hms_msg_get_body( msg, &body, &body_len) && body ) {
      if( __send_all( connector->socket, body, body_len ) == -1 ) {
	erred = HMS_TRUE;
      }
      free(body); body = NULL; body_len = 0;
    } else { erred = HMS_TRUE; }
  } else { erred = HMS_TRUE; }

  return (erred == HMS_FALSE) ? 0 : -1;

} /* end hms_connector_send_msg() */

int hms_connector_destroy( hms_connector *connector ) {

  /* Check input */
  hms_assert_not_equals( __FILE__, __LINE__ , (int) NULL, (int) connector);

  close(connector->socket);
  connector->socket = -1;
  connector->status = HMS_ENDPOINT_FREE;
  
  free(connector); connector = NULL;

  return 0;

} /* end hms_connector_destroy() */

/* Endpoint */
/* ---------------------------------------------------- */

hms_endpoint* hms_endpoint_init( int fd, hms_ops ops ) {

  hms_endpoint *endpoint = NULL;

  /* Malloc space */
  endpoint = calloc( 1, sizeof(hms_endpoint) );
  hms_assert_not_equals( __FILE__, __LINE__ , (uintptr_t) NULL, (uintptr_t) endpoint);

  /* Initialize the endpoint */
  endpoint->socket = fd;
  endpoint->status = HMS_ENDPOINT_FREE;
  endpoint->ops = ops;
  pthread_mutex_init( &endpoint->meta_lock, NULL );
  gettimeofday( &endpoint->start, NULL );

  /* Setup ops */
  if(ops.hms_validate == NULL ) endpoint->ops.hms_validate = _hms_default_validate;
  if(ops.hms_accepts == NULL ) endpoint->ops.hms_accepts = _hms_default_accepts;
  if(ops.hms_handle == NULL ) endpoint->ops.hms_handle = _hms_default_handle;
  

  return endpoint;

} /* end hms_endpoint_init() */

int hms_endpoint_recv_msg( hms_endpoint *endpoint, hms_msg **msg ) {

  /* Check input */
  hms_assert_not_equals( __FILE__, __LINE__ , (uintptr_t) NULL, (uintptr_t) endpoint);

  /* Receive msg */
  hms_msg *tmp_msg = NULL;
  tmp_msg = hms_msg_parse( endpoint->socket, HERMES_MAX_HDR_SIZE );

  /* Parsing failed */
  if(!tmp_msg) {
    *msg = NULL; return -1;
  }

  *msg = tmp_msg;
  return 0;

} /* end hms_endpoint_recv_msg() */

int hms_endpoint_send_msg( hms_endpoint *endpoint, hms_msg *msg ) {

  /* Check input */
  hms_assert_not_equals( __FILE__, __LINE__ , (uintptr_t) NULL, (uintptr_t) endpoint);
  hms_assert_not_equals( __FILE__, __LINE__ , (uintptr_t) NULL, (uintptr_t) msg);
  
  int erred = HMS_FALSE;

  /* Put header in bufffer to send in one shot */
  int hdr_len = 0; hdr_len = hms_msg_get_header_size( msg );
  hms_assert_not_equals( __FILE__, __LINE__ , (int) 0, (int) hdr_len);
  char *hdr_buf = malloc( hdr_len + 1 );
  hms_assert_not_equals( __FILE__, __LINE__ , (uintptr_t) NULL, (uintptr_t) hdr_buf);
  
  /* get header */
  if( hms_msg_print_header( msg, hdr_buf, hdr_len) == 0 ) {
    if( __send_all( endpoint->socket, hdr_buf, hdr_len ) == -1 ) {
      erred = HMS_TRUE;
    }
  } else { erred = HMS_TRUE; }
  free(hdr_buf); hdr_buf = NULL; hdr_len = 0;


  /* send body */
  if( hms_msg_get_body_size( msg ) > 0 ) {
    char *body = NULL; int body_len = 0;
    if( !hms_msg_get_body( msg, &body, &body_len) && body ) {
      if( __send_all( endpoint->socket, body, body_len ) == -1 ) {
	erred = HMS_TRUE;
      }
      free(body); body = NULL; body_len = 0;
    } else { erred = HMS_TRUE; }
  } /* ok if no body exists */

  return (erred == HMS_FALSE) ? 0 : -1;

} /* end hms_endpoint_send_msg() */

int hms_endpoint_destroy( hms_endpoint *endpoint ) {

  /* Check input */
  hms_assert_not_equals( __FILE__, __LINE__ , (uintptr_t) NULL, (uintptr_t) endpoint);

  close(endpoint->socket);
  endpoint->socket = -1;
  endpoint->status = HMS_ENDPOINT_FREE;
  
  free(endpoint); endpoint = NULL;

  return 0;

} /* end hms_endpoint_destroy() */



/* Default function implementations */
/* ---------------------------------------------------- */

static int _hms_default_validate( struct hms_endpoint *endpoint, hms_msg *msg ) {
  return 0; /* every msg is valid */
} /* end _hms_default_validate() */

static int _hms_default_accepts( struct hms_endpoint *endpoint, hms_msg *msg ) {
  return -1; /* user does not accept anything */
} /* end _hms_default_accepts() */

static int _hms_default_handle( struct hms_endpoint *endpoint, hms_msg *msg ) {

  int erred = HMS_FALSE;
  char *verb = NULL;
  hms_msg *reply = hms_msg_create();

  /* get the verb */
  hms_msg_get_verb( msg, &verb );
  
  if( !verb ) {
    hms_msg_set_verb(reply, "ERROR");
    hms_endpoint_send_msg( endpoint, reply );
    erred = HMS_TRUE;
  } 
  else if( strcasecmp(verb, "PING") == 0 ) {
    hms_msg_set_verb(reply, "PONG");
    hms_endpoint_send_msg( endpoint, reply );
  }
  else if( strcasecmp(verb, "INFO") == 0 ) {
    hms_msg_set_verb(reply, "INFO");
    /* return current time */
    char time_buf[30]; time_t now; struct tm tm;
    time( &now ); localtime_r( &now, &tm ); asctime_r( &tm, time_buf);
    hms_msg_add_named_header( reply, "Localtime", time_buf );
    hms_endpoint_send_msg( endpoint, reply );
  }
  else if( strcasecmp( verb, "BYE" ) == 0 ) {erred = HMS_TRUE;}
  else { erred = HMS_TRUE; }


  /* free verb */
  if(verb) { free(verb); verb = NULL; }

  /* free reply */
  hms_msg_destroy( reply );
  
  return (erred == HMS_FALSE ) ? 0 : -1;

} /* end _hms_default_handle() */

