
/**
 * Source file for status.h
 * Please see status.h for documentation
 */

#include "statsdcc/status.h"

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <cstring>
#include <fstream>
#include <iostream>

namespace statsdcc {

bool read_status(std::string status_file,
                 const std::vector<std::string> &keys,
                 Json::Value& status) {
  std::ifstream proc_file(status_file);
  if (!proc_file.is_open()) return false;

  std::string line;
  while (getline(proc_file, line)) {
    std::vector<std::string> key_value;
    boost::split(key_value, line, boost::is_any_of(":"));
    std::string key = boost::trim_right_copy(key_value[0]);

    if (key_value.size() == 2)  {
      std::string key = boost::trim_right_copy(key_value[0]);
      if (keys.size() != 0 &&
          std::find(keys.cbegin(), keys.cend(), key) == keys.cend()) {
        continue;
      }

      std::string value = boost::trim_left_copy(key_value[1]);
      boost::replace_all(value, "\t", "    ");
      status[key] = value;
    }
  }
  return true;
}

}  // namespace statsdcc
