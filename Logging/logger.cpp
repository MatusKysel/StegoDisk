#include "Logging/logger.h"
#include <ctype.h>
#include <iostream>
#include <fstream>


LoggerVerbosityLevel Logger::verbosityLevel = kLoggerVerbosityDisabled;
std::ostream *Logger::ofs = &std::cout;

bool Logger::shouldPrint(LoggerVerbosityLevel verbosityLevel)
{
    if (Logger::verbosityLevel == kLoggerVerbosityDisabled) return false;
    if (verbosityLevel <= Logger::verbosityLevel) return true;
    return false;
}

void Logger::loggerClose()
{
	if ( ofs != &std::cout ) {
		((std::ofstream*)ofs)->close();
	}
}

void Logger::setVerbosityLevel(LoggerVerbosityLevel verbosityLevel, std::string out)
{
    Logger::verbosityLevel = verbosityLevel;
	if ( out.length() != 0 ) {
		ofs = new std::ofstream( out, std::ios::out );
		if ( ((std::ofstream*)ofs)->is_open() ) {
			*ofs << "Log started" << std::endl;
		} else
			ofs = &std::cout;
	}
}

void Logger::setVerbosityLevel(char verbosityLevel, std::string out)
{
    switch(toupper(verbosityLevel)) {
    case 'F':
        Logger::setVerbosityLevel(kLoggerVerbosityFatal);
        break;
    case 'E':
        Logger::setVerbosityLevel(kLoggerVerbosityError);
        break;
    case 'W':
        Logger::setVerbosityLevel(kLoggerVerbosityWarning);
        break;
    case 'I':
        Logger::setVerbosityLevel(kLoggerVerbosityInfo);
        break;
    case 'D':
        Logger::setVerbosityLevel(kLoggerVerbosityDebug);
        break;
    case 'T':
        Logger::setVerbosityLevel(kLoggerVerbosityTrace);
        break;
    default:
        Logger::setVerbosityLevel(kLoggerVerbosityDisabled);
    }

	if ( out.length() != 0 ) {
		ofs = new std::ofstream( out, std::ios::out );
		if ( ((std::ofstream*)ofs)->is_open() ) {
			*ofs << "Log started" << std::endl;
		} else
			ofs = &std::cout;
	}
}
