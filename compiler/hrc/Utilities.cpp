#include "Utilities.h"

OPEN_HRC_NAMESPACE

Utilities::Utilities()
{
}

Utilities::~Utilities()
{
}

void Utilities::write_token_list_to_file(FILE *file, const std::vector<TokenPtr> &tokens)
{
    int indent = 0;
    for (const TokenPtr &token : tokens) {
        std::string name = token->get_token_name();
        if (name[0] == 'T') {
            std::fprintf(file, ";\n");
            for (int i = 0; i < indent; ++i) {
                std::fprintf(file, " ");
            }
            continue;
        }
        if (name == "OPEN_BRACE") {
            indent += 2;
        }
        if (name == "CLOSE_BRACE") {
            indent -= 2;
            if (indent < 0)
                indent = 0;
        }

        if (name == "OPEN_BRACE" || name == "CLOSE_BRACE") {
            std::fprintf(file, "%s\n", name.c_str());
            for (int i = 0; i < indent; ++i) {
                std::fprintf(file, " ");
            }
            continue;
        }
        
        std::fprintf(file, "%s ", token->get_token_name());
    }
}

CLOSE_HRC_NAMESPACE