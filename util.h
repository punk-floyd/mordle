/**
 * @file    util.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   Utility functions
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef util__header_included
#define util__header_included

#include <string>

/// Convert given string to lower case
static inline std::string& string_to_lower(std::string& s)
{
    std::transform(s.begin(), s.end(), s.begin(),
        [](const char& ch){return std::tolower(ch);});

    return s;
}

/// Trim leading and/or trailing whitespace from string
static inline std::string& string_trim (std::string& s,
    bool trim_left = true, bool trim_right = true)
{
    // Whitespace characters that will be stripped
    static const std::string_view my_ws(" \t\n\r\f\v");

    if (trim_left)
        s.erase(0, s.find_first_not_of(my_ws));
    if (trim_right)
        s.erase(s.find_last_not_of(my_ws) + 1);

    return s;
}

#endif // ifndef util__header_included
