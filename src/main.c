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
    dbHeader *dbHeader = NULL;

    while((ch = getopt(argc, argv, "nf:")) != -1){
        switch (ch)
        {
            case 'n':
                newFile = true;
                break;
            case 'f':
                filePath = optarg;
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
        create_db_header(&dbHeader);


    } else{
        dbfd = open_db_file(filePath);
        if(dbfd == STATUS_ERROR){
            printf("Unable to open db file at %s\n", filePath);
            return -1;
        }
    }

    return 0;
}
