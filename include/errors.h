#ifndef ERRORS_H
#define ERRORS_H

typedef struct SendErrorInfo SendErrorInfo;
typedef struct RecvErrorInfo RecvErrorInfo;
typedef struct PthreadMutexLockErrorInfo PthreadMutexLockErrorInfo;

typedef enum {
    ERROR_RECOVERABLE,
    ERROR_FATAL
} ErrorCategory;

struct SendErrorInfo{
    int error_code;
    ErrorCategory category;
};
struct PthreadMutexLockErrorInfo{
    int error_code;
    ErrorCategory category;

};

struct RecvErrorInfo{
    int error_code;
    ErrorCategory category;
};
#endif