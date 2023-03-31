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

#include <functional>
#include <iomanip>
#include <sstream>

#include <stdlib.h>

static constexpr int DINFO    = 1000;
static constexpr int DWARNING = 3000;
static constexpr int DERROR   = 5000;
static constexpr int DDFATAL  = 7000;
static constexpr int DFATAL   = 9000;

struct dlogCharType;
struct dlogAllocatorType;
struct dlogDisableLogger;

using  dlogException = std::exception;

namespace dlog
{
template<typename, typename = void> constexpr bool is_type_complete_v = false;
template<typename T>                constexpr bool is_type_complete_v<T, std::void_t<decltype(sizeof(T))>> = true;
template<typename TCONFIGTYPE, typename TDEFAULTIMPLICITTYPE, int NISCONFIGTYPECOMPLETE> struct TCONFIGIMPLICITTYPE;
template<typename TCONFIGTYPE, typename TDEFAULTIMPLICITTYPE> struct TCONFIGIMPLICITTYPE<TCONFIGTYPE, TDEFAULTIMPLICITTYPE, 0> { using type = TDEFAULTIMPLICITTYPE; };
template<typename TCONFIGTYPE, typename TDEFAULTIMPLICITTYPE> struct TCONFIGIMPLICITTYPE<TCONFIGTYPE, TDEFAULTIMPLICITTYPE, 1> { using type = typename TCONFIGTYPE::type; };

using TCHARTYPE     = TCONFIGIMPLICITTYPE<dlogCharType, char, is_type_complete_v<dlogCharType>>::type;
using TSTRING       = std::basic_string      <TCHARTYPE, std::char_traits<TCHARTYPE>, TCONFIGIMPLICITTYPE<dlogAllocatorType, std::allocator<TCHARTYPE>, is_type_complete_v<dlogAllocatorType>>::type>;
using TSTRINGSTREAM = std::basic_stringstream<TCHARTYPE, std::char_traits<TCHARTYPE>, TCONFIGIMPLICITTYPE<dlogAllocatorType, std::allocator<TCHARTYPE>, is_type_complete_v<dlogAllocatorType>>::type>;
using TSTRINGVIEW   = std::basic_string_view <TCHARTYPE>;

#ifndef NDEBUG
static constexpr bool k_isDebugTarget = true ;
#else
static constexpr bool k_isDebugTarget = false;
#endif//NDEBUG

template<typename TBASETYPE> struct StringConstantsImpl;
template<> struct StringConstantsImpl<char    > final { static constexpr const char*    s_false = "false", *s_true = "true", *s_hexPrefix = "0x", s_hexZeroCh = '0', *s_default = "default", *s_info = "INF", *s_warning = "WRN", *s_error = "ERR", *s_dfatal = "DBG", *s_fatal = "FTL"; };
template<> struct StringConstantsImpl<wchar_t > final { static constexpr const wchar_t* s_false =L"false", *s_true =L"true", *s_hexPrefix =L"0x", s_hexZeroCh =L'0', *s_default =L"default", *s_info =L"INF", *s_warning =L"WRN", *s_error =L"ERR", *s_dfatal =L"DBG", *s_fatal =L"FTL"; };
using StringConstants = StringConstantsImpl<TCHARTYPE>;
}// dlog.

using DLOGLEVELTOSTRFUNC = std::function<void(dlog::TSTRINGSTREAM&, const int)>;

template<typename T> 
inline void dlogStringifyBuiltInType(dlog::TSTRINGSTREAM& inout_stream, const T in_value) noexcept
{
    using TBARETYPE = typename std::remove_reference<std::remove_cv<T>::type>::type;

    if      constexpr (std::is_same<TBARETYPE, bool>()) inout_stream << (in_value ? dlog::StringConstants::s_true : dlog::StringConstants::s_false);
    else if constexpr (std::is_same<TBARETYPE, std::remove_cv<const dlog::TCHARTYPE* const>::type>()) inout_stream << in_value;
    else if constexpr (std::is_same<TBARETYPE, std::remove_cv<const dlog::TSTRING         >::type>()) inout_stream << in_value;
    else if constexpr (std::is_same<TBARETYPE, std::remove_cv<const dlog::TSTRINGVIEW     >::type>()) inout_stream << in_value;
    else if constexpr (std::is_fundamental<TBARETYPE>()) inout_stream << in_value;
    else if constexpr (std::is_pointer    <TBARETYPE>()) inout_stream << dlog::StringConstants::s_hexPrefix << std::uppercase << std::setfill(dlog::StringConstants::s_hexZeroCh) << std::setw(sizeof(const void*)) << std::hex << in_value;
    else ::dlogStringifyCustomType(inout_stream, in_value);
}

namespace dlog
{
struct Frontend final
{
    using TBACKENDFUNC = std::function<void(const TCHARTYPE*, const TCHARTYPE*)>;
    const TCHARTYPE* newLine = "\n";

    template<int NLOGLEVEL>
    struct Stream final
    {
        static constexpr bool k_streamEnabled = !(::dlog::is_type_complete_v<dlogDisableLogger> || (!k_isDebugTarget && (NLOGLEVEL == DDFATAL)));
        Stream(const TCHARTYPE* in_categoryName = StringConstants::s_default) noexcept
            : m_baseLogLevel(k_streamEnabled ? (*Frontend::GetInstancePtr()).logLevel : 0)
            , m_categoryName(in_categoryName) 
        { ; }

       ~Stream() noexcept 
        { 
            if constexpr (k_streamEnabled)
            {
                if (m_baseLogLevel <= NLOGLEVEL)
                {
                    Frontend& frontend = *Frontend::GetInstancePtr();
                    m_out  << frontend.newLine;
                    frontend.Post(m_out.str(), NLOGLEVEL, m_categoryName);
                    if constexpr (NLOGLEVEL >= DDFATAL)
                        exit(EXIT_FAILURE);
                }
            }
        }

        template<typename T> Stream& operator<<(const T in_value) noexcept
        {
            if constexpr (k_streamEnabled)
                if (m_baseLogLevel <= NLOGLEVEL)
                    ::dlogStringifyBuiltInType(m_out, in_value);
            return  *this;
        }

    private:
        const int m_baseLogLevel;
        const TCHARTYPE* m_categoryName;
        TSTRINGSTREAM m_out;
    };

    int logLevel = DINFO;
    std::function<TSTRING(const DLOGLEVELTOSTRFUNC, const TSTRING&, const int)> formatter;
    DLOGLEVELTOSTRFUNC logLevelFormatter = [](TSTRINGSTREAM& inout_stream, const int in_logLevel) noexcept
    {
        switch (in_logLevel)
        {
        case DINFO    : inout_stream << StringConstants::s_info;    break;
        case DWARNING : inout_stream << StringConstants::s_warning; break;
        case DERROR   : inout_stream << StringConstants::s_error;   break;
        case DDFATAL  : inout_stream << StringConstants::s_dfatal;  break;
        case DFATAL   : inout_stream << StringConstants::s_fatal;   break;
        }
    };

    Frontend()
    {
        Frontend* instancePtr = nullptr;
        if (!std::atomic_compare_exchange_strong(&GetInstancePtr(), &instancePtr, this))
            throw dlogException();
    }

   ~Frontend() { std::atomic_store(&GetInstancePtr(), nullptr); }
   void operator+= (const TBACKENDFUNC& in_backendFunction) noexcept  { m_backends.push_back(in_backendFunction); }

private:
    std::vector<TBACKENDFUNC> m_backends;
    static std::atomic<Frontend*>& GetInstancePtr() noexcept { static std::atomic<Frontend*> instancePtr; return instancePtr; }
    void Post(const TSTRING& in_message, const int in_logLevel, const TCHARTYPE* in_optCategoryName) noexcept
    {
        const TSTRING&& message = formatter ? std::move(formatter(logLevelFormatter, in_message, in_logLevel)) : in_message;
        for (auto& it  : m_backends)
            it(message.c_str(), in_optCategoryName);
    }
};
}// dlog.

using DLog = ::dlog::Frontend;
#define DLOG(in_logLevel, ...) ::DLog::Stream<in_logLevel>(__VA_ARGS__)
