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
    int errno;
    // check valid flags
    if (flags != O_RDONLY || flags != O_WRONLY ||flags != O_RDWR) {
        return EINVAL;
    }

    if (filename == NULL) {
        return EFAULT;
    }

    //copy filename into kernal (check valid filename pointer)
    char *kern_filename = kmalloc(__NAME_MAX);

    errno = copyinstr(filename, kfilename, __NAME_MAX, NULL);
    if (errno != 0) {
        return errno;
    }

    //check for lowest available fd and oft spots
    int of_table_num = -1;
    int fd_table_num = -1;
    for(int i = 3; i < __OPEN_MAX; i++) {
        if(of_table[i]->vnode == NULL) {
            of_table_num = i;
        }
    }
    if (of_table_num == -1) {
        return ENFILE;
    }

    for(int i = 3; i < __OPEN_MAX; i++) {
        if(of_table[i] == NULL) {
            fd_table_num = i;
        }
    }
        if (fd_table_num == -1) {
        return EMFILE;
    }

    //get vnode for file
    struct vnode **file_vnode = kmalloc(sizeof(struct vnode));
    
    errno = vfs_open(kern_filename, flags, mode, file_vnode);
    if (errno != 0) {
        return errno;
    }
    //create table entry
    of_table[of_table_num]->fp = 0;
    of_table[of_table_num]->ref_count = 1;
    of_table[of_table_num]->flag = flags;
    of_table[of_table_num]->vnode = file_vnode;

    //assign fd to oft entry 
    curproc->fd_table[fd_table_num] = &of_table[of_table_num];

    return fd_table_num;
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


