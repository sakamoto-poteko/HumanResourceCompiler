#include <cstdio>

#include "TerminalColor.h"
#include "ansi_colors.h"
#include <unistd.h>

TerminalColor::TerminalColor()
{
}

TerminalColor::~TerminalColor()
{
}

void TerminalColor::reset()
{
    bool is_terminal = isatty(fileno(stdout));
    if (is_terminal) {
        COLOR_HIGHLIGHT = HIGHLIGHT;
        COLOR_RESET = RESET;
        COLOR_LIGHT_GREEN = LIGHT_GREEN;
    }
}

TerminalColor __tc;