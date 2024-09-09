#ifndef TERMINALCOLOR_H
#define TERMINALCOLOR_H

class TerminalColor {
public:
    TerminalColor();
    ~TerminalColor();

    const char *C_HIGHLIGHT = "";
    const char *C_RESET = "";
    const char *C_LIGHT_GREEN = "";
    const char *C_LIGHT_RED = "";
    const char *C_DARK_RED = "";
    const char *C_DARK_YELLOW = "";
    const char *C_DARK_BLUE = "";

    void reset();

private:
};

extern TerminalColor __tc;

#endif