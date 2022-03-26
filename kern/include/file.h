/*
 * Declarations for file handle and file table management.
 */

#ifndef _FILE_H_
#define _FILE_H_

/*
 * Contains some file-related maximum length constants
 */
#include <limits.h>


/*
 * Put your function declarations and data types here ...
 */

 /*
 * DATA TYPES
 */

struct of_table {
    int flags;
    off_t fpointer;
    struct vnode *vnode;
}




 /*
 * FUNCTIONS
 */
int sys_open(const char *filename, int flags, mode_t mode, int *retval);

int sys_close(int fd, int *retval);

ssize_t read(int fd, void *buf, size_t buflen, int *retval);

ssize_t write(int fd, const void*buf, size_t nbytes, int *retval);



int dup2(int oldfd, int newfd, int *retval); 

// off_t lseek(int fd, off_t pos, int whence, retval);





#endif /* _FILE_H_ */
