#include <reent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "syscalls.h"

int fatfs_open_r(struct _reent *r, const char *path, int flags, int mode);
int fatfs_close_r(struct _reent *r, int fd);
_ssize_t fatfs_read_r(struct _reent *r, int fd, void *ptr, size_t len);
_ssize_t fatfs_write_r(struct _reent *r, int fd, const void *ptr, size_t len);
_off_t fatfs_lseek_r(struct _reent *r, int fd, _off_t ptr, int dir);
int fatfs_fstat_r(struct _reent *r, int fd, struct stat *stat_buf);


