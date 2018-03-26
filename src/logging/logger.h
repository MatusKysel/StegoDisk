/**
* @file logger.h
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Simple logger class
*
*/

#ifndef STEGODISK_LOGGING_LOGGER_H_
#define STEGODISK_LOGGING_LOGGER_H_

#include <iostream>

#include "stego-disk_export.h"

typedef enum LoggerVerbosityLevel {
  kLoggerVerbosityDisabled = 0,
  kLoggerVerbosityFatal = 1,
  kLoggerVerbosityError = 2,
  kLoggerVerbosityWarning = 3,
  kLoggerVerbosityInfo = 4,
  kLoggerVerbosityDebug = 5,
  kLoggerVerbosityTrace = 6
} LoggerVerbosityLevel;

class STEGO_DISK_EXPORT Logger {
public:
  static void SetVerbosityLevel(LoggerVerbosityLevel verbosity_level,
                                std::string out = "");
  static void SetVerbosityLevel(std::string &verbosity_level, std::string out = "");
  static bool ShouldPrint(LoggerVerbosityLevel verbosity_level);
  static void LoggerClose();

public:
  static LoggerVerbosityLevel verbosity_level_;
  static std::ostream *ofs_;
};

#define LOG_TRACE(expression) if (Logger::ShouldPrint(kLoggerVerbosityTrace)) \
{ *Logger::ofs_ << "TRACE: " << expression << std::endl; }
#define LOG_DEBUG(expression) if (Logger::ShouldPrint(kLoggerVerbosityDebug)) \
{ *Logger::ofs_ << "DEBUG: " << expression << std::endl; }
#define LOG_INFO(expression) if (Logger::ShouldPrint(kLoggerVerbosityInfo)) \
{ *Logger::ofs_ << "INFO: " << expression << std::endl; }
#define LOG_WARN(expression) if (Logger::ShouldPrint(kLoggerVerbosityWarning)) \
{ *Logger::ofs_ << "WARN: " << expression << std::endl; }
#define LOG_ERROR(expression) if (Logger::ShouldPrint(kLoggerVerbosityError)) \
{ *Logger::ofs_ << "ERROR: " << expression << std::endl; }
#define LOG_FATAL(expression) if (Logger::ShouldPrint(kLoggerVerbosityFatal)) \
{ *Logger::ofs_ << "FATAL: " << expression << std::endl; }

#endif // STEGODISK_LOGGING_LOGGER_H_
