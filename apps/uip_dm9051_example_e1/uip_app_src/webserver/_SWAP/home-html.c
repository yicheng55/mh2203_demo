//
// To access the httpd implement source code data, 'http_index_html'
//   Such as in ./AT/http-strings.c
//   or in ./uip-1-0/http-strings.c
//
#include <stdio.h>
extern const char http_index_html[];	//#include "http-strings.h"
#define HOME_HTML http_index_html

void httpd_print_homehtml(void)
{
	printf("HonePage: %s\n", HOME_HTML);
}
