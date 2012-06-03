/**
 * HERMES
 * ------
 * by Daniel Lupei and Gokul Soundararajan
 *
 * Message struct for Hermes
 *
 **/

#include <hermes.h>


/* Function prototypes */
/* -------------------------------------------------- */
static hms_msg_uheader* __hms_create_header( char * value );
static int              __hms_init_header( hms_msg_uheader *hdr, char *value );
static int              __hms_destroy_header( hms_msg_uheader *hdr );
static int              __hms_deinit_header( hms_msg_uheader *hdr );

static hms_msg_nheader* __hms_create_named_header( char *key, char *value );
static int              __hms_init_named_header( hms_msg_nheader *hdr, char *key, char *value ); 
static int              __hms_destroy_named_header( hms_msg_nheader *hdr );
static int              __hms_deinit_named_header( hms_msg_nheader *hdr );

/* Implementation */
/* -------------------------------------------------- */

/* Message */
/* -------------------------------------------------- */

hms_msg *hms_msg_create() {

  /* malloc space */
  hms_msg *msg = calloc( 1, sizeof(hms_msg) );
  hms_assert_not_equals( __FILE__, __LINE__, (uintptr_t) NULL, (uintptr_t) msg );
  
  /* initialize verb */
  msg->id = NULL;
  msg->verb = NULL;
  
  /* initialize headers/named headers */
  //__hms_init_header( &msg->headers, "" ); --buggy
  //__hms_init_named_header( &msg->named_headers, "", "" ); --buggy
  HMS_INIT_LIST_HEAD( &msg->headers.lh );
  HMS_INIT_LIST_HEAD( &msg->named_headers.lh );
  msg->num_headers = 0;
  msg->num_named_headers = 0;

  /* initialize body */
  msg->content = NULL;
  msg->content_len = 0;

  return msg;
  
} /* end hms_msg_create() */

int hms_msg_get_header_size( hms_msg *msg ) {

  int size = 0;
  hms_msg_uheader *uhdr;
  hms_msg_nheader *nhdr;

  /* Check input */
  hms_assert_not_equals( __FILE__, __LINE__, (uintptr_t) NULL, (uintptr_t) msg );

  /* if no verb then it is malformed so return 0 */
  if(!msg->verb) { return 0; }
  
  /* verb length */
  if(msg->verb) { size += strlen( msg->verb ); }
  /* header lengths */
  hms_list_for_each_entry( uhdr, &msg->headers.lh, lh) {
    size += 1 + strlen(uhdr->val);
  }
  /* end of line */
  size += 1;

  /* named headers */
  /* 1 for ":" and 1 for "\n" */
  hms_list_for_each_entry( nhdr, &msg->named_headers.lh, lh) {
    size += strlen(nhdr->key) + 1 + strlen(nhdr->val) + 1;
  }

  /* end of msg hdr */
  /* 1 for "." and 1 for "\n" */
  size+= 2;

  return size;

} /* end hms_msg_get_header_size() */

int hms_msg_print_header( hms_msg *msg, char *buffer, int len) {

  /* Check input */
  hms_assert_not_equals( __FILE__, __LINE__, (uintptr_t) NULL, (uintptr_t) msg );
  hms_assert_not_equals( __FILE__, __LINE__, (uintptr_t) NULL, (uintptr_t) buffer );

  /* Make sure buffer is big enough */
  int hdr_len = hms_msg_get_header_size( msg );
  hms_msg_uheader *uhdr;
  hms_msg_nheader *nhdr;

  /* Copy into user buffer */
  char *offset = buffer;
  
  /* verb length */
  if(msg->verb) {
    int verb_len = strlen(msg->verb);
    strncpy( offset , msg->verb, verb_len );
    offset += verb_len;
  }
  /* header lengths */
  hms_list_for_each_entry( uhdr, &msg->headers.lh, lh) {
    int hdr_len = strlen(uhdr->val);
    strncpy( offset, " ", 1); offset += 1;
    strncpy( offset, uhdr->val, hdr_len ); offset += hdr_len;
  }
  /* end of line */
  strncpy( offset, "\n", 1); offset += 1;

  /* named headers */
  /* 1 for ":" and 1 for "\n" */
  hms_list_for_each_entry( nhdr, &msg->named_headers.lh, lh) {
    int key_len = strlen(nhdr->key);
    int val_len = strlen(nhdr->val);
    strncpy( offset, nhdr->key, key_len); offset += key_len;
    strncpy( offset, ":", 1); offset += 1;
    strncpy( offset, nhdr->val, val_len); offset += val_len;
    strncpy( offset, "\n", 1); offset += 1;
  }

  /* end of msg hdr */
  /* 1 for "." and 1 for "\n" */
  strncpy( offset, ".\n", 2); offset += 2;

  hms_assert_equals( __FILE__, __LINE__, (int) hdr_len, (int) (offset-buffer) );

  return 0;

} /* end hms_msg_print_header() */

int hms_msg_destroy( hms_msg *msg ) {

  /* Check input */
  hms_assert_not_equals( __FILE__, __LINE__, (uintptr_t) NULL, (uintptr_t) msg );
  
  /* Delete content */
  if( msg->content ) {
    free(msg->content); msg->content = NULL; msg->content_len = 0;
  }

  /*Delete verb */
  if( msg->verb ) {
    free(msg->verb); msg->verb = NULL;
  }

  /* Delete headers */
  {
    hms_msg_uheader *hdr, *next;
    hms_list_for_each_entry_safe( hdr, next, &msg->headers.lh, lh) {
      hms_list_del( &hdr->lh );
      __hms_destroy_header( hdr );
      msg->num_headers--;
    }
    hms_assert_equals( __FILE__, __LINE__, (int) 0, (int) msg->num_headers );
  }

  /* Delete named headers */
  {
    hms_msg_nheader *hdr, *next;
    hms_list_for_each_entry_safe( hdr, next, &msg->named_headers.lh, lh) {
      hms_list_del( &hdr->lh );
      __hms_destroy_named_header( hdr );
      msg->num_named_headers--;
    }
    hms_assert_equals( __FILE__, __LINE__, (int) 0, (int) msg->num_named_headers );
  }

  /* Delete the message itself */
  free( msg ); msg = NULL;

  return 0;

} /* end hms_msg_destroy() */

/* Verb */
/* -------------------------------------------------- */

int hms_msg_set_verb(hms_msg *msg, char *verb ) {

  /* Check inputs */
  hms_assert_not_equals( __FILE__, __LINE__, (uintptr_t) NULL, (uintptr_t) msg );
  hms_assert_not_equals( __FILE__, __LINE__, (uintptr_t) NULL, (uintptr_t) verb );

  /* replaces existing verb */
  if( msg->verb ) { free( msg->verb ); msg->verb = NULL;}
  hms_assert_equals( __FILE__, __LINE__, (uintptr_t) NULL, (uintptr_t) msg->verb );

  /* add new verb */
  msg->verb = strdup( verb );
  hms_assert_not_equals( __FILE__, __LINE__, (uintptr_t) NULL, (uintptr_t) msg->verb );

  return 0;

} /* end hms_msg_set_verb() */

int hms_msg_get_verb(hms_msg *msg, char **verb ) {

  /* Check inputs */
  hms_assert_not_equals( __FILE__, __LINE__, (uintptr_t) NULL, (uintptr_t) msg );

  /* make sure verb exists */
  if( !msg->verb ) { *verb = NULL; return -1; }

  /* copy verb to user */
  hms_assert_not_equals( __FILE__, __LINE__, (uintptr_t) NULL, (uintptr_t) msg->verb );
  *verb = strdup( msg->verb );
  return 0;

} /* end hms_msg_get_verb() */

/* Headers */
/* -------------------------------------------------- */

int hms_msg_add_header(hms_msg *msg, char *value ) {
  
  /* Check inputs */
  hms_assert_not_equals( __FILE__, __LINE__, (uintptr_t) NULL, (uintptr_t) msg );
  hms_assert_not_equals( __FILE__, __LINE__, (uintptr_t) NULL, (uintptr_t) value );

  /* create header */
  hms_msg_uheader *hdr = __hms_create_header( value );
  
  /* add to list */
  hms_list_add_tail( &hdr->lh, &msg->headers.lh );
  msg->num_headers++;
  return 0;

} /* end hms_msg_add_header() */

int hms_msg_get_header(hms_msg *msg, int index , char **value) {

  /* Check inputs */
  hms_assert_not_equals( __FILE__, __LINE__, (uintptr_t) NULL, (uintptr_t) msg );

  /* index must be in bounds */
  if( msg->num_headers <= 0 || index < 0 || index >= msg->num_headers ) {
    *value = NULL; return -1;
  }

  /* find the entry */
  int i = 0;
  hms_msg_uheader *hdr = NULL;
  hms_list_for_each_entry( hdr, &msg->headers.lh, lh) {
    if(i++ == index) break;
  }
  
  *value = strdup( hdr->val );
  return 0;

} /* end hms_msg_get_header() */


int hms_msg_del_header(hms_msg *msg, int index ) {

  /* Check inputs */
  hms_assert_not_equals( __FILE__, __LINE__, (uintptr_t) NULL, (uintptr_t) msg );

  /* index must be in bounds */
  if( msg->num_headers <= 0 || index < 0 || index >= msg->num_headers ) {
    return -1;
  }

  /* Delete the header */
  int i = 0;
  hms_msg_uheader *hdr = NULL, *tmp;
  hms_list_for_each_entry_safe(hdr, tmp, &msg->headers.lh, lh) {
    if( i++ == index) {
      hms_list_del( &hdr->lh );
      __hms_destroy_header( hdr );
      msg->num_headers--;
      break;
    }
  }

  return 0;

} /* end hms_msg_del_header() */

int hms_msg_num_headers( hms_msg *msg ) {
  
  /* Check input */
  hms_assert_not_equals( __FILE__, __LINE__, (uintptr_t) NULL, (uintptr_t) msg );

  return msg->num_headers;

} /* end hms_msg_num_headers() */

/* Named Headers */
/* -------------------------------------------------- */

int hms_msg_add_named_header( hms_msg *msg, char *key, char *value ) {

  /* Check input */
  hms_assert_not_equals( __FILE__, __LINE__, (uintptr_t) NULL, (uintptr_t) msg );
  hms_assert_not_equals( __FILE__, __LINE__, (uintptr_t) NULL, (uintptr_t) key );
  hms_assert_not_equals( __FILE__, __LINE__, (uintptr_t) NULL, (uintptr_t) value );

  /* Delete old value if it exists */
  hms_msg_del_named_header( msg, key );

  /* Malloc space for header */
  hms_msg_nheader *hdr = __hms_create_named_header( key, value );
  hms_assert_not_equals( __FILE__, __LINE__, (uintptr_t) NULL, (uintptr_t) hdr );
  
  /* Add to list */
  hms_list_add_tail( &hdr->lh, &msg->named_headers.lh);
  msg->num_named_headers++;

  return 0;

} /* end hms_msg_add_named_header() */

int hms_msg_get_named_header( hms_msg *msg, char *key, char **value ) {

  /* Check input */
  hms_assert_not_equals( __FILE__, __LINE__, (uintptr_t) NULL, (uintptr_t) msg );
  hms_assert_not_equals( __FILE__, __LINE__, (uintptr_t) NULL, (uintptr_t) key );

  /* Search through the headers */
  hms_msg_nheader *hdr = NULL;
  unsigned found = HMS_FALSE;
  hms_list_for_each_entry( hdr, &msg->named_headers.lh, lh) {
    if(strcasecmp( hdr->key, key) == 0) { found = HMS_TRUE; break; }
  }

  /* Return value */
  if(found && hdr) {
    *value = strdup( hdr->val );
    return 0;
  }

  /* Nothing matched */
  *value = NULL;
  return -1;

} /* end hms_msg_get_named_header() */

int hms_msg_del_named_header( hms_msg *msg, char *key ) {

  /* Check inputs */
  hms_assert_not_equals( __FILE__, __LINE__, (uintptr_t) NULL, (uintptr_t) msg );
  hms_assert_not_equals( __FILE__, __LINE__, (uintptr_t) NULL, (uintptr_t) key );

  /* Delete the header */
  hms_msg_nheader *hdr = NULL, *tmp;
  hms_list_for_each_entry_safe(hdr, tmp, &msg->named_headers.lh, lh) {
    if( strcasecmp(hdr->key, key) == 0) {
      hms_list_del( &hdr->lh );
      __hms_destroy_named_header( hdr );
      msg->num_named_headers--;
      break;
    }
  }

  return 0;

} /* end hms_msg_del_named_header() */

int hms_msg_num_named_headers( hms_msg *msg ) {
  
  /* Check input */
  hms_assert_not_equals( __FILE__, __LINE__, (uintptr_t) NULL, (uintptr_t) msg );
  
  return msg->num_named_headers;

} /* end hms_msg_get_num_named_headers() */

/* Content */
/* -------------------------------------------------- */

int hms_msg_get_body_size( hms_msg *msg ) {

  /* Check inputs */
  hms_assert_not_equals( __FILE__, __LINE__, (uintptr_t) NULL, (uintptr_t) msg );
  
  /* Get Content-Length */
  unsigned content_len = 0;
  char *value;
  if( 0 == hms_msg_get_named_header( msg, HMS_CONTENT_LENGTH, &value ) ) {
    content_len = atoi( value );
    free( value );
    hms_assert_equals( __FILE__, __LINE__, (int) msg->content_len, (int) content_len );
    return msg->content_len;
  }

  /* If there is no named_header with CONTENT-LENGTH 
     then the variable better be zero ! */
  hms_assert_equals( __FILE__, __LINE__, (int) 0, (int) msg->content_len );

  return 0;
      
} /* end hms_msg_get_body_size() */

int hms_msg_get_body( hms_msg *msg, char **data, int *len) {

  /* Check inputs */
  hms_assert_not_equals( __FILE__, __LINE__, (uintptr_t) NULL, (uintptr_t) msg );

  /* Check if content exists  */
  if( msg->content ) {

    char *value;

    /* make sure named header matches actual length */
    int ret = hms_msg_get_named_header( msg, HMS_CONTENT_LENGTH, &value );
    hms_assert_equals( __FILE__, __LINE__, (int) 0, (int) ret );
    unsigned content_len = atoi( value );
    free( value );
    hms_assert_equals( __FILE__, __LINE__, (int) msg->content_len, (int) content_len );

    /* copy to user */
    *data = ( char *) malloc( msg->content_len );
    hms_assert_not_equals( __FILE__, __LINE__, (uintptr_t) NULL, (uintptr_t) *data );
    memcpy( *data, msg->content, msg->content_len );
    *len = msg->content_len;

    return 0;

  }

  *data = NULL;
  *len = 0;

  return 0;

} /* end hms_msg_get_body() */

int hms_msg_set_body( hms_msg *msg, char *data, int len ) {

  /* Check inputs */
  hms_assert_not_equals( __FILE__, __LINE__, (uintptr_t) NULL, (uintptr_t) msg );
  hms_assert_not_equals( __FILE__, __LINE__, (uintptr_t) NULL, (uintptr_t) data );
  hms_assert_not_equals( __FILE__, __LINE__, (int) 0, (int) len );

  /* Delete old content */
  hms_msg_del_body( msg );

  /* Trust what the user provided */
  hms_assert_equals( __FILE__, __LINE__, (uintptr_t) NULL, (uintptr_t) msg->content );
  hms_assert_equals( __FILE__, __LINE__, (int) 0, (int) msg->content_len );

  /* Copy the data in and also add named header */
  msg->content = ( char * ) malloc( len );
  hms_assert_not_equals( __FILE__, __LINE__, (uintptr_t) NULL, (uintptr_t) msg->content );
  memcpy( msg->content, data, len );
  msg->content_len = len;

  /* named header */
  {
    char value[256];
    sprintf( value, "%d", len );
    hms_assert_equals( __FILE__, __LINE__, (int) 0, 
		       hms_msg_add_named_header( msg, HMS_CONTENT_LENGTH, value ) 
		       );
  }

  /* add checksum */
#ifdef HERMES_ENABLE_CHECKSUMS  
  {
    char hex_checksum[16], computed_checksum[33];
    md5_buffer( msg->content , msg->content_len, (void *) hex_checksum );
    md5_sig_to_string( hex_checksum, computed_checksum, 33);
    hms_assert_equals( __FILE__, __LINE__, (int) 0, 
		       hms_msg_add_named_header( msg, HMS_CONTENT_CHECKSUM, computed_checksum ) 
		       );
  }
#endif

  return 0;

} /* end hms_msg_set_body() */

int hms_msg_del_body( hms_msg *msg ) {

  /* Check inputs */
  hms_assert_not_equals( __FILE__, __LINE__, (uintptr_t) NULL, (uintptr_t) msg );

  if( msg->content ) {

    char *value;

    /* free old content */
    free( msg->content );

    /* make sure named header matches actual length */
    int ret = hms_msg_get_named_header( msg, HMS_CONTENT_LENGTH, &value );
    hms_assert_equals( __FILE__, __LINE__, (int) 0, (int) ret );
    unsigned content_len = atoi( value );
    free( value );
    hms_assert_equals( __FILE__, __LINE__, (int) msg->content_len, (int) content_len );

    /* remove named header */
    ret = hms_msg_del_named_header( msg, HMS_CONTENT_LENGTH );
    hms_assert_equals( __FILE__, __LINE__, (int) 0, (int) ret );
    ret = hms_msg_del_named_header( msg, HMS_CONTENT_CHECKSUM );
    hms_assert_equals( __FILE__, __LINE__, (int) 0, (int) ret );

    msg->content = NULL;
    msg->content_len = 0;

  }
  
  return 0;

} /* end hms_msg_del_body() */

/* Helper Functions */
/* -------------------------------------------------- */

static hms_msg_uheader* __hms_create_header( char * value ) {

  /* Check input */
  hms_assert_not_equals( __FILE__ , __LINE__ , (uintptr_t) NULL, (uintptr_t) value );  

  /* malloc space */
  hms_msg_uheader *hdr = calloc( 1, sizeof(hms_msg_uheader) );
  hms_assert_not_equals( __FILE__ , __LINE__ , (uintptr_t) NULL, (uintptr_t) hdr );

  /* initialize it */
  __hms_init_header( hdr, value );

  return hdr;

} /* end __hms_create_header() */

static int __hms_init_header( hms_msg_uheader *hdr, char *value ) {

  /* Check input */
  hms_assert_not_equals( __FILE__ , __LINE__ , (uintptr_t) 0, (uintptr_t) hdr );  
  hms_assert_not_equals( __FILE__ , __LINE__ , (uintptr_t) 0, (uintptr_t) value );  

  /* set the values */
  hdr->val = strdup( value );
  HMS_INIT_LIST_HEAD( &hdr->lh );

  return 0;

} /* end __hms_init_header() */

static int __hms_deinit_header( hms_msg_uheader *hdr ) {

  /* Check inputs */
  hms_assert_not_equals( __FILE__ , __LINE__ , (uintptr_t) NULL, (uintptr_t) hdr );  
  hms_assert_not_equals( __FILE__ , __LINE__ , (uintptr_t) NULL, (uintptr_t) hdr->val );  
  
  free( hdr->val );

  return 0;

} /* end __hms_deinit_header() */


static int __hms_destroy_header( hms_msg_uheader *hdr ) {

  /* make sure header is alloc-ed */
  hms_assert_not_equals( __FILE__ , __LINE__ , (uintptr_t) NULL, (uintptr_t) hdr );
  hms_assert_not_equals( __FILE__ , __LINE__ , (uintptr_t) NULL, (uintptr_t) hdr->val );

  /* free value */
  __hms_deinit_header( hdr );
  
  /* free hdr itself */
  free( hdr );

  return 0;

} /* end __hms_destroy_header() */

static hms_msg_nheader* __hms_create_named_header( char *key, char *value ) {

  /* Check inputs */
  hms_assert_not_equals( __FILE__ , __LINE__ , (uintptr_t) NULL, (uintptr_t) key );
  hms_assert_not_equals( __FILE__ , __LINE__ , (uintptr_t) NULL, (uintptr_t) value );

  /* Malloc space for header */
  hms_msg_nheader *hdr = calloc( 1, sizeof(hms_msg_nheader) );
  hms_assert_not_equals( __FILE__ , __LINE__ , (uintptr_t) NULL, (uintptr_t) hdr );

  /* Initialize it */
  __hms_init_named_header( hdr, key, value );

  return hdr;

} /* end __hms_create_named_header() */

static int __hms_init_named_header( hms_msg_nheader *hdr, char *key, char *value ) {

  /* Check inputs */
  hms_assert_not_equals( __FILE__ , __LINE__ , (uintptr_t) NULL, (uintptr_t) hdr );
  hms_assert_not_equals( __FILE__ , __LINE__ , (uintptr_t) NULL, (uintptr_t) key );
  hms_assert_not_equals( __FILE__ , __LINE__ , (uintptr_t) NULL, (uintptr_t) value );

  hdr->key = strdup( key );
  hdr->val = strdup( value );
  HMS_INIT_LIST_HEAD( &hdr->lh );

  /* Make sure it copied correctly */
  hms_assert_not_equals( __FILE__ , __LINE__ , (uintptr_t) NULL, (uintptr_t) hdr->key );
  hms_assert_not_equals( __FILE__ , __LINE__ , (uintptr_t) NULL, (uintptr_t) hdr->val );

  return 0;

} /* end __hms_init_named_header() */

static int __hms_destroy_named_header( hms_msg_nheader *hdr ) {

  /* make sure header is alloc-ed */
  hms_assert_not_equals( __FILE__ , __LINE__ , (uintptr_t) NULL, (uintptr_t) hdr );
  hms_assert_not_equals( __FILE__ , __LINE__ , (uintptr_t) NULL, (uintptr_t) hdr->key );
  hms_assert_not_equals( __FILE__ , __LINE__ , (uintptr_t) NULL, (uintptr_t) hdr->val );

  /* free value */
  __hms_deinit_named_header( hdr );
  
  /* free hdr itself */
  free( hdr );

  return 0;


} /* end __hms_destroy_named_header() */

static int __hms_deinit_named_header( hms_msg_nheader *hdr ) {

  /* Check inputs */
  hms_assert_not_equals( __FILE__ , __LINE__ , 0, (uintptr_t) hdr );  
  hms_assert_not_equals( __FILE__ , __LINE__ , 0, (uintptr_t) hdr->key );  
  hms_assert_not_equals( __FILE__ , __LINE__ , 0, (uintptr_t) hdr->val );  
  
  free( hdr->key );
  free( hdr->val );

  return 0;

} /* end __hms_deinit_named_header() */
