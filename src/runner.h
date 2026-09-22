#ifndef RUNNER_H
#define RUNNER_H
#include "tokenize.h"
#include "astnize.h"

typedef enum
{
    RUNNER_NULL,
    RUNNER_INT,
    RUNNER_BOOL
} RunnerType;

typedef struct
{
    RunnerType type;

    union
    {
        int number;
        char *string;
        int boolean;
    } as;

} RunnerValue;

void runner(Ast ast);

#endif
