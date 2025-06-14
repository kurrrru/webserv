#pragma once

#include <string>

namespace http {

/**
 * @brief Get the current date in HTTP format.
 * 
 * The format is "Day, DD Mon YYYY HH:MM:SS GMT".
 * Example: "Mon, 01 Jan 2023 12:00:00 GMT"
 * 
 * @return A string representing the current date in HTTP format.
 */
std::string getCurrentGMT();

}  // namespace http
