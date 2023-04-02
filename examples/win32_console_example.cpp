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

#include "dlog_custom.h"
#include "elapsed_time_formatter.h"
#include "simple_formatter.h"

#include <climits>
#include <cstdio>
#include <mutex>
#include <vector>
#include <string>

#ifndef NDEBUG
#define USERDEFINEDLOG(in_logLevel, in_message) DLOG(in_logLevel) << in_message
#else
#define USERDEFINEDLOG(in_logLevel, in_message) do { } while(0)
#endif//NDEBUG

int main(int, char**)
{
    std::mutex defaultBackendMutex;
    DLog logger;
    logger.logLevel = DWARNING;
    logger += [&defaultBackendMutex](const char* in_message, const char* in_categoryName)  
    {
        std::scoped_lock<std::mutex> backendLock(defaultBackendMutex);
        printf("[%s] %s", in_categoryName, in_message); 
    };
    logger.formatter = SimpleFormatter;

    DLOG(DWARNING) << "bool..............: " << (bool              )false     << ", " << (bool              )true       << ".";
    DLOG(DWARNING) << "char..............: " << (char              )'A'       << ", " << (char              )'Z'        << ".";
    DLOG(DWARNING) << "unsigned char.....: " << (unsigned char     )'a'       << ", " << (unsigned char     )'z'        << ".";
    DLOG(DWARNING) << "signed char.......: " << (signed char       )'@'       << ", " << (signed char       )'!'        << ".";
    DLOG(DWARNING) << "unsigned short....: " << (unsigned short    )0         << ", " << (unsigned short    )USHRT_MAX  << ".";
    DLOG(DWARNING) << "short.............: " << (short             )SHRT_MIN  << ", " << (short             )SHRT_MAX   << ".";
    DLOG(DWARNING) << "unsigned int......: " << (unsigned int      )0         << ", " << (unsigned int      )UINT_MAX   << ".";
    DLOG(DWARNING) << "int...............: " << (int               )INT_MIN   << ", " << (int               )INT_MAX    << ".";
    DLOG(DWARNING) << "unsigned long.....: " << (unsigned long     )0         << ", " << (unsigned long     )ULONG_MAX  << ".";
    DLOG(DWARNING) << "long..............: " << (long              )LONG_MIN  << ", " << (long              )LONG_MAX   << ".";
    DLOG(DWARNING) << "unsigned long long: " << (unsigned long long)0         << ", " << (unsigned long long)ULLONG_MAX << ".";
    DLOG(DWARNING) << "long long.........: " << (long long         )LLONG_MIN << ", " << (long long         )LLONG_MAX  << ".";
    DLOG(DWARNING) << "float.............: " << (float             )FLT_MIN   << ", " << (float             )FLT_MAX    << ".";
    DLOG(DWARNING) << "double............: " << (double            )DBL_MIN   << ", " << (double            )DBL_MAX    << ".";
    DLOG(DWARNING) << "pointer...........: " << &logger << ".";

    DLOG(DWARNING) << "const char*.......: " << "Hello, world.";
    DLOG(DWARNING) << "std::string.......: " << std::string("Hello, world.");
    DLOG(DWARNING) << "std::string_view..: " << std::string_view(std::string("Hello, world.").c_str() + sizeof("Hello, ") - 1, sizeof("world.") - 1);

    const std::vector<int> somePrimes  = { 2, 3, 5, 7, 11 };
    DLOG(DWARNING) << "Custom type.......: " << somePrimes << ".";
    DLOG(DINFO   ) << "This message will not be displayed.";
    DLOG(DWARNING, "Foo") << "Logging message to custom category \"Foo\".";
    DLOG(DERROR  ) << "Error-level message.";
    USERDEFINEDLOG(DWARNING, "This message uses a user-defined macro" << ' ' << "that only logs if NDEBUG is undefined.");
    DLOG(DDFATAL ) << "This message (1) should be the last one if NDEBUG is undefined.";
    DLOG(DFATAL  ) << "This message (2) should be the last one.";
    DLOG(DINFO   ) << "This message will not be displayed because the program should have exited...";
}
