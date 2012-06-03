/**
 * HERMES
 * ------
 * by Gokul Soundararajan
 *
 * A C implementation of the Hermes protocol
 *
 **/

#ifndef __HERMES_INTERNAL_H__
#define __HERMES_INTERNAL_H__

#include <err.h>

struct hms_msg;

/* Assertions */
/* ---------------------------------------------------- */

void hms_assert_equals( char * name, unsigned id, int expected , int condition );
void hms_assert_not_equals( char *name, unsigned id, int expected, int condition );

/* Parser - by Daniel */
/* ---------------------------------------------------- */
struct hms_msg *hms_msg_parse( int fd , int max_hdr_len );

#endif
