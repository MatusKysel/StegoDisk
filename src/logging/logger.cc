/**
* @file logger.cc
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Simple logger class implementation
*
*/

#include "logger.h"

#include <ctype.h>
#include <iostream>
#include <algorithm>
#include <fstream>

#include "api_mask.h"

LoggerVerbosityLevel Logger::verbosity_level_ = LoggerVerbosityLevel::kLoggerVerbosityDisabled;
std::ostream *Logger::ofs_ = &std::cout;

bool Logger::ShouldPrint(LoggerVerbosityLevel verbosity_level) {
  if (verbosity_level_ == LoggerVerbosityLevel::kLoggerVerbosityDisabled) return false;
  if (verbosity_level <= verbosity_level_) return true;
  return false;
}

void Logger::LoggerClose() {
  if ( ofs_ != &std::cout ) {
    ((std::ofstream*)ofs_)->close();
  }
}

void Logger::SetVerbosityLevel(LoggerVerbosityLevel verbosity_level, std::string out) {
  verbosity_level_ = verbosity_level;
  if ( out.length() != 0 ) {

    if (out == "stdout" || out == "cout") ofs_ = &std::cout;
    else ofs_ = new std::ofstream( out, std::ios::out );

    if ( ((std::ofstream*)ofs_)->is_open() ) {
      *ofs_ << "Log started" << std::endl;
    } else
      ofs_ = &std::cout;
  }
}

void Logger::SetVerbosityLevel(std::string &verbosity_level, std::string out) {
  std::transform(verbosity_level.begin(), verbosity_level.end(),
                 verbosity_level.begin(), ::toupper);

  if (verbosity_level == "FATAL") {
    Logger::SetVerbosityLevel(LoggerVerbosityLevel::kLoggerVerbosityFatal);
  } else if (verbosity_level == "ERROR" ) {
    Logger::SetVerbosityLevel(LoggerVerbosityLevel::kLoggerVerbosityError);
  } else if (verbosity_level == "WARN" ) {
    Logger::SetVerbosityLevel(LoggerVerbosityLevel::kLoggerVerbosityWarning);
  } else if (verbosity_level == "INFO" ) {
    Logger::SetVerbosityLevel(LoggerVerbosityLevel::kLoggerVerbosityInfo);
  } else if (verbosity_level == "DEBUG" ) {
    Logger::SetVerbosityLevel(LoggerVerbosityLevel::kLoggerVerbosityDebug);
  } else if (verbosity_level == "TRACE" ) {
    Logger::SetVerbosityLevel(LoggerVerbosityLevel::kLoggerVerbosityTrace);
  } else {
    Logger::SetVerbosityLevel(LoggerVerbosityLevel::kLoggerVerbosityDisabled);
  }

  if ( out.length() != 0 ) {

    if (out == "stdout" || out == "cout") ofs_ = &std::cout;
    else ofs_ = new std::ofstream( out, std::ios::out );

    if ( ((std::ofstream*)ofs_)->is_open() ) {
      *ofs_ << "Log started" << std::endl;
    } else {
      ofs_ = &std::cout;
    }
  }
}
