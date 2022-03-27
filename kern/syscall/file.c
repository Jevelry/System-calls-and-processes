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

int sys_open(userptr_t filename, int flags, mode_t mode, int *retval) {
    int errno;
    // Set to return an error for now until end of function is reached
    *retval = -1;
    // check valid flags
    int flag_mode = (flags & O_ACCMODE);
    int flag = -1;
    if (flag_mode == O_RDONLY) {
        flag = O_RDONLY;
    }
    else if (flag_mode == O_WRONLY) {
        flag = O_WRONLY;
    }
    else if (flag_mode == O_RDWR) {
        flag = O_RDWR;
    } else {
        return EINVAL;
    }


    if (filename == NULL) {
        return EFAULT;
    }

    //copy filename into kernal (check valid filename pointer)
    char *kern_filename = kmalloc(__NAME_MAX);

    errno = copyinstr(filename, kern_filename, __NAME_MAX, NULL);
    if (errno != 0) {
        return errno;
    }

    //check for lowest available fd and oft spots
    int of_table_num = -1;
    int fd_table_num = -1;
    for(int i = 3; i < __OPEN_MAX; i++) {
        if(of_table[i].vnode == NULL) {
            of_table_num = i;
        }
    }
    if (of_table_num == -1) {
        return ENFILE;
    }

    for(int i = 3; i < __OPEN_MAX; i++) {
        if(curproc->fd_table[i] == NULL) {
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
    of_table[of_table_num].fp = 0;
    of_table[of_table_num].ref_count = 1;
    of_table[of_table_num].flags = flag;
    of_table[of_table_num].vnode = *file_vnode;

    //assign fd to oft entry 
    curproc->fd_table[fd_table_num] = &of_table[of_table_num];

    // put return val in address
    *retval = fd_table_num;
    return 0;
}

int sys_close(int fd, int *retval) {
    //Check if fd in valid range
    if (fd < 0 || fd > __OPEN_MAX) {
        return EBADF;
    }
    // Check if fd table entry is valid
    if (curproc->fd_table[fd] == NULL) {
        return EBADF;
    }
    // Close file
    open_file *file = curproc->fd_table[fd];
    file->ref_count--;
    if (file->ref_count == 0) {
        // no need to check for error. vfs.h says it doesn't fail
        vfs_close(file->vnode);
        file->fp = 0;
        file->flags = -1;
        file->vnode = NULL;
    }
    curproc->fd_table[fd] = NULL;
    *retval = 0;
    return 0;
}

int sys_read(int fd, void *buf, size_t buflen, int *retval) {
    //Check if fd in valid range
    if (fd < 0 || fd >= OPEN_MAX) {
        return EBADF;
    }

    //Check if file is valid
    open_file *file = curproc->fd_table[fd];
    if (file->vnode == NULL || (file->flags != O_RDONLY && file->flags != O_RDWR)) {
        return EBADF;
    }

    //Check if buf is valid
    if (buf == NULL) {
        return EFAULT;
    }

    struct iovec iovec;
    struct uio uio;
    uio_uinit(&iovec, &uio, (userptr_t) buf, buflen, file->fp, UIO_READ);

    int errno = VOP_READ(file->vnode, &uio);
    if (errno) {
        return errno;
    }

    file->fp = uio.uio_offset;
    *retval = buflen - uio.uio_resid;
    return 0;
}

int sys_write(int fd, void *buf, size_t nbytes, int *retval){
    //Check if fd in valid range
    if (fd < 0 || fd >= OPEN_MAX) {
        return EBADF;
    }

    //Check if file is valid
    open_file *file = curproc->fd_table[fd];
    if (file->vnode == NULL || (file->flags != O_WRONLY && file->flags != O_RDWR)) {
        return EBADF;
    }

    //Check if buf is valid
    if (buf == NULL) {
        return EFAULT;
    }

    struct iovec iovec;
    struct uio uio;
    uio_uinit(&iovec, &uio, (userptr_t) buf, nbytes, file->fp, UIO_WRITE);

    int errno = VOP_WRITE(file->vnode, &uio);
    if (errno) {
        return errno;
    }

    file->fp = uio.uio_offset;
    *retval = nbytes - uio.uio_resid;
    return 0;
}

int sys_dup2(int oldfd, int newfd, int *retval) {
    // Assume something will go wrong for now
    *retval = -1;
    // Check if both fds are valid ints

    if (oldfd < 0 || oldfd > __OPEN_MAX) {
        return EBADF;
    }
    if (newfd < 0 || newfd > __OPEN_MAX) {
        return EBADF;
    }
    // Check if they are the same number. if so just return
    if (oldfd == newfd) {
        *retval = newfd;
        return 0;
    }

    // Check if old fd exists
    if (curproc->fd_table[oldfd] == NULL) {
        return EBADF;
    }

    // Check if newfd names an open file. If so close it
    if (curproc->fd_table[newfd] != NULL) {
        int file_close_ret = 0;
        int errno = sys_close(newfd, &file_close_ret);
        if( errno != 0 ) {
            return  errno; 
        }
    }

    // Update oldfd onto newfd 
    curproc->fd_table[newfd] = curproc->fd_table[oldfd];

    // Return newfd
    *retval = newfd;
    return 0;
}

int sys_lseek(int fd, off_t offset, int whence, off_t *retval) {
    // Assume something will go wrong for now
    *retval = -1;
    // Check fd in valid range
    if (fd < 0 || fd > __OPEN_MAX) {
        return EBADF;
    }
    // Check fd valid in table, i.e if fd 0 is assigned
    if (curproc->fd_table[fd] == NULL) {
        return EBADF;
    }
    // Check whence flag is valid
    if (whence == SEEK_CUR || whence == SEEK_END || whence == SEEK_SET) {
    } else {
        return EINVAL;
    }

    // Check if file is seekable, ie not a device
    open_file *file = curproc->fd_table[fd];
    struct stat *stats = kmalloc(sizeof(struct stat));
    int can_seek = VOP_ISSEEKABLE(file->vnode);
    if(can_seek == 0) {
        // cannot seek file
        return ESPIPE;
    }
    off_t new_pos;
    off_t cur_pos = file->fp;

    // Set new position
    if (whence == SEEK_CUR) {
        new_pos = cur_pos + offset;
    } else if (whence == SEEK_END) {
        VOP_STAT(file->vnode, stats);
        new_pos = stats->st_size + offset;
    } else {
        new_pos = offset;
    }

    // Check if resulting seek position would be negative
    if(new_pos < 0) {
        return EINVAL;
    }

    file->fp = new_pos;
    kfree(stats);

    *retval = new_pos;
    return 0;
}


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
        of_table[counter].flags = -1;
        of_table[counter].vnode = NULL; 
        of_table[counter].ref_count = 0;
    }

    char con1[] = "con:";
    char con2[] = "con:";
    char con3[] = "con:";

    //stdout
    of_table[1].flags = O_WRONLY;
    vfs_open(con1, O_WRONLY, 0, &of_table[1].vnode);

    //stderr
    of_table[2].flags = O_WRONLY;
    vfs_open(con2, O_WRONLY, 0, &of_table[2].vnode);

    //stdin TEMP
    of_table[0].flags = O_RDONLY;
    vfs_open(con3, O_RDONLY, 0, &of_table[0].vnode);
}

void destroy_of_table() {
    if (of_table == NULL) {
        return;
    }

    for (int counter = 0; counter < OPEN_MAX; counter++) {
        if(of_table[counter].vnode != NULL) {
            vfs_close(of_table[counter].vnode);
        }
    }

    kfree(of_table);
}


