#include "HRLLexer.h"

using namespace hrl::lexer;

int main(int argc, char **argv)
{
    HRLLexer lexer;
    // FILE *file = fopen("/home/afa/projects/HumanResourceCompiler/design/lang.hrl", "r");
    FILE *file = std::fopen("/home/afa/anna-lang/demo/sample.anna", "r");

    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    char str[100];
    str[99] = 0;
    fread(str, 10, 10, file);
    printf("%s", str);
    fseek(file, 0, SEEK_END);

    std::vector<GCToken> tokens;
    lexer.lex(file, tokens);
    
    fclose(file);
}