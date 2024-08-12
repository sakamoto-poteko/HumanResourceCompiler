#include "Utilities.h"

OPEN_HRC_NAMESPACE

Utilities::Utilities()
{
}

Utilities::~Utilities()
{
}

void Utilities::write_token_list_to_file(FILE *file, const std::vector<GCToken> &tokens)
{
    for (const GCToken &token : tokens) {
        const char *name = token->get_token_name();
        if (name[0] == ';') {
            std::fprintf(file, ";\n");
            continue;
        }
        std::fprintf(file, "%s ", token->get_token_name());
    }
}

CLOSE_HRC_NAMESPACE