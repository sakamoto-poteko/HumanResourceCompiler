#include <cstdio>

#include "TerminalColor.h"
#include "ansi_colors.h"

#ifndef _WINDOWS
#include <unistd.h>
#endif

TerminalColor::TerminalColor()
{
}

TerminalColor::~TerminalColor()
{
}

void TerminalColor::reset()
{
#ifdef _WINDOWS
    bool is_terminal = false;
#else
    bool is_terminal = isatty(fileno(stdout));
#endif
    if (is_terminal) {
        C_HIGHLIGHT = HIGHLIGHT;
        C_RESET = RESET;
        C_LIGHT_GREEN = LIGHT_GREEN;
        C_LIGHT_RED = LIGHT_RED;
        C_DARK_BLUE = DARK_BLUE;
        C_DARK_RED = DARK_RED;
        C_DARK_YELLOW = DARK_YELLOW;
    }
}

TerminalColor __tc;