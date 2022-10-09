/**
 * @file    os.h
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   OS/platform specific stufff
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef os__header_included
#define os__header_included

#include <string>

/// Returns user's home directory or empty string on error
std::string GetUserHomeDirectory();

#endif // ifndef os__header_included
