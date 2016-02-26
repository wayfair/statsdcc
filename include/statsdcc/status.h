
#ifndef INCLUDE_STATSDCC_STATUS_H_
#define INCLUDE_STATSDCC_STATUS_H_

#include <json/json.h>

#include <string>
#include <vector>

namespace statsdcc {

 /**
  * Reads the various resource's status from a given status file
  *
  * @param status_file file path to the status file (e.g., /proc/PID/status)
  * @param keys params that should be included in the result (e.g., State)
  * @param status the status read from the file in Json object
  *
  * @return true if sucess else false
  */
bool read_status(std::string status_file,
                 const std::vector<std::string> &keys,
                 Json::Value& status);

}  // namespace statsdcc

#endif  // INCLUDE_STATSDCC_STATUS_H_
