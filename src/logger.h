
#pragma once


#define TRACE(format, ...) logger::trace("LOG [ TRACE ]: " format "\n", ##__VA_ARGS__)
#define INFO(format, ...) logger::info("LOG [ INFO  ]: " format "\n", ##__VA_ARGS__)


namespace logger {


enum class LogLevel {
    TRACE = 0,
    INFO,
};


void setLogLevel(LogLevel level);
void trace(const char* format, ...);
void info(const char* format, ...);


} // namespace logger
