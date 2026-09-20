#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "astnize.h"
#include "tokenize.h"

Ast *ast;
size_t astSize;
size_t astCurrent;

Token *tokensList;
size_t sizeTokens;

astHelper astHp;

void astnize_init(Token *tk, size_t sz)
{
    tokensList = tk;
    sizeTokens = sz;

    ast = NULL;
    astSize = 0;
    astCurrent = -1;

    astHp.ifTwoDot = false;
    astHp.getingStat = false;
}

static bool tokens_end()
{
    return sizeTokens == astCurrent + 1;
}

static Token peek()
{
    return tokensList[astCurrent];
}
static Token peek_back()
{
    return tokensList[astCurrent - 1];
}
static Token peek_next()
{
    return tokensList[astCurrent+1];
}

static Token back()
{
    astCurrent--;
    return peek();
}
static Token next()
{
    astCurrent++;
    return peek();
}

static void append_ast(Ast newAst)
{
    astSize++;

    ast = realloc(ast, astSize * sizeof(newAst));

    ast[astSize - 1] = newAst;
}

static Ast makeNode(TokenType type)
{
    Token token = peek();

    Ast newAst;

    newAst.type = type;
    newAst.line = token.line;
    newAst.column = token.column;

    switch(type)
    {
        case TOKEN_ERROR:
        case TOKEN_STRING:
        case TOKEN_ID:
            newAst.strToken.start = token.start;
            newAst.strToken.length = token.length;

            break;
        case TOKEN_FLOAT:
        {
            char buffer[token.length + 1];

            memcpy(buffer, token.start, token.length);
            buffer[token.length] = '\0';

            size_t decimalPlaces = 0;
            for(size_t i = 0;i < token.length;i++)
            {
                if(token.start[i] == '.')
                {
                    decimalPlaces = token.length - i - 1;
                }
            }

            newAst.floatToken.value = strtod(buffer, NULL);
            newAst.floatToken.length = decimalPlaces;

            break;
        }
        case TOKEN_INT:
        {
            char buffer[token.length + 1];

            memcpy(buffer, token.start, token.length);
            buffer[token.length] = '\0';

            newAst.intToken = (int)strtol(buffer, NULL, 10);

            break;
        }
        case TOKEN_BOOL:
        {
            char buffer[token.length + 1];

            memcpy(buffer, token.start, token.length);
            buffer[token.length] = '\0';

            if(strcmp(buffer, "true") == 0)
            {
                newAst.boolToken = true;
            }else
            {
                newAst.boolToken = false;
            }

            break;
        }

    }

    return newAst;
}

static Ast errorNode(const char* message)
{
    Token token = peek();

    Ast newAst;

    newAst.type = TOKEN_ERROR;
    newAst.line = token.line;
    newAst.column = token.column;
    newAst.strToken.start = message;
    newAst.strToken.length = (int)strlen(message);

    return newAst;
}

static Ast astOperator()
{
    Token token = back();
    bool opIdAc = true;

    Ast **tksAst = NULL;
    size_t tksAstSize = 0;

    int *opAst = NULL;
    size_t opAstSize = 0;

    while(!tokens_end())
    {
        token = next();

        switch(token.type)
        {
            case TOKEN_STRING:
            case TOKEN_FLOAT:
            case TOKEN_INT:
            case TOKEN_BOOL:
            case TOKEN_ID:
                if(opIdAc)
                {
                    tksAstSize++;

                    token = back();
                    Ast tokenAst = astnize(true);

                    tksAst = realloc(tksAst, tksAstSize * sizeof(tokenAst));
                    tksAst[tksAstSize - 1] = &tokenAst;

                    opIdAc = false;
                }
                break;
            case TOKEN_OPERATOR:
                if(!opIdAc)
                {
                    opAstSize++;
                    opAst = realloc(opAst, opAstSize * sizeof(int));
                    // opAst[opAstSize - 1] = *token.start;

                    switch(*token.start)
                    {
                        case '*':
                            opAst[opAstSize - 1] = OP_MUL;
                            break;
                        case '/':
                            opAst[opAstSize - 1] = OP_DIV;
                            break;
                        case '%':
                            opAst[opAstSize - 1] = OP_MOD;
                            break;

                        case '+':
                            opAst[opAstSize - 1] = OP_ADD;
                            break;
                        case '-':
                            opAst[opAstSize - 1] = OP_SUB;
                            break;

                        case '<':
                            if(token.start[1] == '=')
                            {
                                opAst[opAstSize - 1] = OP_LTE;
                                break;
                            }
                            opAst[opAstSize - 1] = OP_LT;
                            break;
                        case '>':
                            if(token.start[1] == '=')
                            {
                                opAst[opAstSize - 1] = OP_GTE;
                                break;
                            }
                            opAst[opAstSize - 1] = OP_GT;
                            break;

                        case '=':
                            if(token.start[1] != '=')
                            {
                                return errorNode("Unexpected character");
                            }
                            opAst[opAstSize - 1] = OP_EQ;
                            break;
                        case '!':
                            if(token.start[1] != '=')
                            {
                                return errorNode("Unexpected character");
                            }
                            opAst[opAstSize - 1] = OP_NEQ;
                            break;

                        case '|':
                            if(token.start[1] != '|')
                            {
                                return errorNode("Unexpected character");
                            }
                            opAst[opAstSize - 1] = OP_OR;
                            break;
                        case '&':
                            if(token.start[1] != '&')
                            {
                                return errorNode("Unexpected character");
                            }
                            opAst[opAstSize - 1] = OP_AND;
                            break;
                    }

                    opIdAc = true;

                    Token nTk = peek_next();

                    switch(nTk.type)
                    {
                        case TOKEN_STRING:
                        case TOKEN_FLOAT:
                        case TOKEN_INT:
                        case TOKEN_BOOL:
                        case TOKEN_ID:
                            break;
                        default:
                            size_t len = strlen("Expected an expression after ");
                            char *message = malloc(len + 2); // e nunca vou dar free nisso aqui :(

                                if(message == NULL) return errorNode("Unexpected character");

                                memcpy(message, "Expected an expression after ", len);
                        message[len] = *token.start;
                        message[len + 1] = '\0';

                        return errorNode(message);
                    }

                    break;
                }
                return errorNode("Unexpected character"); // WARNING finalizou com um operando e nao um id
        }
    }

    if(tksAstSize != opAstSize + 1)
    {
        return errorNode("Unexpected character");
    }

    int opPrece[] = {OP_MUL, OP_DIV, OP_MOD,  OP_ADD, OP_SUB,  OP_LT, OP_LTE, OP_GT, OP_GTE,  OP_EQ, OP_NEQ,  OP_AND,  OP_OR};
    int opPreceI = 0;
    while(opPreceI <= 13)
    {
        opPreceI++;

        int iI = 0;
        while(iI < opAstSize)
        {
            if(opPrece[opPreceI - 1] == opAst[iI])
            {
                Ast *x = tksAst[iI];
                Ast *y = tksAst[iI + 1];

                Ast result;
                result.type = TOKEN_OPERATOR;
                result.line = x->line;
                result.column = x->column;
                result.op.left = x;
                result.op.right = y;
                result.op.type = opPrece[opPreceI - 1];

                // switch(opPrece[opPreceI - 1])
                // {
                //     case '*':
                //         result.op.type = OP_MUL;
                //         break;
                //     case '/':
                //         result.op.type = OP_DIV;
                //         break;
                //     case '+':
                //         result.op.type = OP_ADD;
                //         break;
                //     case '-':
                //         result.op.type = OP_SUB;
                //         break;
                //     default:
                //         break;
                // }

                tksAst[iI] = &result;

                memmove(
                    &tksAst[iI + 1],
                    &tksAst[iI + 2],
                    (tksAstSize - iI - 2) * sizeof(Ast)
                );

                tksAstSize--;

                tksAst = realloc(tksAst, tksAstSize * sizeof(Ast));

                if(iI < opAstSize)
                {
                    memmove(
                        &opAst[iI],
                        &opAst[iI + 1],
                        opAstSize - iI - 1
                    );

                    opAstSize--;

                    int *tmp = realloc(opAst, opAstSize * sizeof *opAst);

                    if(tmp != NULL || opAstSize == 0) opAst = tmp;
                }
            }else
            {
                iI++;
            }
        }
    }

    Ast result = *tksAst[0];
    // free(tksAst[0]);
    // free(tksAst);
    return result;
}

static Ast astOperatorBACKUP_ORIGINAL()
{
    Token token = back();
    bool opIdAc = true;

    Ast *tksAst = NULL;
    size_t tksAstSize = 0;

    int *opAst = NULL;
    size_t opAstSize = 0;

    while(!tokens_end())
    {
        token = next();

        switch(token.type)
        {
            case TOKEN_STRING:
            case TOKEN_FLOAT:
            case TOKEN_INT:
            case TOKEN_BOOL:
            case TOKEN_ID:
                if(opIdAc)
                {
                    tksAstSize++;

                    token = back();
                    Ast tokenAst = astnize(true);

                    tksAst = realloc(tksAst, tksAstSize * sizeof(tokenAst));
                    tksAst[tksAstSize - 1] = tokenAst;

                    opIdAc = false;
                }
                break;
            case TOKEN_OPERATOR:
                if(!opIdAc)
                {
                    opAstSize++;
                    opAst = realloc(opAst, opAstSize * sizeof(int));
                    // opAst[opAstSize - 1] = *token.start;

                    switch(*token.start)
                    {
                        case '*':
                            opAst[opAstSize - 1] = OP_MUL;
                            break;
                        case '/':
                            opAst[opAstSize - 1] = OP_DIV;
                            break;
                        case '%':
                            opAst[opAstSize - 1] = OP_MOD;
                            break;

                        case '+':
                            opAst[opAstSize - 1] = OP_ADD;
                            break;
                        case '-':
                            opAst[opAstSize - 1] = OP_SUB;
                            break;

                        case '<':
                            if(token.start[1] == '=')
                            {
                                opAst[opAstSize - 1] = OP_LTE;
                                break;
                            }
                            opAst[opAstSize - 1] = OP_LT;
                            break;
                        case '>':
                            if(token.start[1] == '=')
                            {
                                opAst[opAstSize - 1] = OP_GTE;
                                break;
                            }
                            opAst[opAstSize - 1] = OP_GT;
                            break;

                        case '=':
                            if(token.start[1] != '=')
                            {
                                return errorNode("Unexpected character");
                            }
                            opAst[opAstSize - 1] = OP_EQ;
                            break;
                        case '!':
                            if(token.start[1] != '=')
                            {
                                return errorNode("Unexpected character");
                            }
                            opAst[opAstSize - 1] = OP_NEQ;
                            break;

                        case '|':
                            if(token.start[1] != '|')
                            {
                                return errorNode("Unexpected character");
                            }
                            opAst[opAstSize - 1] = OP_OR;
                            break;
                        case '&':
                            if(token.start[1] != '&')
                            {
                                return errorNode("Unexpected character");
                            }
                            opAst[opAstSize - 1] = OP_AND;
                            break;
                    }

                    opIdAc = true;

                    Token nTk = peek_next();

                    switch(nTk.type)
                    {
                        case TOKEN_STRING:
                        case TOKEN_FLOAT:
                        case TOKEN_INT:
                        case TOKEN_BOOL:
                        case TOKEN_ID:
                            break;
                        default:
                            size_t len = strlen("Expected an expression after ");
                            char *message = malloc(len + 2); // e nunca vou dar free nisso aqui :(

                            if(message == NULL) return errorNode("Unexpected character");

                            memcpy(message, "Expected an expression after ", len);
                            message[len] = *token.start;
                            message[len + 1] = '\0';

                            return errorNode(message);
                    }

                    break;
                }
                return errorNode("Unexpected character"); // WARNING finalizou com um operando e nao um id
        }
    }

    if(tksAstSize != opAstSize + 1)
    {
        return errorNode("Unexpected character");
    }

    int opPrece[] = {OP_MUL, OP_DIV, OP_MOD,  OP_ADD, OP_SUB,  OP_LT, OP_LTE, OP_GT, OP_GTE,  OP_EQ, OP_NEQ,  OP_AND,  OP_OR};
    int opPreceI = 0;
    while(opPreceI <= 13)
    {
        opPreceI++;

        int iI = 0;
        while(iI < opAstSize)
        {
            if(opPrece[opPreceI - 1] == opAst[iI])
            {
                Ast *x = &tksAst[iI];
                Ast *y = &tksAst[iI + 1];

                Ast result;
                result.type = TOKEN_OPERATOR;
                result.line = x->line;
                result.column = x->column;
                result.op.left = x;
                result.op.right = y;
                result.op.type = opPrece[opPreceI - 1];

                // switch(opPrece[opPreceI - 1])
                // {
                //     case '*':
                //         result.op.type = OP_MUL;
                //         break;
                //     case '/':
                //         result.op.type = OP_DIV;
                //         break;
                //     case '+':
                //         result.op.type = OP_ADD;
                //         break;
                //     case '-':
                //         result.op.type = OP_SUB;
                //         break;
                //     default:
                //         break;
                // }

                tksAst[iI] = result;

                memmove(
                    &tksAst[iI + 1],
                    &tksAst[iI + 2],
                    (tksAstSize - iI - 2) * sizeof(Ast)
                );

                tksAstSize--;

                tksAst = realloc(tksAst, tksAstSize * sizeof(Ast));

                if(iI < opAstSize)
                {
                    memmove(
                        &opAst[iI],
                        &opAst[iI + 1],
                        opAstSize - iI - 1
                    );

                    opAstSize--;

                    int *tmp = realloc(opAst, opAstSize * sizeof *opAst);

                    if(tmp != NULL || opAstSize == 0) opAst = tmp;
                }
            }else
            {
                iI++;
            }
        }
    }

    return tksAst[0];
}

// a primeira função foi feita 100% por mim
// a segunda foi modificada pelo ChatGPT
// havia um bug e eu estava com preguiça

static Ast astOperatorBACKUP()
{
    Token token = back();

    bool opIdAc = true;

    /*
     * Stack de operandos.
     *
     * Cada Ast possui endereço estável porque é alocado
     * individualmente.
     */
    Ast **values = NULL;
    size_t valuesSize = 0;

    /*
     * Stack de operadores.
     */
    int *operators = NULL;
    size_t operatorsSize = 0;

    /*
     * Função auxiliar não pode ser declarada portavelmente
     * dentro desta função em C padrão, então a redução é feita
     * diretamente abaixo quando necessário.
     */
    while(!tokens_end())
    {
        token = next();

        switch(token.type)
        {
            /*
             * ---------------------------------------------------------
             * OPERANDO
             * ---------------------------------------------------------
             */
            case TOKEN_STRING:
            case TOKEN_FLOAT:
            case TOKEN_INT:
            case TOKEN_BOOL:
            case TOKEN_ID:
            {
                /*
                 * Dois operandos consecutivos são inválidos.
                 */
                if(!opIdAc)
                {
                    free(values);
                    free(operators);

                    return errorNode("Unexpected character");
                }

                token = back();

                Ast *value = malloc(sizeof *value);

                if(value == NULL)
                {
                    free(values);
                    free(operators);

                    return errorNode("Out of memory");
                }

                /*
                 * astnize() continua sendo usado exatamente como
                 * anteriormente.
                 */
                *value = astnize(true);

                Ast **tmp = realloc(
                    values,
                    (valuesSize + 1) * sizeof *values
                );

                if(tmp == NULL)
                {
                    free(value);
                    free(values);
                    free(operators);

                    return errorNode("Out of memory");
                }

                values = tmp;
                values[valuesSize++] = value;

                opIdAc = false;

                break;
            }

            /*
             * ---------------------------------------------------------
             * OPERADOR
             * ---------------------------------------------------------
             */
            case TOKEN_OPERATOR:
            {
                /*
                 * Dois operadores consecutivos são inválidos.
                 */
                if(opIdAc)
                {
                    free(values);
                    free(operators);

                    return errorNode("Unexpected character");
                }

                int currentOp;

                switch(*token.start)
                {
                    case '*':
                        currentOp = OP_MUL;
                        break;

                    case '/':
                        currentOp = OP_DIV;
                        break;

                    case '%':
                        currentOp = OP_MOD;
                        break;

                    case '+':
                        currentOp = OP_ADD;
                        break;

                    case '-':
                        currentOp = OP_SUB;
                        break;

                    case '<':
                        if(token.start[1] == '=')
                            currentOp = OP_LTE;
                    else
                        currentOp = OP_LT;
                    break;

                    case '>':
                        if(token.start[1] == '=')
                            currentOp = OP_GTE;
                    else
                        currentOp = OP_GT;
                    break;

                    case '=':
                        if(token.start[1] != '=')
                        {
                            free(values);
                            free(operators);

                            return errorNode("Unexpected character");
                        }

                        currentOp = OP_EQ;
                        break;

                    case '!':
                        if(token.start[1] != '=')
                        {
                            free(values);
                            free(operators);

                            return errorNode("Unexpected character");
                        }

                        currentOp = OP_NEQ;
                        break;

                    case '|':
                        if(token.start[1] != '|')
                        {
                            free(values);
                            free(operators);

                            return errorNode("Unexpected character");
                        }

                        currentOp = OP_OR;
                        break;

                    case '&':
                        if(token.start[1] != '&')
                        {
                            free(values);
                            free(operators);

                            return errorNode("Unexpected character");
                        }

                        currentOp = OP_AND;
                        break;

                    default:
                        free(values);
                        free(operators);

                        return errorNode("Unexpected character");
                }

                /*
                 * Verifica se existe um operando depois do operador.
                 */
                Token nTk = peek_next();

                switch(nTk.type)
                {
                    case TOKEN_STRING:
                    case TOKEN_FLOAT:
                    case TOKEN_INT:
                    case TOKEN_BOOL:
                    case TOKEN_ID:
                        break;

                    default:
                    {
                        size_t len =
                        strlen("Expected an expression after ");

                        char *message = malloc(len + 2);

                        if(message == NULL)
                        {
                            free(values);
                            free(operators);

                            return errorNode("Unexpected character");
                        }

                        memcpy(
                            message,
                            "Expected an expression after ",
                            len
                        );

                        message[len] = *token.start;
                        message[len + 1] = '\0';

                        free(values);
                        free(operators);

                        /*
                         * Mantém o comportamento original.
                         */
                        return errorNode(message);
                    }
                }

                /*
                 * -----------------------------------------------------
                 * RESOLVE OPERADORES DE MAIOR OU IGUAL PRECEDÊNCIA
                 * -----------------------------------------------------
                 *
                 * Exemplo:
                 *
                 *     2 * 2 + ...
                 *
                 * quando chega o '+', o '*' já é reduzido.
                 *
                 * Para:
                 *
                 *     2 + 2 * ...
                 *
                 * o '+' permanece na stack porque sua precedência
                 * é menor que a do '*'.
                 */
                while(operatorsSize > 0)
                {
                    int previousOp =
                    operators[operatorsSize - 1];

                    int previousPrecedence;
                    int currentPrecedence;

                    /*
                     * Precedência do operador anterior.
                     */
                    switch(previousOp)
                    {
                        case OP_MUL:
                        case OP_DIV:
                        case OP_MOD:
                            previousPrecedence = 6;
                            break;

                        case OP_ADD:
                        case OP_SUB:
                            previousPrecedence = 5;
                            break;

                        case OP_LT:
                        case OP_LTE:
                        case OP_GT:
                        case OP_GTE:
                            previousPrecedence = 4;
                            break;

                        case OP_EQ:
                        case OP_NEQ:
                            previousPrecedence = 3;
                            break;

                        case OP_AND:
                            previousPrecedence = 2;
                            break;

                        case OP_OR:
                            previousPrecedence = 1;
                            break;

                        default:
                            previousPrecedence = 0;
                            break;
                    }

                    /*
                     * Precedência do operador atual.
                     */
                    switch(currentOp)
                    {
                        case OP_MUL:
                        case OP_DIV:
                        case OP_MOD:
                            currentPrecedence = 6;
                            break;

                        case OP_ADD:
                        case OP_SUB:
                            currentPrecedence = 5;
                            break;

                        case OP_LT:
                        case OP_LTE:
                        case OP_GT:
                        case OP_GTE:
                            currentPrecedence = 4;
                            break;

                        case OP_EQ:
                        case OP_NEQ:
                            currentPrecedence = 3;
                            break;

                        case OP_AND:
                            currentPrecedence = 2;
                            break;

                        case OP_OR:
                            currentPrecedence = 1;
                            break;

                        default:
                            currentPrecedence = 0;
                            break;
                    }

                    /*
                     * Se o operador anterior tem precedência menor,
                     * paramos.
                     */
                    if(previousPrecedence < currentPrecedence)
                        break;

                    /*
                     * Para operadores com a mesma precedência,
                     * fazemos associação à esquerda:
                     *
                     *     2 - 2 - 2
                     *
                     * vira:
                     *
                     *     (2 - 2) - 2
                     */

                    /*
                     * Precisamos de pelo menos dois operandos.
                     */
                    if(valuesSize < 2)
                    {
                        free(values);
                        free(operators);

                        return errorNode("Unexpected character");
                    }

                    /*
                     * Retira o operador.
                     */
                    operatorsSize--;

                    /*
                     * Retira os dois operandos.
                     */
                    Ast *right = values[valuesSize - 1];
                    Ast *left  = values[valuesSize - 2];

                    valuesSize -= 2;

                    /*
                     * Cria o novo nó.
                     */
                    Ast *result = malloc(sizeof *result);

                    if(result == NULL)
                    {
                        free(values);
                        free(operators);

                        return errorNode("Out of memory");
                    }

                    result->type = TOKEN_OPERATOR;
                    result->line = left->line;
                    result->column = left->column;

                    result->op.left = left;
                    result->op.right = right;
                    result->op.type = previousOp;

                    /*
                     * Coloca o resultado de volta na stack.
                     */
                    Ast **tmp = realloc(
                        values,
                        (valuesSize + 1) * sizeof *values
                    );

                    if(tmp == NULL)
                    {
                        free(result);
                        free(values);
                        free(operators);

                        return errorNode("Out of memory");
                    }

                    values = tmp;
                    values[valuesSize++] = result;
                }

                /*
                 * Coloca o operador atual na stack.
                 */
                int *tmp = realloc(
                    operators,
                    (operatorsSize + 1) * sizeof *operators
                );

                if(tmp == NULL)
                {
                    free(values);
                    free(operators);

                    return errorNode("Out of memory");
                }

                operators = tmp;
                operators[operatorsSize++] = currentOp;

                opIdAc = true;

                break;
            }

            default:
                break;
        }
    }

    /*
     * A expressão não pode terminar com operador.
     */
    if(opIdAc)
    {
        free(values);
        free(operators);

        return errorNode("Unexpected character");
    }

    /*
     * Deve existir exatamente um operando a mais que operadores
     * antes da redução final.
     */
    if(valuesSize != operatorsSize + 1)
    {
        free(values);
        free(operators);

        return errorNode("Unexpected character");
    }

    /*
     * -------------------------------------------------------------
     * REDUÇÃO FINAL
     * -------------------------------------------------------------
     *
     * Agora não existem mais operadores chegando.
     *
     * Portanto reduzimos tudo que sobrou na stack.
     */
    while(operatorsSize > 0)
    {
        if(valuesSize < 2)
        {
            free(values);
            free(operators);

            return errorNode("Unexpected character");
        }

        int op = operators[operatorsSize - 1];
        operatorsSize--;

        Ast *right = values[valuesSize - 1];
        Ast *left  = values[valuesSize - 2];

        valuesSize -= 2;

        Ast *result = malloc(sizeof *result);

        if(result == NULL)
        {
            free(values);
            free(operators);

            return errorNode("Out of memory");
        }

        result->type = TOKEN_OPERATOR;
        result->line = left->line;
        result->column = left->column;

        result->op.left = left;
        result->op.right = right;
        result->op.type = op;

        Ast **tmp = realloc(
            values,
            (valuesSize + 1) * sizeof *values
        );

        if(tmp == NULL)
        {
            free(result);
            free(values);
            free(operators);

            return errorNode("Out of memory");
        }

        values = tmp;
        values[valuesSize++] = result;
    }

    /*
     * Se tudo deu certo, resta exatamente a raiz da AST.
     */
    if(valuesSize != 1)
    {
        free(values);
        free(operators);

        return errorNode("Unexpected character");
    }

    /*
     * Mantemos a assinatura original:
     *
     *     static Ast astOperator()
     *
     * portanto retornamos Ast por valor.
     *
     * Os filhos continuam válidos porque foram alocados
     * individualmente.
     */
    Ast result = *values[0];

    free(values[0]);
    free(values);
    free(operators);

    return result;


}

// DANGER BUG: NÃO SEI O PORQUE AINDA MAS ENTRA EM LOOP DENTRO DA FUNÇÃO OPERADORES - CONSERTAR ISSO

static bool verifyIsConf(TokenType type)
{
    switch(type)
    {
        case TOKEN_BOOL:
        case TOKEN_ID:
            return true;
        default:
            return false;
    }
}

static bool verifyTokenDeep(TokenType type)
{
    switch(type)
    {
        case TOKEN_IF:
            return true;
        default:
            return false;
    }
}

static Ast getStatment(bool inlineT, bool inIfStatement, size_t kindStat)
{
    size_t currentBackup = astCurrent;
    size_t deepEnd = 0;

    Ast *statements = NULL;
    size_t sizeSt = 0;

    if(kindStat == 1) back();

    while(!tokens_end())
    {
        Token token = next();

        if(!inlineT && (token.type != TOKEN_ELSE || kindStat != 1))
        {
            if(token.type == TOKEN_END || (token.type == TOKEN_ELSE && inIfStatement) || (kindStat == 1 && sizeSt > 0))
            {
                if(deepEnd == 0 || (kindStat == 1 && sizeSt > 0))
                {
                    Ast newAst;
                    newAst.type = TOKEN_STATEMENT;
                    newAst.line = peek().line;
                    newAst.column = peek().column;
                    newAst.block.statements = statements;
                    newAst.block.size = sizeSt;

                    return newAst;
                }
                deepEnd--;
            }else if(verifyTokenDeep(token.type))
            {
                deepEnd++;
            }
        }else if(inlineT)
        {
            if(token.type == TOKEN_NEWLINE || token.type == TOKEN_EOF)
            {
                if(deepEnd == 0)
                {
                    Ast newAst;
                    newAst.type = TOKEN_STATEMENT;
                    newAst.line = peek().line;
                    newAst.column = peek().column;
                    newAst.block.statements = statements;
                    newAst.block.size = sizeSt;

                    return newAst;
                }
                deepEnd--;
            }else if(verifyTokenDeep(token.type))
            {
                deepEnd++;
            }
        }

        if(tokens_end()) break;

        Ast nTk = astnize(false);

        sizeSt++;
        statements = realloc(statements, sizeSt * sizeof(nTk));
        statements[sizeSt - 1] = nTk;
    }

    astCurrent = currentBackup;
    return errorNode("Expected a closing token for statement");
}

Ast astnize(bool firstReturn)
{
    while(!tokens_end())
    {
        Token token = next();

        switch(token.type)
        {
            case TOKEN_END:
            case TOKEN_ELSE:
                if(astHp.getingStat)
                {
                    token = back();

                    return makeNode(token.type);
                }
                break;
            case TOKEN_NEWLINE:
            case TOKEN_EOF:
                Ast newAst;
                newAst.type = token.type;
                newAst.line = token.line;
                newAst.column = token.column;

                return newAst;
            case TOKEN_STRING:
            case TOKEN_FLOAT:
            case TOKEN_INT:
            case TOKEN_BOOL:
            case TOKEN_ID:
                Token next_token = peek_next();

                if(!firstReturn && next_token.type == TOKEN_OPERATOR)
                {
                    return astOperator();
                }else if(firstReturn || next_token.type == TOKEN_EOF || next_token.type == TOKEN_NEWLINE || next_token.type == TOKEN_DELIMITER || (peek_next().type == TOKEN_TWODOT && astHp.ifTwoDot))
                {
                    return makeNode(token.type);
                }else
                {
                    return errorNode("Unexpected character"); // erro desconhecido - estou com preguica
                }
                break;
            case TOKEN_ADD:
                Ast identTk = astnize(true);
                if(identTk.type == TOKEN_ID)
                {
                    Ast typeTk = astnize(true);
                    if(typeTk.type == TOKEN_ID)
                    {
                        if(peek_next().type == TOKEN_EOF || peek_next().type == TOKEN_NEWLINE || peek_next().type == TOKEN_DELIMITER)
                        {
                            Ast newAst;
                            newAst.type = TOKEN_ADD;
                            newAst.line = token.line;
                            newAst.column = token.column;
                            newAst.addToken.ident = &identTk;
                            newAst.addToken.type = &typeTk;

                            return newAst;
                        }
                        return errorNode("Unexpected character");
                    }
                    return errorNode("Expected a type identifier for 'add'");
                }
                return errorNode("Expected an identifier after 'add'");
            case TOKEN_PUT:
                identTk = astnize(true);
                if(identTk.type == TOKEN_ID)
                {
                    if(peek_next().type != TOKEN_EOF && peek_next().type != TOKEN_NEWLINE)
                    {
                        Ast valueTk = astnize(false);

                        if(valueTk.type != TOKEN_EOF && valueTk.type != TOKEN_NEWLINE && valueTk.type != TOKEN_ERROR)
                        {
                            Ast newAst;
                            newAst.type = TOKEN_PUT;
                            newAst.line = token.line;
                            newAst.column = token.column;
                            newAst.putToken.ident = &identTk;
                            newAst.putToken.value = &valueTk;

                            return newAst;
                        }
                        return errorNode("Unexpected character");
                    }

                    return errorNode("Expected a value for 'put'");
                }
                return errorNode("Expected an identifier after 'put'");
            case TOKEN_IF:
                if(peek_next().type != TOKEN_NEWLINE && peek_next().type != TOKEN_EOF)
                {
                    astHp.ifTwoDot = true;

                    Ast condIf = astnize(false);
                    bool ifStInline = false;

                    astHp.ifTwoDot = false;

                    if(verifyIsConf(condIf.type))
                    {
                        if(peek_next().type == TOKEN_NEWLINE || peek_next().type == TOKEN_TWODOT)
                        {
                            if(peek_next().type == TOKEN_TWODOT) ifStInline = true;

                            astHp.getingStat = true;

                            Ast bodyIf = getStatment(ifStInline, true, 0);
                            Ast bodyElse;

                            token = peek();

                            if(token.type == TOKEN_ELSE)
                            {
                                Token backupNextTk = peek_next();

                                if(backupNextTk.type == TOKEN_IF)
                                {
                                    bodyElse = getStatment(ifStInline, true, 1);
                                }else
                                {
                                    bodyElse = getStatment(ifStInline, true, 0);
                                }

                                token = peek();

                                if(token.type != TOKEN_END && backupNextTk.type != TOKEN_IF) return errorNode("Expected a closing token for 'else' statement");
                            }else if(token.type != TOKEN_END) return errorNode("Expected a closing token for 'else' statement");

                            astHp.getingStat = false;

                            if(bodyIf.type == TOKEN_ERROR) return bodyIf;

                            Ast ifStat;
                            ifStat.type = TOKEN_IF;
                            ifStat.ifStatment.cond = &condIf;
                            ifStat.ifStatment.then_branch = &bodyIf;
                            ifStat.ifStatment.else_branch = &bodyElse;

                            return ifStat;
                        }
                    }
                }

                return errorNode("Expected a condition for 'if' statment");
            case TOKEN_SHOW:
                astHp.ifTwoDot = false;
                astHp.getingStat = false;

                Ast showArgs;
                Ast *argsAst = NULL;
                Ast argI = astnize(false);

                argsAst = realloc(argsAst, 1 * sizeof(argI));
                argsAst[0] = argI;

                showArgs.type = TOKEN_SHOW;
                showArgs.line = token.line;
                showArgs.column = token.column;
                showArgs.block.statements = argsAst;
                showArgs.block.size = 1;

                return showArgs;
            default:
                return errorNode("Undefined expression");
        }
    }

    return errorNode("Undefined expression");
}

Ast firstAst(bool firstReturn)
{
    Ast *astList = NULL;
    size_t astSize = 0;

    Token token = peek();

    while(!tokens_end())
    {
        Ast newAst = astnize(firstReturn);

        astSize++;
        astList = realloc(astList, astSize * sizeof(newAst));
        astList[astSize - 1] = newAst;
    }

    Ast newAst;
    newAst.type = TOKEN_STATEMENT;
    newAst.line = token.line;
    newAst.column = token.column;
    newAst.block.statements = astList;
    newAst.block.size = astSize;

    return newAst;
}
