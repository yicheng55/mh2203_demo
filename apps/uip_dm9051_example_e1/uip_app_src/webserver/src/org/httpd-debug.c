//
// To access the httpd implement source code data, 'http_index_html'
//   Such as add implement code in ./AT/httpd.c
//   or in ./uip-1-0/httpd.c
//	 It will be stll OK,
//	 If not work to add such implement code.
//
#include <stdio.h>
#include "httpd-debug.h"

//Add implement code in "httpd.c"
//(1) #include "httpd-debug.h"
//(2) void httpd_report_filename(void) { ... } //Added code
//      fs_HOME_HTML = http_index_html; //For an example, the home index fs_filename.
//      to 'fs_HOME_HTML' varible.
//(3) httpd_report_filename(); //Call this in "httpd_init()"

//In main() make fs_HOME_HTML print
//(1) #include "httpd-debug.h"
//(2) httpd_init(); //Call this in main(), and then
//    httpd_debug_print() //Afterward call this.

const char *fs_HOME_HTML = NULL; 
const struct httpd_fsdata_file *fs_ROOT_HTML = NULL;

void httpd_debug_print(void)
{
	if (fs_HOME_HTML){
		printf("HonePage: %s\n", fs_HOME_HTML);
	}
	if (fs_ROOT_HTML){
		printf("HonePage: PRINT ROOT FS TREE!!\n"); //'fs_ROOT_HTML'
		if (1){
		  struct httpd_fsdata_file_noconst *f;
		  for(f = (struct httpd_fsdata_file_noconst *)fs_ROOT_HTML;
			  f != NULL;
			  f = (struct httpd_fsdata_file_noconst *)f->next) {
				  printf("%s\n", f->name);
		  }
		}
	}
	
	if (!fs_HOME_HTML)
		printf("(check HomeIndex filename)\n");
	if (!fs_ROOT_HTML)
		printf("(check HTTPD_FS_ROOT)\n");
}
