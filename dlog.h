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

#include <atomic>
#include <cassert>
#include <exception>
#include <functional>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#define DDEBUG          1000
#define DWARNING        2000
#define DERROR          5000
#define DFATAL          7000
#define DCRITICAL       9000

#ifndef DLOG_CHARTYPE
#define DLOG_CHARTYPE   char
#endif//DLOG_CHARTYPE

#ifndef DLOG_ALLOCATOR
#define DLOG_ALLOCATOR  std::allocator<DLOG_CHARTYPE>
#endif//DLOG_ALLOCATOR

#ifndef NDEBUG
#define DLOG_DISABLED   0
#else
#define DLOG_DISABLED   1
#endif//NDEBUG

namespace dlog
{
using TCHARTYPE          = DLOG_CHARTYPE;
using TSTRING            = std::basic_string      <DLOG_CHARTYPE, std::char_traits<DLOG_CHARTYPE>, DLOG_ALLOCATOR>;
using TSTRINGSTREAM      = std::basic_stringstream<DLOG_CHARTYPE, std::char_traits<DLOG_CHARTYPE>, DLOG_ALLOCATOR>;
using TSTRINGVIEW        = std::basic_string_view <DLOG_CHARTYPE>;
using TLOGLEVELTOSTRFUNC = std::function<void(TSTRINGSTREAM&, const int)>;
using Exception          = std::exception;

template<typename T > 
inline void StringifyBuiltInType(TSTRINGSTREAM& inout_stream, const T in_value) noexcept
{
    if      constexpr (std::is_fundamental<T>()) inout_stream << in_value;
    else if constexpr (std::is_pointer    <T>()) inout_stream << "0x" << std::uppercase << std::setfill('0') << std::setw(sizeof(const void*)) << std::hex << in_value;
    else    ::dlogStringifyCustomType(inout_stream, in_value);
}

inline void StringifyBuiltInType(TSTRINGSTREAM& inout_stream, const bool         in_value) noexcept { inout_stream <<(in_value  ? "true" : "false"); }
inline void StringifyBuiltInType(TSTRINGSTREAM& inout_stream, const TCHARTYPE*   in_value) noexcept { inout_stream << in_value; }
inline void StringifyBuiltInType(TSTRINGSTREAM& inout_stream, const TSTRING&     in_value) noexcept { inout_stream << in_value.c_str(); }
inline void StringifyBuiltInType(TSTRINGSTREAM& inout_stream, const TSTRINGVIEW& in_value) noexcept { inout_stream << in_value; }

namespace impl
{
using TBACKENDFUNC = std::function<void(const TCHARTYPE*)>;
template<typename TBASETYPE, int NDISABLELOGGER = DLOG_DISABLED> struct Frontend;
template<typename TBASETYPE>
struct Frontend<TBASETYPE,0>
{
    const  TCHARTYPE* newLine = "\n";
    struct Stream
    {
        Stream(const int in_logLevel) noexcept : m_logLevel(in_logLevel) { ; }
       ~Stream() noexcept 
        { 
            if (m_baseLogLevel <= m_logLevel)
            {
                Frontend& frontend = *Frontend::GetInstancePtr();
                m_out  << frontend.newLine;
                frontend.Post(m_out.str(), m_logLevel);
            }
        }

        template<typename T> Stream& operator<<(const T in_value) noexcept
        {
            if (m_baseLogLevel <= m_logLevel)
                StringifyBuiltInType(m_out, in_value);
            return  *this;
        }

    private:
        const int m_logLevel = DDEBUG;
        const int m_baseLogLevel = (*Frontend::GetInstancePtr()).logLevel;
        TSTRINGSTREAM m_out;
    };

    std::function<TSTRING(const TLOGLEVELTOSTRFUNC, const TSTRING&, const int)> formatter;
    int logLevel = DDEBUG;

    TLOGLEVELTOSTRFUNC logLevelFormatter = [](TSTRINGSTREAM& inout_stream, const int in_logLevel) noexcept
    {
        switch (in_logLevel)
        {
        case DDEBUG   : inout_stream << "DBG"; break;
        case DWARNING : inout_stream << "WRN"; break;
        case DERROR   : inout_stream << "ERR"; break;
        case DFATAL   : inout_stream << "FAT"; break;
        case DCRITICAL: inout_stream << "CRT"; break;
        }
    };

    Frontend()
    {
        Frontend* instancePtr = nullptr;
        if (!std::atomic_compare_exchange_strong(&GetInstancePtr(), &instancePtr, this))
            throw Exception();
    }

   ~Frontend() { std::atomic_store(&GetInstancePtr(), nullptr); }
   void operator+= (const TBACKENDFUNC& in_backendFunction) noexcept  { m_backends.push_back(in_backendFunction); }

private:
    std::vector<TBACKENDFUNC> m_backends;
    std::mutex m_backendsMutex;

    static std::atomic<Frontend*>& GetInstancePtr() noexcept { static std::atomic<Frontend*> instancePtr; return instancePtr; }
    void Post(const TSTRING& in_message, const int in_logLevel) noexcept
    {
        const TSTRING&& message = formatter ? std::move(formatter(logLevelFormatter, in_message, in_logLevel)) : in_message;
        std::scoped_lock(m_backendMutex);
        for (auto& it  : m_backends)
            it(message.c_str());
    }
};

template<typename TBASETYPE>
struct Frontend<TBASETYPE,1>
{
    int logLevel = DDEBUG;
    std::function<TSTRING(const TLOGLEVELTOSTRFUNC, const TSTRING&, const int)> formatter;
    TLOGLEVELTOSTRFUNC logLevelFormatter;

    struct Stream
    {
        Stream(const int in_logLevel) noexcept { ; }
       ~Stream() = default;
       template<typename T> Stream& operator<<(const T) noexcept { return *this; }
    };

    Frontend() = default;
   ~Frontend() = default;
    void operator+= (const TBACKENDFUNC&) noexcept  { ; }
};
}// impl.
}// dlog.

using DLog = ::dlog::impl::Frontend<dlog::TCHARTYPE>;
#define DLOG(in_level) ::DLog::Stream(in_level)
