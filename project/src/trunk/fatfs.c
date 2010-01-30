#include "fatfs.h"
//
// Map driver calls to system calls
//

// Init uart on open, Always succeeds
int fatfs_open_r(struct _reent *r, const char *path, int flags, int mode) {
    printf("FatFs open\n");
    printf("Opening path: %s", path+strlen("fatfs:"));
    return 0;
}

// Close always succeeds for uart
int fatfs_close_r(struct _reent *r, int fd) {
   return 0; 
}

_ssize_t fatfs_read_r(struct _reent *r, int fd, void *ptr, size_t len)
{
    return 0;
}


_ssize_t fatfs_write_r(struct _reent *r, int fd, const void *ptr, size_t len) {
    return 0;
}

_off_t fatfs_lseek_r(struct _reent *r, int fd, _off_t ptr, int dir) {
    //  Always indicate we are at file beginning
    return (_off_t)0;
}

int fatfs_fstat_r(struct _reent *r, int fd, struct stat *stat_buf) {
    return 0;
}

// Define the driver operations table
extern const devop_tab_t devop_tab_fatfs = { 
    .name = "fatfs",
    .open_r = fatfs_open_r,
    .close_r = fatfs_close_r,
    .write_r = fatfs_write_r,
    .read_r = fatfs_read_r,
    .lseek_r = fatfs_lseek_r,
    .fstat_r = fatfs_fstat_r
};


