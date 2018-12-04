/**
* @file logger.h
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Simple logger class
*
*/

#pragma once

#include <iostream>

#include "api_mask.h"
#include "stego-disk_export.h"

//typedef enum LoggerVerbosityLevel {
//  kLoggerVerbosityDisabled = 0,
//  kLoggerVerbosityFatal = 1,
//  kLoggerVerbosityError = 2,
//  kLoggerVerbosityWarning = 3,
//  kLoggerVerbosityInfo = 4,
//  kLoggerVerbosityDebug = 5,
//  kLoggerVerbosityTrace = 6
//} LoggerVerbosityLevel;

enum class LoggerVerbosityLevel : unsigned 
{
	kLoggerVerbosityDisabled = 0,
	kLoggerVerbosityFatal = 1,
	kLoggerVerbosityError = 2,
	kLoggerVerbosityWarning = 3,
	kLoggerVerbosityInfo = 4,
	kLoggerVerbosityDebug = 5,
	kLoggerVerbosityTrace = 6
};

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

#define LOG_TRACE(expression) if (Logger::ShouldPrint(LoggerVerbosityLevel::kLoggerVerbosityTrace)) \
{ *Logger::ofs_ << "TRACE: " << expression << std::endl; }
#define LOG_DEBUG(expression) if (Logger::ShouldPrint(LoggerVerbosityLevel::kLoggerVerbosityDebug)) \
{ *Logger::ofs_ << "DEBUG: " << expression << std::endl; }
#define LOG_INFO(expression) if (Logger::ShouldPrint(LoggerVerbosityLevel::kLoggerVerbosityInfo)) \
{ *Logger::ofs_ << "INFO: " << expression << std::endl; }
#define LOG_WARN(expression) if (Logger::ShouldPrint(LoggerVerbosityLevel::kLoggerVerbosityWarning)) \
{ *Logger::ofs_ << "WARN: " << expression << std::endl; }
#define LOG_ERROR(expression) if (Logger::ShouldPrint(LoggerVerbosityLevel::kLoggerVerbosityError)) \
{ *Logger::ofs_ << "ERROR: " << expression << std::endl; }
#define LOG_FATAL(expression) if (Logger::ShouldPrint(LoggerVerbosityLevel::kLoggerVerbosityFatal)) \
{ *Logger::ofs_ << "FATAL: " << expression << std::endl; }

