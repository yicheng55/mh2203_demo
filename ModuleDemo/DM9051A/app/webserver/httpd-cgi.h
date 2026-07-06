#ifndef __HTTPD_CGI_H__
#define __HTTPD_CGI_H__

#include "psock.h"
#include "httpd.h"

typedef PT_THREAD((* httpd_cgifunction)(struct httpd_state *, char *));

httpd_cgifunction httpd_cgi(char *name);

struct httpd_cgi_call {
  const char *name;
  const httpd_cgifunction function;
};

#define HTTPD_CGI_CALL(name, str, function) \
static PT_THREAD(function(struct httpd_state *, char *)); \
static const struct httpd_cgi_call name = {str, function}

void httpd_cgi_init(void);
#endif
