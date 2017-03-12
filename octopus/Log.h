#ifndef LOG_H
#define LOG_H

#include "Base.h"
#include "ScopedLoger.h"

#define LOG_TRACE if (Octopus::ScopedLoger::getLogLevel() <= ScopedLoger::TRACE) \
  Octopus::ScopedLoger(__FILE__, __LINE__, Octopus::ScopedLoger::TRACE, __func__).stream()
#define LOG_DEBUG if (Octopus::ScopedLoger::getLogLevel() <= Octopus::ScopedLoger::DEBUG) \
  Octopus::ScopedLoger(__FILE__, __LINE__, Octopus::ScopedLoger::DEBUG, __func__).stream()
#define LOG_INFO if (Octopus::ScopedLoger::getLogLevel() <= Octopus::ScopedLoger::INFO) \
  Octopus::ScopedLoger(__FILE__, __LINE__).stream()
#define LOG_WARN Octopus::ScopedLoger(__FILE__, __LINE__, Octopus::ScopedLoger::WARN).stream()
#define LOG_ERROR Octopus::ScopedLoger(__FILE__, __LINE__, Octopus::ScopedLoger::ERROR).stream()
#define LOG_FATAL Octopus::ScopedLoger(__FILE__, __LINE__, Octopus::ScopedLoger::FATAL).stream()
#define LOG_SYSERR Octopus::ScopedLoger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL Octopus::ScopedLoger(__FILE__, __LINE__, true).stream()

#endif
