#ifndef TERMINALCOLOR_H
#define TERMINALCOLOR_H

class TerminalColor {
public:
    TerminalColor();
    ~TerminalColor();

    const char *C_BOLD = "";
    const char *C_WEAK = "";
    const char *C_HIGHLIGHT = "";
    const char *C_UNDERLINE = "";
    const char *C_BLACK = "";
    const char *C_DARK_RED = "";
    const char *C_DARK_GREEN = "";
    const char *C_DARK_YELLOW = "";
    const char *C_DARK_BLUE = "";
    const char *C_DARK_PINK = "";
    const char *C_DARK_CYAN = "";
    const char *C_BLACK_BG = "";
    const char *C_DARK_RED_BG = "";
    const char *C_DARK_GREEN_BG = "";
    const char *C_DARK_YELLOW_BG = "";
    const char *C_DARK_BLUE_BG = "";
    const char *C_DARK_PINK_BG = "";
    const char *C_DARK_CYAN_BG = "";
    const char *C_GRAY = "";
    const char *C_LIGHT_RED = "";
    const char *C_LIGHT_GREEN = "";
    const char *C_LIGHT_YELLOW = "";
    const char *C_LIGHT_BLUE = "";
    const char *C_LIGHT_PINK = "";
    const char *C_LIGHT_CYAN = "";
    const char *C_LIGHT_GRAY = "";
    const char *C_GRAY_BG = "";
    const char *C_LIGHT_RED_BG = "";
    const char *C_LIGHT_GREEN_BG = "";
    const char *C_LIGHT_YELLOW_BG = "";
    const char *C_LIGHT_BLUE_BG = "";
    const char *C_LIGHT_PINK_BG = "";
    const char *C_LIGHT_CYAN_BG = "";
    const char *C_LIGHT_GRAY_BG = "";

    const char *C_RESET = "";

    void reset();

private:
};

extern TerminalColor __tc;

#endif