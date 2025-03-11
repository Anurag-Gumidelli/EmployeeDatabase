#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#include "common.h"
#include "file.h"
#include "parse.h"


void print_usage(char *argv[]){
    printf("Usage: %s -n -f <database file>\n", argv[0]);
    printf("\t -n - Create new database file\n");
    printf("\t -f - (required) Path to database file\n");
    return;
}

int main(int argc, char *argv[]){
    
    int ch, dbfd;
    bool newFile = false;
    char *filePath = NULL;
    dbHeader *dbh = NULL;
    Employee *emps = NULL;
    char *addString = NULL;

    while((ch = getopt(argc, argv, "nf:a:")) != -1){
        switch (ch)
        {
            case 'n':
                newFile = true;
                break;
            case 'f':
                filePath = optarg;
                break;
            case 'a':
                addString = optarg;
                break;
            case '?':
                printf("Unknown option detected -%c\n", ch);
            default:
                print_usage(argv);
                return -1;
        }
    }

    if(filePath == NULL){
        printf("Missing a required argument: Filepath\n");
        print_usage(argv);
        return -1;
    }

    if(newFile){
        dbfd = create_db_file(filePath);
        if(dbfd == STATUS_ERROR){
            printf("Unable to create new db file at %s\n", filePath);
            return -1;
        }
        if(create_db_header(&dbh) == STATUS_ERROR){
            printf("Failed to create database header\n");
            close(dbfd);
            return -1;
        }

    } else{
        dbfd = open_db_file(filePath);
        if(dbfd == STATUS_ERROR){
            printf("Unable to open db file at %s\n", filePath);
            return -1;
        }
        if(validate_db_header(dbfd, &dbh) == STATUS_ERROR){
            printf("Failed to validate database header\n");
            close(dbfd);
            return -1;
        }
    }

    if(read_employees(dbfd, dbh, &emps) == STATUS_ERROR){
        printf("Failed to read database employees\n");
        close(dbfd);
        return -1;
    }

    if(addString){
        if(add_employee(dbh, &emps, addString) == STATUS_ERROR){
            printf("Failed to add employee  %s to database\n", addString);
            close(dbfd);
            return -1;
        }
    }

    output_db_file(dbfd, dbh, emps);

    return 0;
}
