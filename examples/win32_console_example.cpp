/*
 * MIT License
 * 
 * Copyright (c) 2023 David Cañadas Mazo.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
 */

#include "../dlog.h"
#include "elapsed_time_formatter.h"
#include "simple_formatter.h"
#include "std_vector_value_writer.h"

#include <cstdio>
#include <vector>
#include <string>

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>

extern void OtherTranslationUnitTest();

int main(int, char**)
{
    const std::vector<int> somePrimes = { 2, 3, 5, 7, 11 };
    
    DLog logger;
    logger.logLevel = DWARNING;
    logger += [](const char* in_message) { OutputDebugStringA(in_message); };
    logger += [](const char* in_message) { printf(in_message); };
    logger.formatter = SimpleFormatter;

    DLOG(DWARNING) << "Custom type is " << typeid(somePrimes).name() << ".";
    DLOG(DWARNING) << "Memory address " << &somePrimes << ".";
    DLOG(DWARNING) << somePrimes.size() << " elements: " << somePrimes << ".";
    DLOG(DWARNING) << "================================================================";
    OtherTranslationUnitTest();

    DLOG(DDEBUG)   << "This message will not be displayed.";
    
    DLOG(DWARNING) << "================================================================";
    DLOG(DWARNING) << "bool               = " << false << ", " << true << ".";
    DLOG(DWARNING) << "char               = " << char          ('A') << ", " << char          ('B') << ".";
    DLOG(DWARNING) << "unsigned char      = " << unsigned char ('C') << ", " << unsigned char ('D') << ".";
    DLOG(DWARNING) << "signed char        = " << signed   char ('E') << ", " << signed   char ('F') << ".";
    DLOG(DWARNING) << "unsigned short     = " << unsigned short( 0 ) << ", " << unsigned short( 1 ) << ".";
    DLOG(DWARNING) << "short              = " << short         ( 2 ) << ", " << short         (-3 ) << ".";
    DLOG(DWARNING) << "unsigned int       = " << 4u    << ", " <<  5u    << ".";
    DLOG(DWARNING) << "int                = " << 6     << ", " << -7     << ".";
    DLOG(DWARNING) << "unsigned long      = " << 8lu   << ", " <<  9lu   << ".";
    DLOG(DWARNING) << "long               = " << 10l   << ", " << -11l   << ".";
    DLOG(DWARNING) << "unsigned long long = " << 12llu << ", " <<  13llu << ".";
    DLOG(DWARNING) << "long long          = " << 14ll  << ", " << -15ll  << ".";
    DLOG(DWARNING) << "float              = " << float (16.123) << ", " << float (-17.456) << ".";
    DLOG(DWARNING) << "double             = " << double(18.789) << ", " << double(-19.000) << ".";

    DLOG(DWARNING) << "================================================================";
    std::string testString = "HELLO, WORLD";
    std::string_view testStringView(testString.c_str() + sizeof("HELLO, ") - 1, sizeof("WORLD") - 1);
    DLOG(DWARNING) << "std::string        = " << testString     << ".";
    DLOG(DWARNING) << "std::string_view   = " << testStringView << ".";
}
