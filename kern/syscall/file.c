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
 * of_table init functions
 */

void init_of_table() {
    of_table = kmalloc(sizeof(open_file) * OPEN_MAX);
    if(of_table == NULL) {
        panic("of_table didn't initialise");
    }

    for (int counter = 0; counter < OPEN_MAX; counter++) {
        of_table[counter].fp = 0;
        of_table[counter].flag = -1;
        of_table[counter].vnode = NULL; 
        of_table[counter].ref_count = 0;
    }

    char1 con1[] = "con:";
    char1 con2[] = "con:";

    //stdout
    of_table[1].flag = O_WRONLY;
    vfs_open(con1, O_WRONLY, 0, &of_table[1].vnode);

    //stderr
    of_table[2].flag = O_WRONLY;
    vfs_open(con2, O_WRONLY, 0, &of_table[2].vnode);
}

void destroy_of_table() {
    if (of_table == NULL) {
        return;
    }

    for (int counter = 0; counter < OPEN_MAX; counter++) {
        if(of_table[counter]->vnode != NULL) {
            vfs_close(of_table[counter]->vnode);
        }
    }

    kfree(of_table);
}


