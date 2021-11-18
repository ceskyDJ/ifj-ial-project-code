/*
 * @file logger.h
 * Macros for logging.
 * Simplified version of macro-logger by David Rogrigues.
 * Source: https://github.com/dmcrodrigues/macro-logger
 *
 * IFJ and IAL project (IFJ21 compiler)
 * Team: 128 (variant II)
 *
 * @author Martin Havlík (xhavli56)
 *
 * Copyright (c) 2012 David Rodrigues
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * --- Example usage ---
 *
 * LOG_ERROR("memory allocation failed: %s", strerror(errno));
 *
 * LOG_{DEBUG,INFO,ERROR}_M is for logging messages only, without any additional arguments:
 *
 * LOG_DEBUG_M("cleaning up...");
 *
 * --- To set log level in a file ---
 *
 * #define LOG_LEVEL <level>
 * where <level> can be one of {DEBUG,INFO,ERROR}
 *
 * When no level is set, it becomes DEBUG by default.
 */

#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <stdio.h>

#define ERROR 0x01
#define INFO 0x02
#define DEBUG 0x03

#ifndef LOG_LEVEL
#define LOG_LEVEL DEBUG
#endif

#define PRINT(format, ...) fprintf(stderr, format, __VA_ARGS__)

#define LOG_FMT "%-7s | %-15s | %s:%d | "
#define LOG_ARGS(LOG_TYPE) LOG_TYPE, __FILE__, __func__, __LINE__

#define NEWLINE "\n"

#define ERROR_TYPE "ERROR"
#define INFO_TYPE "INFO"
#define DEBUG_TYPE "DEBUG"


#if LOG_LEVEL >= DEBUG
#define LOG_DEBUG(message, ...) PRINT(LOG_FMT message NEWLINE, LOG_ARGS(DEBUG_TYPE), ## __VA_ARGS__)
#define LOG_DEBUG_M(message) PRINT(LOG_FMT message NEWLINE, LOG_ARGS(DEBUG_TYPE))
#else
#define LOG_DEBUG(message, ...)
#define LOG_DEBUG_M(message)
#endif

#if LOG_LEVEL >= INFO
#define LOG_INFO(message, ...) PRINT(LOG_FMT message NEWLINE, LOG_ARGS(INFO_TYPE), ## __VA_ARGS__)
#define LOG_INFO_M(message) PRINT(LOG_FMT message NEWLINE, LOG_ARGS(INFO_TYPE))
#else
#define LOG_INFO(message, ...)
#define LOG_INFO_M(message)
#endif

#if LOG_LEVEL >= ERROR
#define LOG_ERROR(message, ...) PRINT(LOG_FMT message NEWLINE, LOG_ARGS(ERROR_TYPE), ## __VA_ARGS__)
#define LOG_ERROR_M(message) PRINT(LOG_FMT message NEWLINE, LOG_ARGS(ERROR_TYPE))
#else
#define LOG_ERROR(message, ...)
#define LOG_ERROR_M(message)
#endif

#endif

