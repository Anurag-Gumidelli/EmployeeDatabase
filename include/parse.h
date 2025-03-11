#ifndef PARSE_H
#define PARSE_H

typedef struct db_header_t{
    uint32_t magic;
    u_int16_t version;
    u_int16_t numEmployees;
    uint32_t fileSize;
} dbHeader;

#define DB_HEADER_SIZE sizeof(dbHeader)

typedef struct employee_t {
    char name[STR_MAX_SIZE];
    char address[STR_MAX_SIZE];
    uint32_t hourlyWage;
} Employee;

int create_db_header(dbHeader **ppDbHeaderOut);
int validate_db_header(int dbFd, dbHeader **ppDbHeaderOut);
int read_employees(int dbFd, dbHeader *pDbHeader, Employee **employeesOut);
int output_db_file(int dbFd, dbHeader *pDbHeader, Employee *employees);
int add_employee(dbHeader *pDbHeader, Employee **employees, char *newEmpString);
void list_employees(dbHeader *pDbHeader, Employee *employees);
void remove_employees_matching_name(dbHeader *pDbHeader, Employee **employees, char *empName);

#endif