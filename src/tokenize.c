#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>
#include "tokenize.h"

Tokenize tokenize;

void tokenize_init(const char* source)
{
    tokenize.start = source;
    tokenize.current = source;
    tokenize.line = 1;
    tokenize.column = 1;
}

bool is_at_end()
{
    return *tokenize.current == '\0';
}

static char advance()
{
    tokenize.current++;
    tokenize.column++;
    return tokenize.current[-1];
}

static char back()
{
    tokenize.current--;
    tokenize.column--;
    return tokenize.current[-1];
}

static char peek()
{
    return *tokenize.current;
}

static char peek_next()
{
    if(is_at_end()) return '\0';
    return tokenize.current[1];
}

static char peek_back()
{
    return tokenize.current[-1];
}

static bool match(char expected)
{
    if(is_at_end()) return false;
    if(*tokenize.current != expected) return false;
    tokenize.current++;
    return true;
}

static Token make_token(TokenType type)
{
    Token token;
    token.type = type;
    token.start = tokenize.start;
    token.line = tokenize.line;
    token.column = tokenize.column;
    token.length = (int)(tokenize.current - tokenize.start);

    return token;
}

static void skip_whitespace()
{
    for(;;)
    {
        char c = peek();

        switch(c)
        {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '-': // comment
                if(peek_next() == '-')
                {
                    while(peek() != '\n' && !is_at_end()) advance();
                }
            default:
                return;
        }
    }
}

static Token error_token(const char* message)
{
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = tokenize.line;
    token.column = tokenize.column;
    return token;
}

static Token string()
{
    while((peek() != '"' || peek_back() == '\\') && !is_at_end())
    {
        if(peek() == '\n')
        {
            tokenize.line++;
            tokenize.column = 1;
        }
        advance();
    }
    if(is_at_end()) return error_token("Unterminated string");
    advance();
    return make_token(TOKEN_STRING);
}

static Token number()
{
    while(isdigit(peek())) advance();
    if(peek() == '.' && isdigit(peek_next()))
    {
        advance();
        while(isdigit(peek())) advance();
        return make_token(TOKEN_FLOAT);
    }
    return make_token(TOKEN_INT);
}

static TokenType check_keyword(int start, int length, const char* rest, TokenType type)
{
    if(tokenize.current - tokenize.start == start + length && memcmp(tokenize.start + start, rest, length) == 0)
    {
        return type;
    }
    return TOKEN_ID;
}

static TokenType identifier_type()
{
    switch(tokenize.start[0])
    {
        case 't':
            return check_keyword(1, 3, "rue", TOKEN_BOOL);
        case 'f':
            return check_keyword(1, 4, "alse", TOKEN_BOOL);
        case 'a':
            return check_keyword(1, 2, "dd", TOKEN_ADD);
            break;
        case 'p':
            return check_keyword(1, 2, "ut", TOKEN_PUT);
        case 'i':
            return check_keyword(1, 1, "f", TOKEN_IF);
        case 'e':
            switch(tokenize.start[1])
            {
                case 'l':
                    return check_keyword(1, 3, "lse", TOKEN_ELSE);
                case 'n':
                    return check_keyword(1, 2, "nd", TOKEN_END);
            }
        case 's':
            return check_keyword(1, 3, "how", TOKEN_SHOW);
    }
    return TOKEN_ID;
}

static Token identifier()
{
    while(isalnum(peek()) || peek() == '_') advance();
    return make_token(identifier_type());
}

Token tokenize_next_token()
{
    skip_whitespace();
    tokenize.start = tokenize.current;

    if(is_at_end()) return make_token(TOKEN_EOF);

    char c = advance();

    if(isdigit(c)) return number();
    if(isalpha(c) || c == '_') return identifier();

    char nC;

    switch(c)
    {
        case '\n':
            tokenize.line++;
            tokenize.column = 0;
            return make_token(TOKEN_NEWLINE);
        case ';':
            return make_token(TOKEN_DELIMITER);
        case ':':
            return make_token(TOKEN_TWODOT);
        case '"': return string();
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
            return make_token(TOKEN_OPERATOR);
        case '<':
            nC = advance();

            if(nC != '=')
            {
                nC = back();
            }

            return make_token(TOKEN_OPERATOR);
        case '>':
            nC = advance();

            if(nC != '=')
            {
                nC = back();
            }

            return make_token(TOKEN_OPERATOR);
        case '=':
            nC = advance();

            if(nC != '=')
            {
                return error_token("Unexpected character");
            }

            return make_token(TOKEN_OPERATOR);
        case '!':
            nC = advance();

            if(nC != '=')
            {
                return error_token("Unexpected character");
            }

            return make_token(TOKEN_OPERATOR);
        case '&':
            nC = advance();

            if(nC != '&')
            {
                return error_token("Unexpected character");
            }

            return make_token(TOKEN_OPERATOR);
        case '|':
            nC = advance();

            if(nC != '|')
            {
                return error_token("Unexpected character");
            }

            return make_token(TOKEN_OPERATOR);
    }
    return error_token("Unexpected character");
}
