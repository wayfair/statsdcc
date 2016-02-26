
/**
 * Source file for the Logger class
 * Please see Logger.h for documentation
 */

#include <memory>

#include "statsdcc/logger.h"

#include "statsdcc/os.h"

namespace statsdcc {

std::unordered_map<Logger::LogLevel,
  std::shared_ptr<Logger>,
  Logger::LogLevelHasher> Logger::loggers;

void Logger::log(const std::string& message, Logger::LogLevel level) const {
  switch (level) {
    case Logger::LogLevel::error:
      syslog(LOG_ERR, "[error] [%s] [%d] %s\n",
        chrono::current_time().c_str(),
        os::get_tid(),
        message.c_str());
      break;
    case Logger::LogLevel::warn:
      syslog(LOG_WARNING, "[warn] [%s] [%d] %s\n",
        chrono::current_time().c_str(),
        os::get_tid(),
        message.c_str());
      break;
    case Logger::LogLevel::info:
      if (static_cast<int16_t>(this->level) <
          static_cast<int16_t>(Logger::LogLevel::warn)) {
        syslog(LOG_INFO, "[info] [%s] [%d] %s\n",
          chrono::current_time().c_str(),
          os::get_tid(),
          message.c_str());
      }
      break;
    default:  // debug
      if (static_cast<int16_t>(this->level) <
          static_cast<int16_t>(Logger::LogLevel::info)) {
        syslog(LOG_DEBUG, "[debug] [%s] [%d] %s\n",
          chrono::current_time().c_str(),
          os::get_tid(),
          message.c_str());
      }
      break;
  }
}

}  // namespace statsdcc
