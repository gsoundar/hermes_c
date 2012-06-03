/**
 * HERMES
 * ------
 * by Gokul Soundararajan
 *
 * A C implementation of the Hermes protocol
 *
 **/

#include <hermes.h>

enum _hms_assert_type { HMS_ASSERT_EQ, HMS_ASSERT_NEQ };

/* Function prototypes */
/* -------------------------------------------------------------- */

static void _hms_assert_low( char *file, unsigned id, int expected, int condition, int type );

/* Utility functions */
/* -------------------------------------------------------------- */

void hms_assert_not_equals( char *file, unsigned id, int expected, int condition ) {
  return _hms_assert_low(file, id,expected,condition,HMS_ASSERT_NEQ);
}

void hms_assert_equals( char *file, unsigned id, int expected , int condition ) {
  return _hms_assert_low(file, id,expected,condition,HMS_ASSERT_EQ);
}

/* Internal functions */
/* -------------------------------------------------------------- */

static void _hms_assert_low( char *file, unsigned id, int expected, int condition, int type ) {
  unsigned eval = 0;
  if(type == HMS_ASSERT_EQ ) { eval = !(condition == expected); }
  else if(type == HMS_ASSERT_NEQ) { eval = !(condition != expected); }
  else { fprintf(stderr, "assert: %id failed. unknown check!!\n", id); exit(-1); }

  /* if eval != 0 then assert failed */
  if(eval) {
    fprintf(stderr, "assert: %s [line: %u] failed. expected: %d condition: %d\n", 
	    file, id, expected, condition );
    exit(-1);
  }

} /* end _hms_assert_low() */
