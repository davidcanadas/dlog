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

#include <cstdlib>
#include <functional>
#include <iomanip>
#include <sstream>
#include <type_traits>

static constexpr int DINFO    = 1000;
static constexpr int DWARNING = 3000;
static constexpr int DERROR   = 5000;
static constexpr int DDFATAL  = 7000;
static constexpr int DFATAL   = 9000;

struct dlogAllocatorType;
struct dlogDisableLogger;

namespace dlog
{
using Exception = std::exception;

template<typename, typename = void> constexpr bool is_type_complete_v = false;
template<typename T> constexpr bool is_type_complete_v<T, std::void_t<decltype(sizeof(T))>> = true;

template<typename TCONFIGTYPE, typename TDEFAULTIMPLICITTYPE, int NISCONFIGTYPECOMPLETE> struct configured_type;
template<typename TCONFIGTYPE, typename TDEFAULTIMPLICITTYPE> struct configured_type<TCONFIGTYPE, TDEFAULTIMPLICITTYPE, 0> { using type = TDEFAULTIMPLICITTYPE; };
template<typename TCONFIGTYPE, typename TDEFAULTIMPLICITTYPE> struct configured_type<TCONFIGTYPE, TDEFAULTIMPLICITTYPE, 1> { using type = typename TCONFIGTYPE::type; };
template<typename TCONFIGTYPE, typename TDEFAULTIMPLICITTYPE, int NISCONFIGTYPECOMPLETE> using  configured_type_t = typename configured_type<TCONFIGTYPE, TDEFAULTIMPLICITTYPE, NISCONFIGTYPECOMPLETE>::type;

#if defined(USE_WIDE_CHAR) || defined(UNICODE) || defined(_UNICODE)
using TCHARTYPE     = wchar_t;
#define DSTRING(in_string)  L##in_string
#else
using TCHARTYPE     = char;
#define DSTRING(in_string)  in_string
#endif//defined(USE_WIDE_CHAR) || defined(UNICODE) || defined(_UNICODE)

using TSTRING       = std::basic_string      <TCHARTYPE, std::char_traits<TCHARTYPE>, configured_type_t<dlogAllocatorType, std::allocator<TCHARTYPE>, is_type_complete_v<dlogAllocatorType>>>;
using TSTRINGSTREAM = std::basic_stringstream<TCHARTYPE, std::char_traits<TCHARTYPE>, configured_type_t<dlogAllocatorType, std::allocator<TCHARTYPE>, is_type_complete_v<dlogAllocatorType>>>;
using TSTRINGVIEW   = std::basic_string_view <TCHARTYPE>;

#ifndef NDEBUG
static constexpr bool is_debug_target_v = true ;
#else
static constexpr bool is_debug_target_v = false;
#endif//NDEBUG

template<typename = void, typename... TARGS> struct has_customtype_stringifier : std::false_type { };
template<typename... TARGS> struct has_customtype_stringifier<std::void_t<decltype(::dlogStringifyCustomType(std::declval<TARGS>()...))>, TARGS...> : std::true_type { };
template<typename... TARGS> inline constexpr bool has_customtype_stringifier_v = has_customtype_stringifier<void, TARGS...>::value;

template<typename T>
void InvokeCustomTypeStringifier(TSTRINGSTREAM& inout_stream, const T in_value) 
{
    static_assert(has_customtype_stringifier_v<TSTRINGSTREAM&, const T&>, "Missing dlogStringifyCustomType implementation for the given type. Check compiler error(s) for details.");
    if constexpr (has_customtype_stringifier_v<TSTRINGSTREAM&, const T&>)
        ::dlogStringifyCustomType(inout_stream,  in_value);
}
}// dlog.

using DLOGLEVELTOSTRFUNC = std::function<void(dlog::TSTRINGSTREAM&, const int)>;

template<typename T, typename TRETURNTYPE = std::enable_if_t< std::is_fundamental_v<std::remove_reference_t<std::remove_cv_t<T>>> ||  std::is_pointer_v<T> ||  std::is_array_v<T>, void>> 
inline TRETURNTYPE dlogStringifyBuiltInType(dlog::TSTRINGSTREAM& inout_stream, const T in_value) noexcept
{
    using   TBARETYPE = std::remove_reference_t<std::remove_cv_t<T>>;
    if      constexpr (std::is_same_v<TBARETYPE, bool>) inout_stream << (in_value ? DSTRING("true") : DSTRING("false"));
    else if constexpr (std::is_same_v<TBARETYPE, std::remove_cv_t<const dlog::TCHARTYPE* const>>) inout_stream << in_value;
    else if constexpr (std::is_fundamental_v<TBARETYPE>) inout_stream << in_value;
    else if constexpr (std::is_pointer_v    <TBARETYPE>) inout_stream << DSTRING("0x") << std::uppercase << std::setfill(DSTRING('0')) << std::setw(sizeof(const void*)) << std::hex << in_value;
    else dlog::InvokeCustomTypeStringifier(inout_stream, in_value);
}

template<typename T, typename TRETURNTYPE = std::enable_if_t<!std::is_fundamental_v<std::remove_reference_t<std::remove_cv_t<T>>> && !std::is_pointer_v<T> && !std::is_array_v<T>>> 
inline void dlogStringifyBuiltInType(dlog::TSTRINGSTREAM& inout_stream, const T& in_value) noexcept
{
    using   TBARETYPE = std::remove_reference_t<std::remove_cv_t<T>>;
    if      constexpr (std::is_same_v<TBARETYPE, std::remove_cv_t<const dlog::TSTRING    >>) inout_stream << in_value;
    else if constexpr (std::is_same_v<TBARETYPE, std::remove_cv_t<const dlog::TSTRINGVIEW>>) inout_stream << in_value;
    else dlog::InvokeCustomTypeStringifier(inout_stream, in_value);
}

namespace dlog
{
struct Frontend final
{
    using TBACKENDFUNC = std::function<void(const TCHARTYPE*, const TCHARTYPE*)>;
    const TCHARTYPE* newLine = DSTRING("\n");

    template<int NLOGLEVEL>
    struct Stream final
    {
        static constexpr bool k_streamEnabled = !(is_type_complete_v<dlogDisableLogger> || (!is_debug_target_v && (NLOGLEVEL == DDFATAL)));
        Stream(const TCHARTYPE* in_categoryName = DSTRING("default")) noexcept
            : m_baseLogLevel (k_streamEnabled ? (*Frontend::GetInstancePtr()).logLevel : 0)
            , m_categoryName (in_categoryName) 
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

        template<typename T, typename TRETURNTYPE = std::enable_if_t< std::is_fundamental_v<std::remove_reference_t<std::remove_cv_t<T>>> ||  std::is_pointer_v<T> ||  std::is_array_v<T>, Stream>> 
        TRETURNTYPE& operator<<(const T  in_value) noexcept
        {
            if constexpr (k_streamEnabled)
                if (m_baseLogLevel <= NLOGLEVEL)
                    ::dlogStringifyBuiltInType(m_out, in_value);
            return  *this;
        }

        template<typename T, typename TRETURNTYPE = std::enable_if_t<!std::is_fundamental_v<std::remove_reference_t<std::remove_cv_t<T>>> && !std::is_pointer_v<T> && !std::is_array_v<T>>> 
        Stream& operator<<(const T& in_value) noexcept
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
    std::function<const TSTRING(const DLOGLEVELTOSTRFUNC, const TSTRING&, const int)> formatter;
    DLOGLEVELTOSTRFUNC logLevelFormatter = [](TSTRINGSTREAM& inout_stream, const int in_logLevel) noexcept
    {
        switch (in_logLevel)
        {
        case DINFO    : inout_stream << DSTRING("INF"); break;
        case DWARNING : inout_stream << DSTRING("WRN"); break;
        case DERROR   : inout_stream << DSTRING("ERR"); break;
        case DDFATAL  : inout_stream << DSTRING("DBG"); break;
        case DFATAL   : inout_stream << DSTRING("FTL"); break;
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
    static std::atomic<Frontend*>& GetInstancePtr() noexcept { static std::atomic<Frontend*> instancePtr; return instancePtr; }
    void Post(const TSTRING& in_message, const int in_logLevel, const TCHARTYPE* in_optCategoryName) noexcept
    {
        if (formatter)
        {
            const TSTRING&& message = formatter(logLevelFormatter, in_message, in_logLevel);
            for (auto& it  : m_backends)
                it(message.c_str(), in_optCategoryName);
        }
        else
            for (auto& it  : m_backends)
                it(in_message.c_str(), in_optCategoryName);
    }
};
}// dlog.

using DLog = ::dlog::Frontend;
#define DLOG(in_logLevel, ...) ::DLog::Stream<in_logLevel>(__VA_ARGS__)
