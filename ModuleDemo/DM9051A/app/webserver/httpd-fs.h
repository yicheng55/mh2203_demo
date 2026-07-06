#ifndef __HTTPD_FS_H__
#define __HTTPD_FS_H__

#define HTTPD_FS_STATISTICS 1

struct httpd_fs_file {
  char *data;
  int len;
};

int httpd_fs_open(const char *name, struct httpd_fs_file *file);

#ifdef HTTPD_FS_STATISTICS
#if HTTPD_FS_STATISTICS == 1
u16_t httpd_fs_count(char *name);
#endif
#endif

void httpd_fs_init(void);

#endif
