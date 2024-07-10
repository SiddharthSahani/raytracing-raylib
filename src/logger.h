
#pragma once

#include <stdio.h>


// trace logging also enables info logging
#if defined(ENABLE_TRACE_LOGGING) && !defined(ENABLE_INFO_LOGGING)
#define ENABLE_INFO_LOGGING
#endif


// info logging also enables error logging
#if defined(ENABLE_INFO_LOGGING) && !defined(ENABLE_ERROR_LOGGING)
#define ENABLE_ERROR_LOGGING
#endif


#ifdef ENABLE_TRACE_LOGGING
#define TRACE(format, ...) printf("LOG[ TRACE ]: " format "\n", ##__VA_ARGS__)
#else
#define TRACE(...)
#endif


#ifdef ENABLE_INFO_LOGGING
#define INFO(format, ...) printf("LOG[ INFO  ]: " format "\n", ##__VA_ARGS__)
#else
#define INFO(...)
#endif


#ifdef ENABLE_ERROR_LOGGING
#define ERROR(format, ...) printf("LOG[ ERROR ]: " format "\n", ##__VA_ARGS__)
#else
#define ERROR(...)
#endif

