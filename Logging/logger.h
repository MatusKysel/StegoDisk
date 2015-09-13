#ifndef __StegoDisk__Logger__
#define __StegoDisk__Logger__

#include <iostream>

typedef enum e_LoggerVerbosityLevel {
    kLoggerVerbosityDisabled = 0,
    kLoggerVerbosityFatal = 1,
    kLoggerVerbosityError = 2,
    kLoggerVerbosityWarning = 3,
    kLoggerVerbosityInfo = 4,
    kLoggerVerbosityDebug = 5,
    kLoggerVerbosityTrace = 6
} LoggerVerbosityLevel;

class Logger
{
public:
    static void setVerbosityLevel(LoggerVerbosityLevel verbosityLevel, std::string out = "");
    static void setVerbosityLevel(char verbosityLevel, std::string out = "");
    static bool shouldPrint(LoggerVerbosityLevel verbosityLevel);
	static void loggerClose();

public:
    static LoggerVerbosityLevel verbosityLevel;
	static std::ostream *ofs;
};

#define LOG_TRACE(expression) if (Logger::shouldPrint(kLoggerVerbosityTrace)) { *Logger::ofs << "TRACE: " << expression << std::endl; }
#define LOG_DEBUG(expression) if (Logger::shouldPrint(kLoggerVerbosityDebug)) { *Logger::ofs << "DEBUG: " << expression << std::endl; }
#define LOG_INFO(expression) if (Logger::shouldPrint(kLoggerVerbosityInfo)) { *Logger::ofs << "INFO: " << expression << std::endl; }
#define LOG_WARN(expression) if (Logger::shouldPrint(kLoggerVerbosityWarning)) { *Logger::ofs << "WARN: " << expression << std::endl; }
#define LOG_ERROR(expression) if (Logger::shouldPrint(kLoggerVerbosityError)) { *Logger::ofs << "ERROR: " << expression << std::endl; }
#define LOG_FATAL(expression) if (Logger::shouldPrint(kLoggerVerbosityFatal)) { *Logger::ofs << "FATAL: " << expression << std::endl; }

#endif // __StegoDisk__Logger__
