# dlog
Simple, easy-to-use logger for your C++ programs.

## Features

* One-header library, works out-of-the-box.
* Support for multi-threaded logging.
* Pretty much standard, no operating system headers dependencies.
* Easily extendable with new message formatters, complex and/or custom data types...

## Adding dlog to your project

Add the following header to every file requiring logging (or to your own header file!):

    #include "dlog.h"

Create the `DLog` object somewhere. This object should be alive during the whole program lifecycle (recommended, not strictly necessary, though). Only one instance should exist.

    DLog logger;

Configure the minimum log level to display:

    // Minimum log level to display, either DDEBUG, DWARNING, DERROR, DFATAL or DCRITICAL. Defaults to DDEBUG.
    logger.logLevel = DDEBUG;

Add your backends of choice:

    // Add a logging backend to print messages to the standard console.
    logger += [](const char* in_message) { printf(in_message); };

And enjoy logging!

    DLOG(DDEBUG) << "Debug message. << ".";
    DLOG(DWARNING) << "Let's write some prime numbers... " << 2 << ', ' << 3 << ', ' << 5 << ', ' << 7 << "..."; 

---
## Customizing dlog

### Configurable options

By default, dlog uses `char` (8-bit) for logging. However, you can define any other character type, like `wchar_t`, by declaring the macro `DLOG_CHARTYPE` with the appropriate type **before including `dlog.h`**.

By default, dlog uses `std::allocator<DLOG_CHARTYPE>` to allocate strings and string streams. However, you can define any other allocator by declaring the macro `DLOG_ALLOCATOR` with the appropriate type **before including `dlog.h`**.

The recommended way if you override either `DLOG_CHARTYPE` or `DLOG_ALLOCATOR` is to provide your own header to be included from your sources, instead of including `dlog.h` directly.

### Writing your own logging backends

You can provide your own logging functions to dlog. If your backend has specific needs to manage resources, it's up to your program to handle that.

You can add your own backend appender functions using the `+=` operator on the `DLog` instance you've created. The function must honor the following signature: `void(const dlog::TCHARTYPE in_message)`, where `in_message` is the message to append.

Backends cannot be removed. Backends are run in registration order. All backends share the minimum log level set.

### Writing your own formatter

You can provide your own formatting function for messages. If your formatter has specific needs to manage resources, it's up to your program to handle that.

To set your own formatting function just do:

    logger.formatter = []
    (
        const dlog::TLOGLEVELTOSTRFUNC& in_logLevelToStrFunc, 
        const dlog::TSTRING& in_message, 
        const int in_logLevel
    ) noexcept
    {
        dlog::TSTRINGSTREAM ss;
        in_logLevelToStrFunc(ss, in_logLevel);
        ss << " - " << in_message;
        return  ss.str();
    }

replacing the function contents by your custom formatting algorithm. The function should return a `std::string` object with the formatted message.

Notice that argument `in_logLevelToStrFunc` represents the function to use in order to convert the log level given in `in_logLevel` to text.

See _Adding custom log levels_ hereby for further information on adding and converting log levels to text.

### Adding custom log levels

Log levels are just plain signed integers. You can declare as many as you like. However, adding custom log levels implies providing your own function to convert them to text.

To set your own log level to text conversion function just do:

    logger.logLevelFormatter = [](dlog::TSTRINGSTREAM& inout_stream, const int in_logLevel) noexcept
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

replacing the function contents by your custom conversion algorithm. The function should write converted data to the `inout_stream` provided as argument. Please notice that if you write your own function, you should take care of the built-in log levels `DDEBUG`, `DWARNING`, `DERROR`, `DFATAL` and `DCRITICAL`.

### Logging other data types

You can provide your own functions to convert complex or custom data types to text so they can be logged. To do so you need to write your own function `dlogStringifyCustomType` for the type of choice, in the global namespace. This function should be either inlined or declared before using the `DLOG` macro. You can provide your own header including `dlog.h` along these function declarations if deemed necessary.

Functions to convert custom types to text can make use of the built-in converters if needed. Take as example this conversion function for `std::vector<T>` (assuming that `T`, in this case, would be a type with an existing built-in or custom converter):

    template<typename T>
    void dlogStringifyCustomType(dlog::TSTRINGSTREAM& inout_stream, const std::vector<T>& in_value) noexcept
    {
        inout_stream << "[ ";
        for (size_t i = 0, l = in_value.size(); i < l;)
        {
            dlog::StringifyBuiltInType(inout_stream, in_value.at(i));
            if ((++i) != l)
                inout_stream << ", ";
        }
        inout_stream << " ]";
    }

---

## Examples

An example solution for Visual Studio 2022 is provided under the folder `vs2022`. Check the following files for more information:
* `simple_formatter.h` and `elapsed_time_formatter.h` to see some examples on custom formatters.
* `std_vector_value_writer.h` to see how to convert custom types to text.

---

*Licensed under the MIT license.*
*This library is under active development and can contain bugs and/or non-portable code. Tested on Microsoft Windows/Visual Studio 2022 only!*
