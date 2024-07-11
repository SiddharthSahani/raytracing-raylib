
#include "logger.h"
#include <cstdarg>
#include <stdio.h>


namespace logger {


LogLevel currentLogLevel = LogLevel::ERROR;


void setLogLevel(LogLevel level) { currentLogLevel = level; }


void trace(const char* format, ...) {
    if (currentLogLevel > LogLevel::TRACE) {
        return;
    }

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}


void info(const char* format, ...) {
    if (currentLogLevel > LogLevel::INFO) {
        return;
    }

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}


void error(const char* format, ...) {
    if (currentLogLevel > LogLevel::ERROR) {
        return;
    }

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}


} // namespace logger
