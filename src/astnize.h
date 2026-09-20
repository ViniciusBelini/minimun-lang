#ifndef ASTNIZE_H
#define ASTNIZE_H

#include "tokenize.h"
typedef enum
{
    OP_MUL, // *
    OP_DIV, // /
    OP_MOD, // %

    OP_ADD, // +
    OP_SUB, // -

    OP_LT,  // <
    OP_LTE, // <=
    OP_GT,  // >
    OP_GTE, // >=

    OP_EQ,  // ==
    OP_NEQ, // !=

    OP_AND, // &&

    OP_OR   // ||
} OpType;

typedef struct Ast
{
    TokenType type;
    int line;
    int column;

    union
    {
        struct
        {
            const char* start;
            int length;
        } strToken;
        int intToken;
        struct
        {
            double value;
            int length;
        } floatToken;
        bool boolToken;

        struct
        {
            struct Ast* left;
            struct Ast* right;
            OpType type;
        } op;

        struct
        {
            struct Ast* ident;
            struct Ast* type;
        } addToken;
        struct
        {
            struct Ast* ident;
            struct Ast* value;
        } putToken;

        struct
        {
            struct Ast *statements;
            size_t size;
        } block;

        struct
        {
            struct Ast* cond;
            struct Ast* then_branch;
            struct Ast* else_branch;
        } ifStatment;

        struct Ast* showArgs;
    };
} Ast;

typedef struct
{
    bool ifTwoDot;
    bool getingStat;
} astHelper;

extern Ast *ast;
extern size_t astSize;
extern size_t astCurrent;

extern Token *tokensList;
extern size_t sizeTokens;

extern astHelper astHp;

void astnize_init(Token *tk, size_t sz);
Ast astnize(bool firstReturn);
Ast firstAst(bool firstReturn);

#endif
