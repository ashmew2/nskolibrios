#include "utils/nsurl.h"
#include "utils/corestrings.h"
#include "http_kolibri.h"
#include "kolibrios/kolibri_http.h"

#include "content/fetch.h"

#include <stdio.h>

/* TODO: Make these symbols static as they don't need to be exported. Only the register function needs to be non static */

bool GLOBAL_DESTROY = false;

FILE *xfile = NULL;

int global_poll_counter = 0;

char *post_multipart_boundary = "-----------------------------55554652820854685881745586916\r\n";
char *post_type_urlencoded = "application/x-www-form-urlencoded";
char *post_type_multipart = "multipart/form-data; boundary=-----------------------------55554652820854685881745586916";

bool HEADER_DEBUG = true;

typedef enum {SEND, RECEIEVE} http_direction;
typedef enum {HTTP_GET, HTTP_POST} http_type;

struct multipart_data_fragment {
  bool is_file;
  char *header_line;
  char *data_or_filename;
  int length_header;
  int length;
  struct multipart_data_fragment *next;
};

/* FIXME: Reorder this struct members logically  */
struct kolibri_fetch {
  struct fetch *fetch_handle;
  struct http_msg *http_handle;
  struct kolibri_fetch *next_kolibri_fetch;
  char *urlenc_data;
  struct multipart_data_fragment *multipart_data;
  int multipart_content_length;
  char *headers;
  http_type fetch_type;
  http_direction transfer_direction;
  char *url_string;
  char *location;  /* Only useful when redirected. */
  char *cookie_string;
  char *realm;
  size_t url_length;
  int content_length;
  unsigned int content_transferred;
  bool redirected;
  bool abort;
  bool finished;
  bool data_processed;
  bool transfer_complete;
  bool headers_processed;
};

void process_headers(struct kolibri_fetch *);
struct fetch_multipart_data* multipart_convert(struct fetch_multipart_data *control, int *content_length);

struct kolibri_fetch *fetcher_head;
/* http_handle for this dummy pointer is always NULL */

struct http_kolibri_multi_handle* fetch_http_kolibri_multi;

/* In case we need to do something fancy for setting up the http handle */
bool http_kolibri_handle_init(void)
{
  /* Do stuff about setting up the HTTP handle here */
  /* Since http.obj just allocates a handle with http_get, I think we can rely on the lib to provide a handle */
  /* The problem is that we need to return a new handle here.. or maybe not. */

  return true;
}

/* void print_linked_list(void) { */
/*   debug_board_write_str("Start of LL: \n"); */
/*   struct kolibri_fetch *f = fetcher_head; */
/*   char addp[40]; */

/*   while(f!=NULL) { */
/*     sprintf(addp, "%p\n", (void *)f); */
/*     debug_board_write_str(addp); */
/*     f = f->next_kolibri_fetch; */
/*   } */

/*   debug_board_write_str("end of LL: \n====\n"); */
/* } */


int http_kolibri_multi_fdset(struct http_kolibri_multi_handle *multi_handle,
			   fd_set *read_fd_set,
			   fd_set *write_fd_set,
			   fd_set *exc_fd_set, int *max_fd)
{

}


bool fetch_http_kolibri_initialise(lwc_string *scheme) {
  //debug_board_write_str("Initializing HTTP fetcher for Netsurf on Kolibri.\n");

  /* Should these be in register() instead? . Depends on if register is called once or this is called once */

  fetcher_head = malloc(sizeof(struct kolibri_fetch));
  fetcher_head -> http_handle = NULL;
  fetcher_head -> next_kolibri_fetch = NULL;

  char addr[30];
  sprintf(addr, "[init]fetcher_head: %p\n", (void *)fetcher_head);
  //debug_board_write_str(addr);


  //debug_board_write_str("Initialed HTTP fetcher for Netsurf on Kolibri.\n");
  return true;
}

bool fetch_http_kolibri_can_fetch(const struct nsurl *url) {

  /* This simply means that we want to make sure NSURL has a HOST component */
  /*FIXME  : 1 is NSURL_SCHEME_HTTP. The enum should be accessible from nsurl.h IMO.*/

  /* FIXME : Use lwc_string_equals or something here for accurate results. */

 if(nsurl_get_component(url, NSURL_SCHEME) == corestring_lwc_http)
    return true;
  else {
    debug_board_write_str("Error: Only HTTP scheme supported. No HTTPS!");
    /* __asm__ __volatile__("int3"); */
    return false;
    }
}

void * fetch_http_kolibri_setup(struct fetch *parent_fetch, struct nsurl *url,
		bool only_2xx, bool downgrade_tls, const char *post_urlenc,
		const struct fetch_multipart_data *post_multipart,
		const char **headers) {

    xfile = fopen("/tmp0/1/poller.txt", "a");
    fprintf(xfile, "Into setup() fetch\n");
    fclose(xfile);


  //debug_board_write_str("HTTP SETUP\n\n");

/* XXXXXXXXXXXXX */
  /* if(post_urlenc!=NULL) { */
  /*   debug_board_write_str("\n"); */
  /*   debug_board_write_str(post_urlenc); */
  /* } */

  /* struct fetch_multipart_data *temp = post_multipart; */
  /* while(temp != NULL) { */
  /*   debug_board_write_str(temp->name); */
  /*   debug_board_write_str("\n"); */
  /*   temp = temp->next; */
  /* } */

    xfile = fopen("/tmp0/1/setup.txt", "a");
    fprintf(xfile, "urlenc data on setup: : %s\n", post_urlenc == NULL ? "NULL" : post_urlenc);
    fprintf(xfile, "multipart data : %s\n\n", post_multipart == NULL? "NULL" : "SOMETHING");
    fclose(xfile);

  /* XXXXXXXXXXXXX */
  struct kolibri_fetch *new_kfetcher = malloc(sizeof(struct kolibri_fetch));
  char addr[100];

  new_kfetcher -> next_kolibri_fetch = NULL;
  new_kfetcher -> fetch_handle = parent_fetch;
  new_kfetcher -> http_handle = NULL;
  new_kfetcher -> finished = false;
  new_kfetcher -> abort = false;
  new_kfetcher -> data_processed = false;
  new_kfetcher -> transfer_complete = false;
  new_kfetcher -> headers_processed = false;
  new_kfetcher -> url_string = NULL;
  new_kfetcher -> redirected = false;
  new_kfetcher -> cookie_string = NULL;
  new_kfetcher -> location = NULL;
  new_kfetcher -> realm = NULL;
  new_kfetcher -> content_transferred = 0;
  new_kfetcher -> multipart_data = NULL;
  new_kfetcher -> multipart_content_length = 0;
  new_kfetcher -> urlenc_data = NULL;
  
  char *header_creator = new_kfetcher->headers;
  int total_length_headers = 0;
  int header_count = 0;

  for (header_count = 0; headers[header_count] != NULL; header_count++)
    total_length_headers += strlen(headers[header_count]);

  if(header_count!=0) { /* We found some headers */
    //debug_board_write_str("header_count != 0");
    /* __asm__ __volatile__("int3"); */

    int j;

    /*Include CRLF for each header line */
    /* +1 for the NULL byte */

    total_length_headers += (2*header_count + 1);

    new_kfetcher->cookie_string = urldb_get_cookie(url, true);

    if(new_kfetcher->cookie_string)
      total_length_headers += (8 + strlen(new_kfetcher->cookie_string) + 2);

    header_creator = (char *)malloc(total_length_headers);

    if(header_creator == NULL) {
      debug_board_write_str("Error: Could not allocate header string.\n");
      /* __asm__ __volatile__("int3"); */

      return NULL;
    }

    new_kfetcher -> headers = header_creator;

    //debug_board_write_str("Setting up headers.\n");

    /* FIXME: Eliminate this call to strlen as we already did it once bfore */
    for(j = 0; headers[j] != NULL; j++) {
      int headerlen = strlen(headers[j]);
      strncpy(header_creator, headers[j], headerlen);
      header_creator+=headerlen;
      *header_creator = '\n';
      header_creator++;
      *header_creator = '\r';
      header_creator++;
    }

    //debug_board_write_str("Looking for cookie now.\n");

    /* If we have a cookie set for this URL, use it */
    if(new_kfetcher->cookie_string) {
      //debug_board_write_str("Found cookie!");
      //debug_board_write_str(new_kfetcher->cookie_string);
      //debug_board_write_str("\n");

      /* __asm__ __volatile__("int3"); */

      strcpy(header_creator, "Cookie: ");
      header_creator += 8;
      strcpy(header_creator, new_kfetcher->cookie_string);
      header_creator += strlen(new_kfetcher->cookie_string);
    }

    *header_creator = 0;

    /* char datax[500]; */
    /* sprintf(datax, "header_creator address : %p\n", header_creator); */
    /* debug_board_write_str(datax); */
    /* sprintf(datax, "new_kfetcher->headers address : %p\n", new_kfetcher -> headers); */
    /* debug_board_write_str(datax); */
    /* debug_board_write_str(new_kfetcher -> headers); */

    /* __asm__ __volatile__("int3"); */
  }
  else
    new_kfetcher -> headers = NULL;

  /* If we have either of these, we need to do a post request */
  /* FIXME: Add multipart data as well */

  if(post_multipart) {
    debug_board_write_str("Setting up multipart.\n\n");
    /* __asm__ __volatile__("int3"); */
    new_kfetcher -> fetch_type = HTTP_POST;
    new_kfetcher -> multipart_data = multipart_convert(post_multipart, &(new_kfetcher->multipart_content_length));
    new_kfetcher -> transfer_direction = SEND;
    debug_board_write_str("Done with setting multipart.\n\n");

    /* __asm__ __volatile__("int3"); */
  }
  else if (post_urlenc) {
    int i;
    new_kfetcher -> fetch_type = HTTP_POST;
    new_kfetcher -> urlenc_data = strdup(post_urlenc);
    new_kfetcher -> transfer_direction = SEND;
    new_kfetcher -> content_length = strlen(post_urlenc);

  }
  else {
    new_kfetcher -> fetch_type = HTTP_GET;
    new_kfetcher -> urlenc_data = NULL;
    new_kfetcher -> transfer_direction = RECEIEVE;
    new_kfetcher -> content_length = 0;
  }

  /* We will initilize the HTTP handle in _start() function later when required */

  /* Set the URL for the new kolibri fetch*/
  char *url_safety = new_kfetcher->url_string;
  size_t url_len;

  if(nsurl_get_utf8(url, &url_safety, &url_len) != NSERROR_OK)
    {
      debug_board_write_str("nsurl_get_utf8 failed!");
      exit(3);
    }

  new_kfetcher->url_length = url_len;
  new_kfetcher->url_string = url_safety;

  /* sprintf(addr, "[Setup hh]Address: %p\n", (void *)(new_kfetcher->http_handle)); */
  /* debug_board_write_str(addr); */

    xfile = fopen("/tmp0/1/poller.txt", "a");
    fprintf(xfile, "Leaving setup() fetch\n");
    fclose(xfile);


  return new_kfetcher;
}

bool fetch_http_kolibri_start(void *kfetch) {
  /* debug_board_write_str("HTTP_START : "); */
  struct kolibri_fetch *fetch = (struct kolibri_fetch *)kfetch;

    xfile = fopen("/tmp0/1/poller.txt", "a");
    fprintf(xfile, "Into start() fetch\n");
    fclose(xfile);

  /* FIXME: add true or false return depending upon http_get_asm return value */

  /* debug_board_write_str(fetch->url_string); */
  /* debug_board_write_str("\n"); */

  if(fetch -> urlenc_data) {
    /* debug_board_write_str("Trying POST\n"); */
    xfile = fopen("/tmp0/1/mpart.txt", "a");
    fprintf(xfile, "urlenc data on handle : %s\n", fetch->urlenc_data);
    fprintf(xfile, "Content length for urlenc : %l", fetch->content_length);
    fclose(xfile);

    fetch->http_handle = http_post_asm(fetch->url_string, NULL, 0, fetch->headers,
				       post_type_urlencoded,
				       strlen(fetch->urlenc_data));
  }
  else if(fetch -> multipart_data) {
    xfile = fopen("/tmp0/1/mpart.txt", "a");
    fprintf(xfile, "Multipart on handle : %p\n", fetch->http_handle);
    fprintf(xfile, "Content length for MP : %d\n", fetch->multipart_content_length);
    fclose(xfile);


    debug_board_write_str("Sending headers for multipart\n\n");
    /* __asm__ __volatile__("int3"); */
    char datax[100];
    sprintf(datax, "Content->length = %d\n", fetch->multipart_content_length);
    debug_board_write_str(datax);


    
    fetch->http_handle = http_post_asm(fetch->url_string, NULL, 0, fetch->headers,
				       post_type_multipart,
				       fetch->multipart_content_length);

    debug_board_write_str("Done Sending headers for multipart\n\n");
    /* __asm__ __volatile__("int3"); */
  }
  else {
    /* debug_board_write_str("Trying GET\n"); */
    fetch->http_handle = http_get_asm(fetch->url_string, 0, 0, fetch->headers);
  }

  if(fetch -> http_handle == NULL) {
    debug_board_write_str("Failed to allocate http handle. Aborting.\n");
    return false;
  }
  else
    {
      /* char addr[400]; */
      /* sprintf(addr, "for URL: %s, Address: %p\n", fetch->url_string, (void*)(fetch->http_handle)); */
      /* debug_board_write_str(addr); */
    }

  /* Add this fetcher to the fetch ring */
  struct kolibri_fetch *f = fetcher_head;
  while(f->next_kolibri_fetch!=NULL)
    f = f->next_kolibri_fetch;

  f->next_kolibri_fetch = kfetch;

    xfile = fopen("/tmp0/1/poller.txt", "a");
    fprintf(xfile, "Leaving start() fetch\n");
    fclose(xfile);



  return true;
}

void fetch_http_kolibri_abort(void *fetch) {

    xfile = fopen("/tmp0/1/poller.txt", "a");
    fprintf(xfile, "Into abort() fetch\n");
    fclose(xfile);


  struct kolibri_fetch* f = (struct kolibri_fetch *)fetch;

  assert(f);

  f->abort = true;

  /* debug_board_write_str("kolibri_http_abort called.!!\n"); */

  fetch_remove_from_queues(f->fetch_handle);
  /* FIXME: Write a function to remove handle from our linked list */
  /* Call it here in abort because we don't want to poll them anymore */

  /* fetch_free(f->fetch_handle); */
  /* fetch_http_kolibri_free(fetch); */

    xfile = fopen("/tmp0/1/poller.txt", "a");
    fprintf(xfile, "Leaving abort() fetch\n");
    fclose(xfile);

}

void fetch_http_kolibri_free(void *kfetch) {

  /* debug_board_write_str("Inside fetch_http_kolibri_free!!!!\n"); */

  struct kolibri_fetch *kfetch_to_free = (struct kolibri_fetch *)kfetch;
  struct kolibri_fetch *delete_ptr = fetcher_head;

    xfile = fopen("/tmp0/1/poller.txt", "a");
    fprintf(xfile, "Into free() fetch\n");
    fclose(xfile);

  /* debug_board_write_str("Calling http_free_asm\n"); */

  if(kfetch_to_free && kfetch_to_free->http_handle)
    http_free_asm(kfetch_to_free -> http_handle);
  else
    {
    /* debug_board_write_str("Fetch already freed. Skipping\n.\n"); */
    return;
    }

  /* Remove this fetcher handle from the linked list */

  while(delete_ptr!=NULL) {
    if(delete_ptr->next_kolibri_fetch == kfetch_to_free)
      {
	/* char addr[30]; */
	/* sprintf(addr, "[free]kfetcher: %p\n", (void *)fetcher_head); */
	/* debug_board_write_str(addr); */

	delete_ptr->next_kolibri_fetch = kfetch_to_free->next_kolibri_fetch;
	/* debug_board_write_str("kfetch removed from Linked List\n"); */

	/* if(delete_ptr->next_kolibri_fetch == NULL) { */
	/*   /\* debug_board_write_str("next is now NULL!"); *\/ */
	/*   /\* if(fetcher_head->next_kolibri_fetch == NULL) *\/ */
	/*   /\*   debug_board_write_str("FETCHER HEAD NEXT IS NULL!\n"); *\/ */
	/*   /\* else *\/ */
	/*   /\*   debug_board_write_str("NOOOOOOOOOOOOOOOOOOOOOOOOOOOOT NULL FETCHER HEAD\n"); *\/ */
	/* } */

	/* else */
	/*   debug_board_write_str("next is not NULL!"); */

	break;
      }
    delete_ptr=delete_ptr->next_kolibri_fetch;
  }

  free(kfetch_to_free);

    xfile = fopen("/tmp0/1/poller.txt", "a");
    fprintf(xfile, "Leaving free() fetch\n");
    fclose(xfile);

  //  print_linked_list();
  //  ;
  /* debug_board_write_str("Leaving kolibri_fetch_free\n"); */
  //  ;
}

void fetch_http_kolibri_poll(lwc_string *scheme) {

  /* Need a global linked list of all http fetchers here */
  /* All those http fetchers are called with http_receive_asm here */
  //  debug_board_write_str("Inside fetch_http_kolibri_poll\n");
  /* Assume all schemes are HTTP for now */
  struct kolibri_fetch *poller = fetcher_head -> next_kolibri_fetch;

  while(poller!=NULL)
    {
      if(poller->abort == true) {
	poller = poller->next_kolibri_fetch;
	continue;
      }

      if(poller->transfer_complete == false) {

	if(poller->transfer_direction == RECEIEVE) {
	  char datax[500];
	  sprintf(datax, "Receiving for: %s with http at %p\n", poller->url_string, poller->http_handle);
	  debug_board_write_str(datax);

	  /* if(GLOBAL_DESTROY == true) { debug_board_write_str("XX1\n"); __asm__ __volatile__("int3"); } */
	  int x = http_receive_asm(poller->http_handle);

	  if(!poller->headers_processed && (poller->http_handle->flags & HTTP_GOT_HEADER))
	    {
	      debug_board_write_str("Process Headers for : \n");
	      debug_board_write_str(poller->url_string);
	      debug_board_write_str("\n");
	      /* if(GLOBAL_DESTROY == true) { debug_board_write_str("XX2\n"); __asm__ __volatile__("int3"); } */
	      process_headers(poller);
	    }

    xfile = fopen("/tmp0/1/poller.txt", "a");
    fprintf(xfile, "FINISHED fetch->http handle : %p\n", poller->http_handle == NULL ? 0 : poller->http_handle);
    fclose(xfile);

	  if(x == 0)
	    {
	      /* if(HEADER_DEBUG == true) { */
	      /* 	char datax[100]; */
	      /* 	sprintf(datax, "Flags are : %u\n", poller->http_handle->flags); */
	      /* 	debug_board_write_str(datax); */
	      /* } */
	      poller->transfer_complete = true;
	      /* sprintf(datax, "Finished for: %s with http at %p\n", poller->url_string, poller->http_handle); */
	      /* debug_board_write_str(datax); */
	    }
	  else {
	    poller = poller->next_kolibri_fetch;
	    continue;
	  }
	}
	else /* if transfer_direction == SEND */
	  {
	    if(poller->multipart_data) {

	      struct multipart_data_fragment *fragment = poller->multipart_data;
	      http_send(poller->http_handle, fragment->header_line, fragment->length_header);
	      /* int remaining_length = fragment->length_header; */
	      /* int sent = 0; */
	      /* int total_sent = 0; */

	      /* while(remaining_length > 0) { */
	      /* 	sent = http_send_asm(poller->http_handle, fragment->header_line + total_sent, remaining_length); */

	      /* 	if(sent == -1) { */
	      /* 	  debug_board_write_str("Error occured with http_send!\n"); */
	      /* 	  /\* __asm__ __volatile__("int3"); *\/ */
	      /* 	  continue; /\* FIXME:  *\/ */
	      /* 	} */
	      /* 	remaining_length -= sent; */
	      /* 	total_sent += sent; */
	      /* } */

	      char *data_with_crlf = malloc(remaining_length + 1);
	      sprintf(data_with_crlf, "%s\r\n", fragment->data_or_filename == NULL ? "" : fragment->data_or_filename);
	      http_send(poller->http_handle, data_with_crlf, fragment->length - fragment->length_header);
	      free(data_with_crlf);

	      /* remaining_length = fragment->length - fragment->length_header; */
	      /* total_sent = 0; */

	      /* char *data_with_crlf = malloc(remaining_length + 1); */
	      /* sprintf(data_with_crlf, "%s\r\n", fragment->data_or_filename == NULL ? "" : fragment->data_or_filename); */
	      
	      /* while(remaining_length > 0) { */
	      /* 	sent = http_send_asm(poller->http_handle, data_with_crlf + total_sent, remaining_length); */

	      /* 	if(sent == -1) { */
	      /* 	  debug_board_write_str("Error occured with http_send!\n"); */
	      /* 	  /\* __asm__ __volatile__("int3");  *\/ */
	      /* 	  continue;/\* FIXME *\/ */
	      /* 	} */
	      /* 	remaining_length -= sent; */
	      /* 	total_sent +=sent; */
	      /* } */
	      /* free(data_with_crlf); */

	      poller->multipart_data = poller->multipart_data -> next;
	      free(fragment);

	      if(poller->multipart_data == NULL)
		{
		  int remaining_length = strlen(post_multipart_boundary) - 2;
		  char *end_of_multipart = malloc(remaining_length + 4 + 1);
		  int i;

		  for(i = 0; i < remaining_length; i++)
		    end_of_multipart[i] = post_multipart_boundary[i];

		  end_of_multipart[i++] = '-';
		  end_of_multipart[i++] = '-';
		  end_of_multipart[i++] = '\r';
		  end_of_multipart[i++] = '\n';
		  end_of_multipart[i++] = '\0';

		  remaining_length += 4;
		  
		  http_send(poller->http_handle, end_of_multipart, remaining_length);

		  /* remaining_length = strlen(post_multipart_boundary) - 2; */
		  /* char *end_of_multipart = malloc(remaining_length + 4 + 1); */
		  /* int i; */
		  /* for(i = 0; i < remaining_length; i++) */
		  /*   end_of_multipart[i] = post_multipart_boundary[i]; */

		  /* end_of_multipart[i++] = '-'; */
		  /* end_of_multipart[i++] = '-'; */
		  /* end_of_multipart[i++] = '\r'; */
		  /* end_of_multipart[i++] = '\n'; */
		  /* end_of_multipart[i++] = '\0'; */
		
		  /* remaining_length += 4; */
		  
		  /* /\* sprintf(end_of_multipart, "%s--\r\n", post_multipart_boundary); *\/ */
		  
		  /* while(remaining_length > 0) { */
		  /*   sent = http_send_asm(poller->http_handle, end_of_multipart + total_sent, remaining_length); */

		  /*   if(sent == -1) { */
		  /*     debug_board_write_str("Error occured with http_send with multipart!\n"); */
		  /*     __asm__ __volatile__("int3"); */
		  /*     continue;/\* FIXME *\/ */
		  /*   } */
		  /*   remaining_length -= sent; */
		  /*   total_sent += sent; */
		  /* } */

		  /*Invert the direction after we have sent all multipart data */
		  poller->transfer_direction = RECEIEVE;
		}

	      debug_board_write_str("Everything done for multipart!");
	      /* __asm__ __volatile__("int3"); */

	      poller = poller->next_kolibri_fetch;
GLOBAL_DESTROY = true;	      
/* if(GLOBAL_DESTROY == true) { debug_board_write_str("XX3\n"); __asm__ __volatile__("int3"); } */
	      continue;
	    }
	    else {

	      http_send(poller->http_handle, poller->urlenc_data, poller->content_length);

	      /* int remaining_length = poller->content_length; */
	      /* int total_sent = 0; */
	      /* int sent = 0; */

	      /* while(remaining_length > 0) */
	      /* 	{ */
	      /* 	  sent = http_send_asm(poller->http_handle, poller->urlenc_data + total_sent, remaining_length); */

	      /* 	  if(sent == -1) { */
	      /* 	    debug_board_write_str("Error occured with http_send for urlenc!\n"); */
	      /* 	    __asm__ __volatile__("int3"); */
	      /* 	    break; */
	      /* 	  } */

	      /* 	  remaining_length -= sent; */
	      /* 	  total_sent += sent; */
	      /* 	} */

	      /* if(remaining_length > 0) continue; */
	      
	      poller->transfer_direction = RECEIEVE;
	      poller->content_length = 0;
	      poller = poller->next_kolibri_fetch;
	    }
	  }
      }
    
      if((poller -> finished == false && poller->transfer_complete == true)) {
	if(poller->transfer_direction == RECEIEVE)
	  {
	    fetch_msg msg;
	    unsigned int http_code = poller->http_handle->status;
	    fetch_set_http_code(poller->fetch_handle, http_code);

	    if(http_code == 200) {
	      /* debug_board_write_str("200 OK! HTTP Code.\n"); */
	      /* __asm__ __volatile__("int3"); */
	      /* debug_board_write_str("Do FETCH_DATA\n"); */

	      msg.type = FETCH_DATA;
	      msg.data.header_or_data.buf = (const uint8_t *) poller -> http_handle -> content_ptr;
	      msg.data.header_or_data.len = (size_t) poller -> http_handle -> content_length;
	      fetch_send_callback(&msg, poller->fetch_handle);
	      poller->data_processed = true;
	    }
	    else if (300 <= http_code && http_code < 400) {
	      /* debug_board_write_str("Got redirect..."); */
	      /* __asm__ __volatile__("int3"); */
	      if(http_code == 301 || http_code == 302) {
		if(poller->location) {
		  msg.type = FETCH_REDIRECT;

  if (HEADER_DEBUG == true) {
    debug_board_write_str("New location: ");
    debug_board_write_str(poller->location);
    debug_board_write_str("\n");
    /* __asm__ __volatile__("int3"); */
  }
		  msg.data.redirect = poller->location;
		  fetch_send_callback(&msg, poller->fetch_handle);
		}
		else {
		  debug_board_write_str("Got 3xx redirect but no target URL.");
		  /* __asm__ __volatile__("int3"); */
		}
	      }
	      else {
		/* Code is 3xx but not 301 or 302 */
	      }
	    }
	    else if (400 <= http_code && http_code < 500) {
	      if(http_code == 401) {
		msg.type = FETCH_AUTH;
		msg.data.auth.realm = poller->realm;
		fetch_send_callback(&msg, poller->fetch_handle);
	      }
	      else if(http_code == 404) {
		msg.type = FETCH_ERROR;
		msg.data.error = messages_get("HTTP Code 404: Not found");
		fetch_send_callback(&msg, poller->fetch_handle);
		fetch_abort(poller->fetch_handle);
	      }
	      else {
		msg.type = FETCH_ERROR;
		msg.data.error = messages_get("HTTP Code 4xx: Client Error.");
		fetch_send_callback(&msg, poller->fetch_handle);
		fetch_abort(poller->fetch_handle);
	      }
	    }
	    else if (http_code >= 500 && http_code < 600) {
	      msg.type = FETCH_ERROR;
	      msg.data.error = messages_get("HTTP Code 5xx: Internal Server Errors.");
	      fetch_send_callback(&msg, poller->fetch_handle);
	      fetch_abort(poller->fetch_handle);
	    }
	    else {
	      msg.type = FETCH_ERROR;
	      msg.data.error = messages_get("Got Unknown HTTP Code. Aborting.");
	      fetch_send_callback(&msg, poller->fetch_handle);
	      fetch_abort(poller->fetch_handle);
	    } /* End of handling http codes */

	    if(poller->data_processed == true) {
	      /* debug_board_write_str("Do FETCH_FINISHED\n"); */
	      /* debug_board_write_str(poller->url_string); */
	      /* debug_board_write_str("\n"); */
	      //__asm__ __volatile__("int3");

	      msg.type = FETCH_FINISHED;
	      fetch_send_callback(&msg, poller->fetch_handle);
	      poller->finished = true;

	      /* FIXME: Properly free the fetcher and http handle */
	      fetch_remove_from_queues(poller->fetch_handle);
	      fetch_abort(poller->fetch_handle);
	    }
	  } /* End of if direction == RECEIEVE */
      }
      else if(poller->finished == true) {
	/* time to free the fetcher  */
      }
      poller = poller -> next_kolibri_fetch;
    }

    xfile = fopen("/tmp0/1/poller.txt", "a");
    fprintf(xfile, "Returning from poller for enrty : %d\n", global_poll_counter);
    fclose(xfile);
}

void fetch_http_kolibri_finalise(lwc_string *scheme) {
  debug_board_write_str("Finalize fetch called!\n");
  //  __asm__ __volatile__("int3");
}

nserror fetch_http_kolibri_register(void)
{
	const struct fetcher_operation_table kolibri_http_ops = {
		.initialise = fetch_http_kolibri_initialise,
		.acceptable = fetch_http_kolibri_can_fetch,
		.setup = fetch_http_kolibri_setup,
		.start = fetch_http_kolibri_start,
		.abort = fetch_http_kolibri_abort,
		.free = fetch_http_kolibri_free,
		.poll = fetch_http_kolibri_poll,
		.finalise = fetch_http_kolibri_finalise
	};

	/* Register the fetcher operation table above for HTTP */

	return fetcher_add(lwc_string_ref(corestring_lwc_http), &kolibri_http_ops);

	/* TODO: Since KolibriOS does not have an HTTPS implementation yet,
	   add it later using a similar approach to HTTP. */

	return NSERROR_OK;
}

time_t kolibri_getdate(char *datestring)
{

  /* debug_board_write_str("Converting : "); */
  /* debug_board_write_str(datestring); */
  /* debug_board_write_str("\n"); */

  struct tm converted_time;
  if (strncmp(datestring, "Mon", 3) == 0)
    converted_time.tm_wday = 1;
  else  if (strncmp(datestring, "Tue", 3) == 0)
    converted_time.tm_wday = 2;
  else  if (strncmp(datestring, "Wed", 3) == 0)
    converted_time.tm_wday = 3;
  else  if (strncmp(datestring, "Thu", 3) == 0)
    converted_time.tm_wday = 4;
  else  if (strncmp(datestring, "Fri", 3) == 0)
    converted_time.tm_wday = 5;
  else  if (strncmp(datestring, "Sat", 3) == 0)
    converted_time.tm_wday = 6;
  else  if (strncmp(datestring, "Sun", 3) == 0)
    converted_time.tm_wday = 0;
  else {
    debug_board_write_str("Unhandled day! : ");
    debug_board_write_str(datestring);
    debug_board_write_str("\n");
    return (time_t)(-1);
  }

  datestring+=3;

  for(;*datestring == ',' || *datestring == ' ';datestring++);

  int day_val = 0;

  for(;isdigit(*datestring);datestring++)
    day_val=(day_val*10) + *datestring - '0';

  for(;*datestring == ',' || *datestring == ' ';datestring++);

  if (strncmp(datestring, "Jan", 3) == 0)
    converted_time.tm_mon = 0;
  else  if (strncmp(datestring, "Feb", 3) == 0)
    converted_time.tm_mon = 1;
  else  if (strncmp(datestring, "Mar", 3) == 0)
    converted_time.tm_mon = 2;
  else  if (strncmp(datestring, "Apr", 3) == 0)
    converted_time.tm_mon = 3;
  else  if (strncmp(datestring, "May", 3) == 0)
    converted_time.tm_mon = 4;
  else  if (strncmp(datestring, "Jun", 3) == 0)
    converted_time.tm_mon = 5;
  else  if (strncmp(datestring, "Jul", 3) == 0)
    converted_time.tm_mon = 6;
  else  if (strncmp(datestring, "Aug", 3) == 0)
    converted_time.tm_mon = 7;
  else  if (strncmp(datestring, "Sep", 3) == 0)
    converted_time.tm_mon = 8;
  else  if (strncmp(datestring, "Oct", 3) == 0)
    converted_time.tm_mon = 9;
  else  if (strncmp(datestring, "Nov", 3) == 0)
    converted_time.tm_mon = 10;
  else  if (strncmp(datestring, "Dec", 3) == 0)
    converted_time.tm_mon = 11;
  else {
    debug_board_write_str("Unhandled month!\n");
    return (time_t)(-1);
  }

  datestring+=3;

  for(;*datestring == ',' || *datestring == ' ';datestring++);

  int year_val = 0;
  for(;isdigit(*datestring) && *datestring;datestring++)
    year_val=(year_val*10) + *datestring - '0';

  for(;*datestring == ',' || *datestring == ' ';datestring++);

  int hh_val = (*datestring-'0') * 10;
  datestring++;
  hh_val += *datestring - '0';

  datestring++; //Skip the separator between HH and MM (usually :)
  datestring++;

  int mm_val = (*datestring-'0') * 10;
  datestring++;
  mm_val += *datestring - '0';

  datestring++;
  datestring++; //Skip the separator between MM and SS (usually :)

  int ss_val = (*datestring-'0') * 10;
  datestring++;
  ss_val += *datestring - '0';

  int days_in_month[12]; /* Index 0 = Jan */
  /* Populate correct number of days in a month */
  days_in_month[0] = 31;

  if(year_val % 4 == 0)
    days_in_month[1] = 29;
  else
    days_in_month[1] = 28;

  days_in_month[2] = 31;
  days_in_month[3] = 30;
  days_in_month[4] = 31;
  days_in_month[5] = 30;
  days_in_month[6] = 30;
  days_in_month[7] = 31;
  days_in_month[8] = 30;
  days_in_month[9] = 31;
  days_in_month[10] = 30;
  days_in_month[11] = 31;

  /* "Sun, 03 Apr 2016 05:28:33 GMT" */
  /* "Wed Jun 30 21:49:08 1993\n" */

  /* Set up member values now for tm struct  */

  converted_time.tm_sec = ss_val;
  converted_time.tm_min = mm_val;    /* Minutes (0-59) */
  converted_time.tm_hour = hh_val;   /* Hours (0-23) */
  converted_time.tm_mday = day_val;   /* Day of the month (1-31) */
  /* converted_time.tm_mon = ;    Month (0-11) Already set. */
  converted_time.tm_year = year_val - 1900;   /* Year - 1900 */
  /* converted_time.tm_wday = ;   Day of the week (0-6, Sunday = 0) Already set*/

  int yday = 0;
  int i;
  for(i=0; i<converted_time.tm_mon; i++)
    yday += days_in_month[i];

  yday+=day_val;

  converted_time.tm_yday = yday;   /* Day in the year (0-365, 1 Jan = 0) */
  converted_time.tm_isdst = 0;  /* Daylight saving time */

  /* FIXME: We do not handle time zones yet and assume everything is in GMT */
  /* char ff[100]; */
  /* strftime(ff, 100, "%s", &converted_time); */

  return mktime(&converted_time);
}

void process_headers(struct kolibri_fetch *poller) {

  fetch_msg msg;

  if(poller->headers_processed == false) {
    int i, newline;
    size_t header_length;
    char *header;

    /* debug_board_write_str("Do FETCH_HEADER\n"); */
    msg.type = FETCH_HEADER;

    /* msg.data.header_or_data.len = (size_t) poller -> http_handle -> header_length; */
    /* msg.data.header_or_data.buf = (const uint8_t *)(&(poller -> http_handle -> http_header)); */

    /* debug_board_write_str("Remove board log file\n"); */
    /* __asm__ __volatile__("int3"); */
    char *headers_copy = NULL;

    if(headers_copy = (char *)malloc(poller->http_handle->header_length + 1)) {

      strncpy(headers_copy, (&(poller->http_handle->http_header)), poller->http_handle->header_length);
      headers_copy[poller->http_handle->header_length] = '\0';

      char *foo = strtok(headers_copy, "\r\n");

      while(foo!=NULL) {
	/* debug_board_write_str(foo); */
	/* debug_board_write_str("\n"); */

	if(HEADER_DEBUG = true) {
	debug_board_write_str(foo);
	debug_board_write_str("\n");
	}

	msg.data.header_or_data.len = strlen(foo);
	msg.data.header_or_data.buf = (const uint8_t *) foo;
	fetch_send_callback(&msg, poller->fetch_handle);

	if(!strncmp("set-cookie:", foo, 10))
	    fetch_set_cookie(poller->fetch_handle, foo+12);
	else if(!strncmp("location:", foo, 9))
	  {
	    poller->location = strdup(foo + 10);
	    poller->redirected = true;
	  }
	else if(!strncmp("realm:", foo, 6))
		poller->realm = strdup(foo + 7);

	foo = strtok(NULL, "\r\n");
      }

      free(headers_copy);
    }
    else {
      debug_board_write_str("Can't parse headers. Malloc failed.\n");
    }
    /* __asm__ __volatile__("int3"); */
  }

  poller->headers_processed = true;

  /* debug_board_write_str("Headers processed\n"); */
  /* __asm__ __volatile__("int3"); */
}

struct fetch_multipart_data* multipart_convert(struct fetch_multipart_data *control, int *content_length)
{
 /* FIXME: Fix this function to do a stat on filenames and get their size. After that, the filesize should be added */
 /*        to content_length pointer (the second parameter to function  */

  FILE *httplog =  NULL;

  struct multipart_data_fragment *free_me = malloc(sizeof(struct multipart_data_fragment));
  struct multipart_data_fragment *creator = NULL;
  struct multipart_data_fragment *to_be_freed = free_me;
  struct multipart_data_fragment *to_be_returned = NULL;

  for (; control; control = control->next) {
    creator = malloc(sizeof(struct multipart_data_fragment));
          httplog = fopen("/tmp0/1/httplog.txt", "a");


    if(control->file) {

      /* Content-Disposition: form-data; name="img"; filename="a.png" */
      /* malloc, ending 1 + 4 + 1 : EndQuote + CRLF + CRLF + NULLBYTE */
      creator->header_line = (char *)malloc(strlen(post_multipart_boundary) + 38 + strlen(control->name) + 13 + strlen(control->rawfile) + 1 + 4 + 1);

      sprintf(creator->header_line, "--%sContent-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\nContent-Type: application/octet-stream\r\n\r\n", post_multipart_boundary, control->name, control->rawfile);
      
      creator->data_or_filename = strdup(control->rawfile);

      /*FIXME: Fix this stat call and file size getting */
      /* creator->length += (stat(filename) -> file_size)  + 2 (for CRLF)*/
      /* +2 for CRLF */
      creator->length_header = strlen(creator->header_line);
      creator->length = creator->length_header + 2 ;
      *content_length += creator->length;
    }    
    else {
      /* Content-Disposition: form-data; name="foo" */
      creator->header_line = (char *)malloc(strlen(post_multipart_boundary) + 38 + strlen(control->name) + 1 + 4 + 1);

      sprintf(creator->header_line, "--%sContent-Disposition: form-data; name=\"%s\"\r\n\r\n", post_multipart_boundary, 
	      control->name);

      creator->data_or_filename = strdup(control->value);
      creator->length_header = strlen(creator->header_line);
      creator->length = creator->length_header + strlen(creator->data_or_filename) + 2;
      *content_length += creator->length;
    }
    creator -> next = NULL;

    fprintf(httplog, "line: %s", creator->header_line);
    fprintf(httplog, "value: %s", creator->data_or_filename);
    fprintf(httplog, "\nLength: %d", creator->length_header);
    fprintf(httplog, "\nTotal length: %d\n", creator->length);
    fclose(httplog);

    free_me->next = creator;
    free_me = free_me->next;
  }
  
  to_be_returned = to_be_freed -> next;
  free(to_be_freed);

  /* __asm__ __volatile__("int3"); */
  /*Compensate for ending boundary and --/r/n to be sent over the network */
  /* -2 because we don't want a CRLF after end of boundary at end of multipart */
  *content_length += strlen(post_multipart_boundary) + 4 - 2;

  return to_be_returned;
}
