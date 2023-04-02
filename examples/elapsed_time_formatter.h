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

#pragma once

#include <chrono>
#include <ctime>

inline std::string ElapsedTimeFormatter(const DLOGLEVELTOSTRFUNC& in_logLevelToStrFunc, const std::string& in_message, const int in_logLevel) noexcept
{
    std::stringstream ss;
    in_logLevelToStrFunc(ss, in_logLevel);
    ss << " - ";

    static const auto begin = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    ss << std::setfill(' ') << std::setw(8) << std::chrono::duration_cast<std::chrono::milliseconds>(now - begin).count();
    ss << " ms - " << in_message;

    return std::move(ss.str());
}
