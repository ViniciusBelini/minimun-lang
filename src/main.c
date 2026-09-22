#include <stdio.h>
#include <stdlib.h>
#include "tokenize.h"
#include "astnize.h"
#include "runner.h"

// descobri agora que `int argc, char* argv[]` faz com que o c envie os args para a funcao

char* read_file(const char* path)
{
    FILE* file = fopen(path, "rb");
    if(file == NULL)
    {
        printf("Could not open file \"$s\"\n", path);
        exit(74);
    }

    fseek(file, 0L, SEEK_END); // move o cursor para o final
    size_t fileSize = ftell(file); // diz onde o cursor esta
    rewind(file); // volta o cursor para o inicio

    char* buffer = (char*)malloc(fileSize + 1); // usa o tamanho do arquivo para alocar x + 1 bytes
    if(buffer == NULL)
    {
        printf("Not enough memory to read \"$s\"\n", path);
        exit(74);
    }

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file); // le e coloca a partir de buffer os dados
    if(bytesRead < fileSize)
    {
        printf("Could not read file \"$s\"\n", path);
        exit(74);
    }
    buffer[bytesRead] = '\0';

    fclose(file);

    return buffer;
}

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        printf("Usage: minimun <file>\n");
        return 0;
    }

    char* source = read_file(argv[1]);

    tokenize_init(source);

    Token *token = NULL;
    size_t sizeTokens = 0;

    while(!is_at_end())
    {
        Token t = tokenize_next_token();

        sizeTokens++;
        token = realloc(token, sizeTokens * sizeof(t));

        token[sizeTokens - 1] = t;
    }

    astnize_init(token, sizeTokens);
    Ast ast = firstAst(false);

    runner(ast);

    free(source);
    free(token);

    return 0;
}
