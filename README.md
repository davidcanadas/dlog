# dlog
The fast-path to get logging support in your C++ program.

## Features

* One-header, works out-of-the-box.
* Multi-thread-ready.
* No operating system headers needed.
* Extendable with custom formatters, types and log levels.

## Using **dlog** in your programs

```c++
#include "dlog.h"

int main(int, char**)
{
    DLog logger; // The persistent DLog object (must live as long as your program do!).
    logger.logLevel = DINFO; // Configure the min. level to display, either DINFO, DWARNING, DERROR, DDFATAL or DFATAL.

    // Add a backend to send data to the standard output.
    logger += [](const char* in_message, const char* in_categoryName) { printf("[%s] %s", in_categoryName, in_message); };

    // We're done... Happy logging!
    DLOG(DDEBUG) << "Example message.";
    DLOG(DWARNING) << "Let's write some prime numbers... " << 2 << ', ' << 3 << ', ' << 5 << ', ' << 7 << "..."; 
}
```
### Built-in log levels

| Level | Description |
| --- | --- |
| `DINFO` | Informational messages. |
| `DWARNING` | Warnings. |
| `DERROR` | Recoverable errors. |
| `DDFATAL` | Only if `NDEBUG` is undefined. Fatal errors. Program will exit automatically after using this log level. |
| `DFATAL` | Fatal errors. Program will exit automatically after using this log level. |

### Built-in data types support

The following data types are supported out of the box: `bool`, `char` (or `wchar_t`), `unsigned char` (or `unsigned wchar_t`), `signed char` (or `signed wchar_t`), `unsigned short`, `short`, `unsigned int`, `int`, `unsigned long`, `long`, `unsigned long long`, `float`, `double`, `pointer`, `const char*`, `std::string` and `std::string_view`.

## Customizing **dlog**

```c++
// Character width (char vs wchar_t) depends on either UNICODE, _UNICODE or USE_WIDE_CHAR macros.
struct dlogAllocatorType { using type = std::allocator<char>; }; // Define with the allocator of choice if you don't want to use std::allocator<char>.
struct dlogDisableLogger { }; // Define in order to disable logging.

// Definitions should come BEFORE including dlog.h.
// Recommended way is to create your own header file that configures the logger and then includes dlog.h.

#include "dlog.h"

#include <sstream>
#include <string>

// Define as many dlogStringifyCustomType functions you'd like to add logging support for custom data types.
// When logging a container, you can call dlogStringifyBuiltInType to try run the stringifier for any contained type, if exists.
// Symbol should be available to *dlog*.
template<typename T>
void dlogStringifyCustomType(std::stringstream& inout_stream, const std::vector<T>& in_value) noexcept
{
    inout_stream << '[';
    for (size_t i = 0, l = in_value.size(); i < l;)
    {
        ::dlogStringifyBuiltInType(inout_stream, in_value.at(i));
        if ((++i) != l)
            inout_stream << ',';
    }
    inout_stream << ']';
}

int main(int, char**)
{
    DLog logger;
    std::mutex defaultBackendMutex; // Mutex to get multi-threaded backend support. 

    // You can add your own backends to log your messages to. The character type to use depends on your character type of choice.
    // Backends cannot be removed.
    // Only messages honoring the minimum log level are displayed.
    // However, you can run your own checks in your backend function to finetune each backend individually.
    logger += [](const char* in_message, const char* in_categoryName)  
    {
        std::scoped_lock(defaultBackendMutex); // Multi-threading must be handled by the backend function.
        printf("[%s] %s", in_categoryName, in_message); 
    };

    // You can override the default *log level to text* conversion function.
    // The function must give support to the built-in log levels. You can extend that with custom log levels, though!
    logger.logLevelFormatter = [](std::stringstream& inout_stream, const int in_logLevel) noexcept
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

    // You can override the default formatter with your custom formatting function.
    // The string and stream types depend on your character type of choice.
    logger.formatter = [](const DLOGLEVELTOSTRFUNC& in_logLevelToStrFunc, const std::string& in_message, const int in_logLevel) noexcept
    {
        std::stringstream ss;
        in_logLevelToStrFunc(ss, in_logLevel); // Run the log level to text conversion function.
        ss << " - " << in_message;
        return  ss.str();
    }
}
```

### Avoiding side-effects

Calling functions that cause side effects in your program while logging are generally a bad practice. Given the way **dlog** works, such functions would be called even when logging has been disabled. You can overcome this by providing your own logging macro that receives the message to log as argument:

```c++
#include "dlog.h"

#ifndef NDEBUG
#define USERDEFINEDLOG(in_logLevel, in_message) DLOG(in_logLevel) << in_message
#else
#define USERDEFINEDLOG(in_logLevel, in_message) do { } while(0)
#endif//NDEBUG

int main(int, char**)
{
    DLog logger;
    std::mutex defaultBackendMutex; // Mutex to get multi-threaded backend support. 

    // You can add your own backends to log your messages to. The character type to use depends on your character type of choice.
    // Backends cannot be removed.
    // Only messages honoring the minimum log level are displayed.
    // However, you can run your own checks in your backend function to finetune each backend individually.
    logger += [](const char* in_message, const char* in_categoryName)  
    {
        std::scoped_lock(defaultBackendMutex); // Multi-threading must be handled by the backend function.
        printf("[%s] %s", in_categoryName, in_message); 
    };
    
    DLOG(DINFO) << "This message will be logged always" << ".";
    USERDEFINEDLOG(DINFO, "This message will be logged only if NDEBUG is undefined" << ".");
}
```

## Examples

An example solution for Visual Studio 2022 is provided under the folder `vs2022`. Check the following files for more information:
* `examples/simple_formatter.h` and `examples/elapsed_time_formatter.h` to see some examples on custom formatters.
* `examples/std_vector_value_writer.h` to see how to convert custom types to text.
* `examples/dlog_custom.h` to see a custom header file that provides custom converters to text.

---

*Licensed under the MIT license.*
*This library is under active development and can contain bugs and/or non-portable code. Tested on Microsoft Windows/Visual Studio 2022 (VC++ & Clang) only!*
