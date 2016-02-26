
/**
 * Class definition for threadsafequeue
 * Copied from Concurrence In Action Practical Mutithreading text book
 */

#ifndef INCLUDE_STATSDCC_LOGGER_H_
#define INCLUDE_STATSDCC_LOGGER_H_

#include <syslog.h>

#include <cstdint>
#include <cstring>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

#include "statsdcc/chrono.h"

namespace statsdcc {

/**
 * synchronizes operations by multiple threads on a queue
 */
class Logger {
  friend class std::shared_ptr<Logger>;  // since construtor is private

 public:
  enum class LogLevel: int16_t {
    debug = 0,
    info,
    warn,
    error
  };

  struct LogLevelHasher {
    inline std::size_t operator()(LogLevel const& level) const {
      std::hash<int> log_level_hash;
      return log_level_hash(static_cast<int>(level));
    }
  };

  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;

  Logger(Logger&&) = delete;
  Logger& operator=(Logger&&) = delete;

  inline ~Logger() {
    ::closelog();
  }

  /**
   * returns default logger
   */
  static inline std::shared_ptr<Logger> get_logger() {
    return get_logger(Logger::LogLevel::warn);
  }

  /**
   * returns instance of logger class
   */
  static inline std::shared_ptr<Logger>
  get_logger(const Logger::LogLevel& level) {
    if (Logger::loggers[level] != NULL) return Logger::loggers[level];
    Logger::loggers[level] = std::shared_ptr<Logger>(new Logger(level));
    return Logger::loggers[level];
  }

  /**
   * Logs the message as debug level
   *
   * @param message message to be logged
   */
  inline void debug(const std::string& message) const {
    this->log(message, Logger::LogLevel::debug);
  }

  /**
   * Logs the message as info level
   *
   * @param message message to be logged
   */
  inline void info(const std::string& message) const {
    this->log(message, Logger::LogLevel::info);
  }

  /**
   * Logs the message as warn level
   *
   * @param message message to be logged
   */
  inline void warn(const std::string& message) const {
    this->log(message, Logger::LogLevel::warn);
  }

  /**
   * Logs the message as error level
   *
   * @param message message to be logged
   */
  inline void error(const std::string& message) const {
    this->log(message, Logger::LogLevel::error);
  }

 private:
  inline void construct() {
    openlog(NULL,  // prepend application name
      LOG_CONS |  // Log to console if cannot send to syslogd daemon
       LOG_PERROR,  // Log to sterr as well as sending to syslogd daemon
      LOG_LOCAL7);  // facility local use
  }

  inline Logger() {
    construct();
  }

  inline explicit Logger(Logger::LogLevel level) {
    construct();
    this->level = level;
  }

  /**
   * Logs the message
   *
   * @param message message to be logged
   * @param level level of the log message
   */
  void log(const std::string& message, Logger::LogLevel level) const;

  static std::unordered_map<Logger::LogLevel,
                            std::shared_ptr<Logger>,
                            Logger::LogLevelHasher> loggers;

  Logger::LogLevel level;
};

}  // namespace statsdcc

extern std::shared_ptr<statsdcc::Logger> logger;

#endif  // INCLUDE_STATSDCC_LOGGER_H_
