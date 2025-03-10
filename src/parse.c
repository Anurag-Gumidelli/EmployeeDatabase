#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>

#include "common.h"
#include "parse.h"

int read_employees(int dbFd, dbHeader *pDbHeader, Employee **employeesOut) {
    return STATUS_OK;
}

int output_db_file(int dbFd, dbHeader *pDbHeader, Employee *employees) {
    if(dbFd < 0){
        printf("Bad Database file descriptor: %d\n", dbFd);
        return STATUS_ERROR;
    }

    pDbHeader->magic = htonl(pDbHeader->magic);
    pDbHeader->version = htons(pDbHeader->version);
    pDbHeader->numEmployees = htons(pDbHeader->numEmployees);
    pDbHeader->fileSize = htonl(pDbHeader->fileSize);

    if(lseek(dbFd, 0, SEEK_SET) == -1){
        perror("lseek");
        printf("Failed write to disk!\n");
        return STATUS_ERROR;
    }

    if(write(dbFd, pDbHeader, DB_HEADER_SIZE) != DB_HEADER_SIZE) {
        perror("write");
        printf("Failed write to disk!\n");
        return STATUS_ERROR;
    }

    return STATUS_OK;

}

int validate_db_header(int dbFd, dbHeader **ppDbHeaderOut) {
    if(dbFd < 0){
        printf("Bad Database file descriptor: %d\n", dbFd);
        return STATUS_ERROR;
    }

    dbHeader *headerBuffer = (dbHeader *)calloc(1, DB_HEADER_SIZE);
    if(headerBuffer == NULL){
        perror("calloc");
        return STATUS_ERROR;
    }

    if(read(dbFd, headerBuffer, DB_HEADER_SIZE) != DB_HEADER_SIZE) {
        perror("read");
        free(headerBuffer);
        return STATUS_ERROR;
    }

    headerBuffer->magic = ntohl(headerBuffer->magic);
    headerBuffer->version = ntohs(headerBuffer->version);
    headerBuffer->numEmployees = ntohs(headerBuffer->numEmployees);
    headerBuffer->fileSize = ntohl(headerBuffer->fileSize);

    if(headerBuffer->magic != MAGIC_HEADER){
        printf("Header magic number does not match!\n");
        free(headerBuffer);
        return STATUS_ERROR;
    }

    if(headerBuffer->version != 0x01){
        printf("DB version is not valid!\n");
        free(headerBuffer);
        return STATUS_ERROR;
    }

    struct stat fileStat = {0};
    fstat(dbFd, &fileStat);
    if(headerBuffer->fileSize != fileStat.st_size) {
        printf("Corrupted dataBase file\n");
        free(headerBuffer);
        return STATUS_ERROR;
    }

    uint32_t expextedFileSize = DB_HEADER_SIZE + (headerBuffer->numEmployees * sizeof(Employee));
    if(expextedFileSize != headerBuffer->fileSize){
        printf("Corrupted dataBase file\n");
        free(headerBuffer);
        return STATUS_ERROR;
    }

    *ppDbHeaderOut = headerBuffer;

    return STATUS_OK;
} 

int create_db_header(dbHeader **ppDbHeaderOut) {
    dbHeader *newHeader = (dbHeader *)calloc(1, DB_HEADER_SIZE);
    if(newHeader == NULL){
        perror("calloc");
        return STATUS_ERROR;
    }

    newHeader->magic = MAGIC_HEADER;
    newHeader->version = 0x01;
    newHeader->numEmployees = 0;
    newHeader->fileSize = DB_HEADER_SIZE;

    *ppDbHeaderOut = newHeader;

    return STATUS_OK;
}
