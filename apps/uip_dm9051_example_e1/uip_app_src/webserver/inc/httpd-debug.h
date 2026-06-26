// httpd-debug.h
#include "httpd-fsdata.h"

extern const char *fs_HOME_HTML;
extern const struct httpd_fsdata_file *fs_ROOT_HTML;

void httpd_report_filename(void);
void httpd_report_root_tree(void);
void httpd_debug_print(void);
