#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define __WINDOWS__
#define NOMINMAX
#include <Windows.h>
#endif

void setLocale()
{
#ifdef __WINDOWS__
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif
}

#include <istream>
#include <limits>

void skipToEndl(std::istream& in)
{
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}