/*
 * Declarations for file handle and file table management.
 */

#ifndef _FILE_H_
#define _FILE_H_

/*
 * Contains some file-related maximum length constants
 */
#include <limits.h>

#include <types.h>

/*
 * Put your function declarations and data types here ...
 */

 /*
 * DATA TYPES
 */
typedef struct open_file_struct {
    off_t fp;
    int flags;
    struct vnode *vnode;
    int ref_count;
} open_file;

open_file *of_table;

 /*
 * FUNCTIONS
 */

int sys_open(userptr_t filename, int flags, mode_t mode, int *retval);

int sys_close(int fd, int *retval);

int sys_read(int fd, void *buf, size_t buflen, int *retval);

int sys_write(int fd, void *buf, size_t nbytes, int *retval);

int sys_dup2(int oldfd, int newfd, int *retval); 

// off_t lseek(int fd, off_t pos, int whence, retval);

void init_of_table(void);

void destroy_of_table(void);



#endif /* _FILE_H_ */
