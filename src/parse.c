#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>

#include "common.h"
#include "parse.h"



void list_employees(dbHeader *pDbHeader, Employee *employees){
    for(int i = 0; i < pDbHeader->numEmployees; i++){
        printf("%d )\n", i+1);
        printf("\tName        : %s\n", employees[i].name);
        printf("\tAddress     : %s\n", employees[i].address);
        printf("\tHourly Wage : %u\n", employees[i].hourlyWage);
    }
    return;
}

void remove_employees_matching_name(dbHeader *pDbHeader, Employee **employees, char *empName) {
    Employee *emps = *employees;
    uint32_t prevNumEmps = pDbHeader->numEmployees;
    uint32_t numRemEmps = 0;
    for(int i = prevNumEmps-1; i > -1; i--) {
        numRemEmps++;
        if(strcmp(emps[i].name, empName) == 0) {
            numRemEmps--;
            if(numRemEmps) {
                memmove(&(emps[i]), &(emps[i+1]), numRemEmps*sizeof(Employee));
            }
        }
    }

    if(prevNumEmps - numRemEmps) {
        emps = realloc(*employees, numRemEmps*sizeof(Employee));
        if(emps == NULL) {
            perror("realloc");
            printf("Remove employees failed!\n");
            return;
        }
        *employees = emps;
        pDbHeader->numEmployees = numRemEmps;
        pDbHeader->fileSize -= (prevNumEmps - numRemEmps)* sizeof(Employee);
    }
    return;
}

int add_employee(dbHeader *pDbHeader, Employee **employees, char *newEmpString) {
    char *name;
    char *address;
    char *wageStr;
    uint32_t wage;
    Employee *newEmployees = NULL;

    if((name = strtok(newEmpString, ",")) == NULL){
        printf("Invalid employee string: %s\n", newEmpString);
        return STATUS_ERROR;
    }
    if((address = strtok(NULL, ",")) == NULL) {
        printf("Invalid employee string: %s\n", newEmpString);
        return STATUS_ERROR;
    }
    if((wageStr = strtok(NULL, ",")) == NULL) {
        printf("Invalid employee string: %s\n", newEmpString);
        return STATUS_ERROR;
    }
    wage = atoi(wageStr);
    if(wage == 0){
        printf("WARNING: Wage for employee %s is 0\n", name);
    }

    newEmployees = (Employee *)realloc(*employees, (pDbHeader->numEmployees + 1) * sizeof(Employee));
    if(newEmployees == NULL) {
        perror("realloc");
        return STATUS_ERROR;
    }
    strncpy(newEmployees[pDbHeader->numEmployees].name, name, STR_MAX_SIZE);
    strncpy(newEmployees[pDbHeader->numEmployees].address, address, STR_MAX_SIZE);
    newEmployees[pDbHeader->numEmployees].hourlyWage = wage;

    pDbHeader->numEmployees += 1;
    pDbHeader->fileSize += sizeof(Employee);

    *employees = newEmployees;

    return STATUS_OK;
}

int read_employees(int dbFd, dbHeader *pDbHeader, Employee **employeesOut) {
    if(dbFd < 0){
        printf("Bad Database file descriptor: %d\n", dbFd);
        return STATUS_ERROR;
    }

    int numE = pDbHeader->numEmployees;

    Employee *es = calloc(numE, sizeof(Employee));
    if(es == NULL){
        perror("calloc");
        return STATUS_ERROR;
    }

    if(read(dbFd, es, numE * sizeof(Employee)) != (numE * sizeof(Employee))){
        perror("read");
        free(es);
        return STATUS_ERROR;
    }

    for(int i = 0; i < numE; i++){
        es[i].hourlyWage = ntohl(es[i].hourlyWage);
    }

    *employeesOut = es;

    return STATUS_OK;
}

int output_db_file(int dbFd, dbHeader *pDbHeader, Employee *employees) {
    if(dbFd < 0){
        printf("Bad Database file descriptor: %d\n", dbFd);
        return STATUS_ERROR;
    }
    uint32_t numE = pDbHeader->numEmployees;


    if(lseek(dbFd, 0, SEEK_SET) == -1){
        perror("lseek");
        printf("Failed write to disk!\n");
        return STATUS_ERROR;
    }

    if(ftruncate(dbFd, pDbHeader->fileSize) == -1){
        perror("ftruncate");
        return STATUS_ERROR;
    }

    pDbHeader->magic = htonl(pDbHeader->magic);
    pDbHeader->version = htons(pDbHeader->version);
    pDbHeader->numEmployees = htons(pDbHeader->numEmployees);
    pDbHeader->fileSize = htonl(pDbHeader->fileSize);

    if(write(dbFd, pDbHeader, DB_HEADER_SIZE) != DB_HEADER_SIZE) {
        perror("write");
        printf("Failed write to disk!\n");
        return STATUS_ERROR;
    }

    for(int i = 0; i < numE; i++){
        employees[i].hourlyWage = htonl(employees[i].hourlyWage);
        if(write(dbFd, &(employees[i]), sizeof(Employee)) != sizeof(Employee)) {
            perror("write");
            printf("Failed write to disk!\n");
            return STATUS_ERROR;
        }
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
