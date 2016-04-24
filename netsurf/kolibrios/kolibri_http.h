#ifndef KOLIBRI_HTTP_H
#define KOLIBRI_HTTP_H

/* kolibri_http.h: Defines C style functions for using the underlying Kolibri HTTP library.
   Defines functions for performing HTTP and related routines. 

   Interface intended to make life of C programmers easy in order to use the http.obj library
   which serves as the HTTP library for KolibriOS on top of the network stack.

   Read description of kolibri_http_init() for more information.
   Read documentation of procedures in http.asm ($SVN/kolibrios/programs/develop/libraries/http/)
   for even more information on the procedures.
      
   Copyright hidnplayr , GPL v2
   Copyright ashmew2 , GPL v2
*/


struct http_msg{
  /* Internal library use only */
  unsigned int socket;
  unsigned int flags;
  unsigned int write_ptr;
  unsigned int buffer_length;
  unsigned int chunk_ptr;
  unsigned int timestamp;

  /* Available for use */
  unsigned int status;
  unsigned int header_length;
  char * content_ptr;
  unsigned int content_length;
  unsigned int content_received;
  unsigned int http_header;
};

/* C - assembly wrapper to initialize and load the http.obj DLL at runtime */
extern int init_network_asm(void);

/* Procedures found in http.obj */
extern struct http_msg* (*http_get_asm)(char *, struct http_msg *, unsigned int, char *) __attribute__((__stdcall__));
extern int (*http_receive_asm)(struct http_msg *) __attribute__((__stdcall__));
extern void (*http_disconnect_asm)(struct http_msg *) __attribute__((__stdcall__));
extern void (*http_free_asm)(struct http_msg *) __attribute__((__stdcall__));
extern struct http_msg* (*http_head_asm)(char *, struct http_msg *, unsigned int, char *) __attribute__((__stdcall__));
extern struct http_post* (*http_post_asm)(char *, struct http_msg *, unsigned int, char *, char *, unsigned int) __attribute__((__stdcall__));
extern int (*http_send_asm)(struct http_msg *, char *, unsigned int) __attribute__((__stdcall__));
extern char * (*http_find_header_field_asm)(struct http_msg *, char *) __attribute__((__stdcall__));
extern char * (*http_escape_asm)(char *, unsigned int) __attribute__((__stdcall__));
extern char * (*http_unescape_asm)(char *, unsigned int) __attribute__((__stdcall__));

/* A C Programmer calls *ONLY* this function for initializing http.obj library at runtime */
/* After successfully returning from kolibri_http_init(), all http_*_asm functions can be used. */
/* Returns 0 on successful init */

int kolibri_http_init(void);

#endif /* KOLIBRI_HTTP_H */
