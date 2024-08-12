#ifndef UTILITIES_H
#define UTILITIES_H

#include <vector>

#include "HRLToken.h"
#include "hrc_global.h"

OPEN_HRC_NAMESPACE

using namespace hrl::lexer;

class Utilities {
public:
    Utilities();
    ~Utilities();

    static void write_token_list_to_file(FILE *file, const std::vector<GCToken> &tokens);

private:
};

CLOSE_HRC_NAMESPACE

#endif