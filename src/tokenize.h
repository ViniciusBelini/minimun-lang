#ifndef TOKENIZE_H
#define TOKENIZE_H

typedef enum
{
    TOKEN_EOF,
    TOKEN_ERROR,
    TOKEN_STRING, TOKEN_FLOAT, TOKEN_INT, TOKEN_BOOL, TOKEN_ID,
    TOKEN_NEWLINE, TOKEN_DELIMITER,
    TOKEN_OPERATOR, TOKEN_TWODOT,
    TOKEN_ADD, TOKEN_PUT,
    TOKEN_IF, TOKEN_END, TOKEN_ELSE,
    TOKEN_STATEMENT,
    TOKEN_SHOW
} TokenType;

typedef struct
{
    TokenType type;
    const char* start;
    int length;
    int line;
    int column;
} Token;

typedef struct
{
    const char* start;
    const char* current;
    int line;
    int column;
} Tokenize;

extern Tokenize tokenize;

void tokenize_init(const char* source);
Token tokenize_next_token();

bool is_at_end();

// Token make_token(TokenType type);

// Token error_token(const char* message);

#endif
