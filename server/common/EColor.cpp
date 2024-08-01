//
// Created by 20075 on 2024/7/16.
//

#include "EColor.h"
#include <iostream>

#ifdef __WIN64__
#include <windows.h>
#include <cassert>

void EnableVirtualTerminalProcessing() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Invalid handle");
    }

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) {
        throw std::runtime_error("GetConsoleMode failed");
    }

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, dwMode)) {
        throw std::runtime_error("SetConsoleMode failed");
    }
}
#endif

void initColor(void)
{
#ifdef __WIN64__
    EnableVirtualTerminalProcessing();
#endif
}

std::string green(const std::string &org) {
    return GREEN_START + org + COLOE_RESET;
}

std::string yellow(const std::string &org) {
    return YELLOW_START + org + COLOE_RESET;
}

std::string cyan(const std::string &org) {
    return CYAN_START + org + COLOE_RESET;
}

std::string blueline(const std::string &org)
{
    return BLUELINE_START + org + COLOE_RESET;
}