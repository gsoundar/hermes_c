/**
 * HERMES - Test
 * -------------
 * by Gokul Soundararajan
 *
 * Message parsing tests for Hermes C edition
 *
 **/

#include <hermes.h>
#include <assert.h>

int main(int argc, char **argv) {

  int loop_count = 0;
  hms_msg *msg = NULL;
 
loop:
  
  fprintf(stdout, "iteration %d\n", loop_count );

  msg = NULL;

  /* Initialize the message */
  {
    msg = hms_msg_create();
    assert( msg );
    assert( msg->verb == NULL );
    assert( msg->num_headers == 0 );
    assert( msg->num_named_headers == 0 );
    assert( msg->content == NULL );
    assert( msg->content_len == 0 );
    fprintf(stdout, "done msg create tests\n" );
  }

  /* Add the verb */
  {
    char *verb;
    assert( hms_msg_get_verb( msg, &verb ) < 0 );

    assert( !hms_msg_set_verb( msg, "PING" ) );
    assert( !hms_msg_get_verb( msg, &verb ) );
    assert( strcmp( verb, "PING" ) == 0 );
    free( verb );

    fprintf(stdout, "done verb tests\n" );
  }

  /* Add headers */
  {
    char *value;
    assert( !hms_msg_num_headers( msg ) );
    
    assert( !hms_msg_add_header( msg, "zero" ) );
    assert( hms_msg_num_headers( msg ) == 1);
    assert( !hms_msg_get_header( msg, 0, &value) );
    assert( strcmp( value, "zero" ) == 0 );
    free(value);

    {
      int i = 0;
      char *names[6] = { "zero", "one", "two", "three", "four", "five" };
      for( i=1; i < 6; i++) {
	assert( !hms_msg_add_header( msg, names[i] ) );
	assert( hms_msg_num_headers( msg ) == (i+1));
	assert( !hms_msg_get_header( msg, i, &value) );
	assert( strcmp( value, names[i] ) == 0 );
	free(value);
      }
    }

    int num_hdrs = 0;
    assert( (num_hdrs = hms_msg_num_headers( msg )) > 0 );
    assert( !hms_msg_del_header( msg, 0 ) );
    assert( (hms_msg_num_headers( msg )) == (num_hdrs - 1) );
    assert( !hms_msg_get_header( msg, 0, &value) );
    assert( strcmp( value, "one" ) == 0 );
    free(value);
    
    while( hms_msg_num_headers( msg ) > 0 ) {
      assert( (num_hdrs = hms_msg_num_headers( msg )) > 0 );
      assert( !hms_msg_del_header( msg, 0 ) );
      assert( (hms_msg_num_headers( msg )) == (num_hdrs - 1) );
    }

    /* TODO: Deletions from back and in middle */

    fprintf(stdout, "done header tests\n" );
  }

  /* Add named headers */
  {
    
    char *keys[5] = { "key0", "key1", "key2", "key3", "key4" };
    char *vals[5] = { "val0", "val1", "val2", "val3", "val4" };
    int i = 0;
    for(i=0; i < 5; i++) {
      char *value;
      assert( hms_msg_num_named_headers( msg ) == i);
      assert( !hms_msg_add_named_header( msg, keys[i], vals[i] ) );
      assert( !hms_msg_get_named_header( msg, keys[i], &value ) );
      assert( !strcmp( value, vals[i] ) );
      free(value);
      assert( hms_msg_num_named_headers( msg ) == (i+1));
    }

    for(i=4; i >= 0; i--) {
      assert( hms_msg_num_named_headers( msg ) == (i+1) );
      assert( !hms_msg_del_named_header( msg, keys[i] ) );
    }

    /* TODO: Need better tests */

    fprintf(stdout, "done named header tests\n" );

  }

  /* Add body */
  {
    
    char body[1025];
    int i = 0;
    for(i=0; i < 5; i++) {
      int sz = 1 + random()%1024;
      char *value;
      int value_len;
      assert( !hms_msg_get_body_size( msg) );
      assert( !hms_msg_set_body( msg, body, sz ) );
      assert( !hms_msg_get_body( msg, &value, &value_len ) );
      assert( !memcmp( value, body, sz ) );
      free(value);
      assert( hms_msg_get_body_size( msg) == sz);
      assert( !hms_msg_del_body( msg ) );
      assert( !hms_msg_get_body_size( msg) );
    }
    
    fprintf(stdout, "done body tests\n" );

  }

  /* Destroy message */
  {
    assert( !hms_msg_destroy( msg ) );
    fprintf(stdout, "done msg destroy\n");
  }

  loop_count++;
  if(loop_count < 1000) goto loop;

  return 0;

} /* end main() */
