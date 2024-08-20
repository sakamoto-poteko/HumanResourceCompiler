#include <cstdio>
#include <ASTNode.h>
#include <ASTNodeVisitor.h>

int yyparse(void);
extern std::shared_ptr<SyntaxNode> root; 

const char *path = "/Users/afa/project/HumanResourceCompiler/design/hrl.ebnf";

int main(int argc, char **argv) {
  FILE *file = std::fopen(path, "r");
  if (!file) {
    std::perror(path);
    return 1;
  }
  // Redirect yyin to read from the file instead of stdin
  extern FILE *yyin;
  yyin = file;

  if (yyparse() == 0) {
    printf("Parsing completed successfully.\n");
  } else {
    printf("Parsing failed.\n");
  }

  ASTPrintVisitor visitor;
  visitor.visit(root);

  fclose(yyin);

  return 0;
}