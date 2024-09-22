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
        C_BOLD = BOLD;
        C_WEAK = WEAK;
        C_HIGHLIGHT = HIGHLIGHT;
        C_UNDERLINE = UNDERLINE;
        C_BLACK = BLACK;
        C_DARK_RED = DARK_RED;
        C_DARK_GREEN = DARK_GREEN;
        C_DARK_YELLOW = DARK_YELLOW;
        C_DARK_BLUE = DARK_BLUE;
        C_DARK_PINK = DARK_PINK;
        C_DARK_CYAN = DARK_CYAN;
        C_BLACK_BG = BLACK_BG;
        C_DARK_RED_BG = DARK_RED_BG;
        C_DARK_GREEN_BG = DARK_GREEN_BG;
        C_DARK_YELLOW_BG = DARK_YELLOW_BG;
        C_DARK_BLUE_BG = DARK_BLUE_BG;
        C_DARK_PINK_BG = DARK_PINK_BG;
        C_DARK_CYAN_BG = DARK_CYAN_BG;
        C_GRAY = GRAY;
        C_LIGHT_RED = LIGHT_RED;
        C_LIGHT_GREEN = LIGHT_GREEN;
        C_LIGHT_YELLOW = LIGHT_YELLOW;
        C_LIGHT_BLUE = LIGHT_BLUE;
        C_LIGHT_PINK = LIGHT_PINK;
        C_LIGHT_CYAN = LIGHT_CYAN;
        C_LIGHT_GRAY = LIGHT_GRAY;
        C_GRAY_BG = GRAY_BG;
        C_LIGHT_RED_BG = LIGHT_RED_BG;
        C_LIGHT_GREEN_BG = LIGHT_GREEN_BG;
        C_LIGHT_YELLOW_BG = LIGHT_YELLOW_BG;
        C_LIGHT_BLUE_BG = LIGHT_BLUE_BG;
        C_LIGHT_PINK_BG = LIGHT_PINK_BG;
        C_LIGHT_CYAN_BG = LIGHT_CYAN_BG;
        C_LIGHT_GRAY_BG = LIGHT_GRAY_BG;
        C_RESET = RESET;
    } else {
        C_BOLD = "";
        C_WEAK = "";
        C_HIGHLIGHT = "";
        C_UNDERLINE = "";
        C_BLACK = "";
        C_DARK_RED = "";
        C_DARK_GREEN = "";
        C_DARK_YELLOW = "";
        C_DARK_BLUE = "";
        C_DARK_PINK = "";
        C_DARK_CYAN = "";
        C_BLACK_BG = "";
        C_DARK_RED_BG = "";
        C_DARK_GREEN_BG = "";
        C_DARK_YELLOW_BG = "";
        C_DARK_BLUE_BG = "";
        C_DARK_PINK_BG = "";
        C_DARK_CYAN_BG = "";
        C_GRAY = "";
        C_LIGHT_RED = "";
        C_LIGHT_GREEN = "";
        C_LIGHT_YELLOW = "";
        C_LIGHT_BLUE = "";
        C_LIGHT_PINK = "";
        C_LIGHT_CYAN = "";
        C_LIGHT_GRAY = "";
        C_GRAY_BG = "";
        C_LIGHT_RED_BG = "";
        C_LIGHT_GREEN_BG = "";
        C_LIGHT_YELLOW_BG = "";
        C_LIGHT_BLUE_BG = "";
        C_LIGHT_PINK_BG = "";
        C_LIGHT_CYAN_BG = "";
        C_LIGHT_GRAY_BG = "";
        C_RESET = "";
    }
}

TerminalColor __tc;