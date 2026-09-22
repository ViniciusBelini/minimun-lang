#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "runner.h"
#include "tokenize.h"
#include "astnize.h"

static void printRunnerValue(RunnerValue args)
{
    switch(args.type)
    {
        case RUNNER_INT:
            printf("%d", args.as.number);
            break;
    }
}

static Ast fixRunnerNode(Ast *ast)
{
    return (Ast){
        .type = TOKEN_STATEMENT,
        .line = ast->line,
        .column = ast->column,
        .block.statements = ast,
        .block.size = 1
    };
}

static bool sameRunnerType(RunnerValue left, RunnerValue right)
{
    return left.type == right.type;
}

static RunnerValue runnerBlock(Ast block)
{
    switch(block.type)
    {
        case TOKEN_ERROR:
            printf("Error: %.*s at line %d, column %d\n", block.strToken.length, block.strToken.start, block.line, block.column);
            return (RunnerValue){.type = RUNNER_BOOL,.as.boolean = 0};
        case TOKEN_INT:
            return (RunnerValue){.type = RUNNER_INT,.as.number = block.intToken};
        case TOKEN_SHOW:
            RunnerValue args = runnerBlock(block.block.statements[0]);

            printRunnerValue(args);

            printf("\n");

            return (RunnerValue){.type = RUNNER_BOOL,.as.boolean = 1};
        case TOKEN_OPERATOR:
            RunnerValue left = runnerBlock(*block.op.left);
            RunnerValue right = runnerBlock(*block.op.right);

            switch(block.op.type)
            {
                case OP_MUL:
                    return (RunnerValue){.type = RUNNER_INT,.as.number = left.as.number * right.as.number};
                case OP_DIV:
                    return (RunnerValue){.type = RUNNER_INT,.as.number = left.as.number / right.as.number};
                case OP_MOD:
                    return (RunnerValue){.type = RUNNER_INT,.as.number = left.as.number % right.as.number};
                case OP_ADD:
                    return (RunnerValue){.type = RUNNER_INT,.as.number = left.as.number + right.as.number};
                case OP_SUB:
                    return (RunnerValue){.type = RUNNER_INT,.as.number = left.as.number - right.as.number};
                default:
                    return (RunnerValue){.type = RUNNER_BOOL,.as.boolean = 0};

            }
        default:
            return (RunnerValue){.type = RUNNER_BOOL,.as.boolean = 0};
    }
}

void runner(Ast ast)
{
    for(size_t i = 0;i < ast.block.size;i++)
    {
        Ast block = ast.block.statements[i];

        RunnerValue result = runnerBlock(block);
    }
}
