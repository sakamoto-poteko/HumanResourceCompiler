#ifndef TERMINALCOLOR_H
#define TERMINALCOLOR_H

class TerminalColor
{
public:
    TerminalColor();
    ~TerminalColor();

    const char *COLOR_HIGHLIGHT = "";
    const char *COLOR_RESET = "";
    const char *COLOR_LIGHT_GREEN = "";
    const char *COLOR_LIGHT_RED = "";

    void reset();
private:
};

extern TerminalColor __tc;

#endif