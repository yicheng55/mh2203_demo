#ifndef __HTTPD_H__
#define __HTTPD_H__

#include "psock.h"
#include "httpd-fs.h"

struct httpd_state {
  unsigned char timer;
  struct psock sin, sout;
  struct pt outputpt, scriptpt;
  char inputbuf[300];
  char filename[20];
  char state;
  struct httpd_fs_file file;
  int len;
  char *scriptptr;
  int scriptlen;
  unsigned short count;
};

void httpd_init(void);
void httpd_appcall(void);

void httpd_log(char *msg);
void httpd_log_file(u16_t *requester, char *file);

#endif
