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

RunnerValue runner(Ast ast)
{
    RunnerValue ret;

    for(size_t i = 0;i < ast.block.size;i++)
    {
        Ast block = ast.block.statements[i];

        switch(block.type)
        {
            case TOKEN_ERROR:
                printf("Error: %.*s at line %d, column %d\n", block.strToken.length, block.strToken.start, block.line, block.column);
                break;
            case TOKEN_INT:
                // printf("%d\n", block.intToken);
                ret.type = RUNNER_INT;
                ret.as.number = block.intToken;
                continue;
            case TOKEN_SHOW:
                // printf("%d\n", block.showArgs->type);
                RunnerValue args = runner(block);

                printRunnerValue(args);

                printf("\n");

                continue;
            case TOKEN_OPERATOR:
                RunnerValue left = runner(fixRunnerNode(block.op.left));
                RunnerValue right = runner(fixRunnerNode(block.op.right));

                // printf("%d\n", block.op.type);

                switch(block.op.type)
                {
                    case OP_MUL:
                        if(sameRunnerType(left, right) && left.type == RUNNER_INT) return (RunnerValue){.type = RUNNER_INT,.as.number = left.as.number * right.as.number};
                    case OP_DIV:
                        if(sameRunnerType(left, right) && left.type == RUNNER_INT) return (RunnerValue){.type = RUNNER_INT,.as.number = left.as.number / right.as.number};
                    case OP_MOD:
                        if(sameRunnerType(left, right) && left.type == RUNNER_INT) return (RunnerValue){.type = RUNNER_INT,.as.number = left.as.number % right.as.number};
                    case OP_ADD:
                        if(sameRunnerType(left, right) && left.type == RUNNER_INT)
                        {
                            return (RunnerValue){.type = RUNNER_INT,.as.number = left.as.number + right.as.number};
                        }else
                        {
                            // printf("%d\n", left.type);
                        }
                    case OP_SUB:
                        if(sameRunnerType(left, right) && left.type == RUNNER_INT) return (RunnerValue){.type = RUNNER_INT,.as.number = left.as.number / right.as.number};
                    default:
                        return (RunnerValue){.type = RUNNER_INT,.as.number = 0};

                }
            default:
                // printf("%d\n", block.line);
                ret.type = RUNNER_NULL;
        }
    }

    return ret;
}
