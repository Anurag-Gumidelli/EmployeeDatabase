#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "common.h"
#include "file.h"

int create_db_file(char *path){
    int fd = open(path, O_RDONLY);

    if(fd != -1){
        printf("File: %s already exists, -n option is only for a non-existing file!\n", path);
        close(fd);
        return STATUS_ERROR;
    }

    fd = open(path, O_RDWR | O_CREAT, 0644);

    if(fd == -1) {
        perror("open");
        return STATUS_ERROR;
    }

    return fd;
}

int open_db_file(char *path){
    int fd = open(path, O_RDWR, 0644);

    if(fd == -1) {
        perror("open");
        return STATUS_ERROR;
    }

    return fd;
}
