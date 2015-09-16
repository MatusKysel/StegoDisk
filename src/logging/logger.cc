#include "logger.h"

#include <ctype.h>
#include <iostream>
#include <fstream>


LoggerVerbosityLevel Logger::verbosity_level_ = kLoggerVerbosityDisabled;
std::ostream *Logger::ofs_ = &std::cout;

bool Logger::ShouldPrint(LoggerVerbosityLevel verbosity_level) {
  if (verbosity_level_ == kLoggerVerbosityDisabled) return false;
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
    ofs_ = new std::ofstream( out, std::ios::out );
    if ( ((std::ofstream*)ofs_)->is_open() ) {
      *ofs_ << "Log started" << std::endl;
    } else
      ofs_ = &std::cout;
  }
}

void Logger::SetVerbosityLevel(char verbosity_level, std::string out) {
  switch(toupper(verbosity_level)) {
    case 'F':
      Logger::SetVerbosityLevel(kLoggerVerbosityFatal);
      break;
    case 'E':
      Logger::SetVerbosityLevel(kLoggerVerbosityError);
      break;
    case 'W':
      Logger::SetVerbosityLevel(kLoggerVerbosityWarning);
      break;
    case 'I':
      Logger::SetVerbosityLevel(kLoggerVerbosityInfo);
      break;
    case 'D':
      Logger::SetVerbosityLevel(kLoggerVerbosityDebug);
      break;
    case 'T':
      Logger::SetVerbosityLevel(kLoggerVerbosityTrace);
      break;
    default:
      Logger::SetVerbosityLevel(kLoggerVerbosityDisabled);
  }

  if ( out.length() != 0 ) {
    ofs_ = new std::ofstream( out, std::ios::out );
    if ( ((std::ofstream*)ofs_)->is_open() ) {
      *ofs_ << "Log started" << std::endl;
    } else {
      ofs_ = &std::cout;
    }
  }
}
