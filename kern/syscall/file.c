#include <types.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <kern/limits.h>
#include <kern/stat.h>
#include <kern/seek.h>
#include <lib.h>
#include <uio.h>
#include <proc.h>
#include <current.h>
#include <synch.h>
#include <vfs.h>
#include <vnode.h>
#include <file.h>
#include <syscall.h>
#include <copyinout.h>

/*
 * Add your file-related functions here ...
 */

int sys_open(const char *filename, int flags, mode_t mode, int *retval) {

}

int sys_close(int fd, int *retval) {

}

ssize_t read(int fd, void *buf, size_t buflen, int *retval) {

}

ssize_t write(int fd, const void*buf, size_t nbytes, int *retval){


}

int dup2(int oldfd, int newfd, int *retval) {

}

// off_t lseek(int fd, off_t pos, int whence, &retval) {

// }


/*
 * fd_table and of_table init functions
 */

int init_of_table() {

}

int init_fd_table() {

}

