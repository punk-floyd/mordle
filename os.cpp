/**
 * @file    os.cpp
 * @author  Mike DeKoker (dekoker.mike@gmail.com)
 * @brief   OS/platform specific stufff
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "os.h"

#ifdef MRDLE_PLAT_LINUX

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

/// Returns user's home directory or empty string on error
std::string GetUserHomeDirectory()
{
    std::string home_dir;

    if (auto pw = getpwuid(getuid()))
        home_dir.assign(pw->pw_dir);

    return home_dir;
}

#endif

#ifdef MRDLE_PLAT_WINDOWS

/// Returns user's home directory or empty string on error
std::string GetUserHomeDirectory()
{
    // Use GetUserProfileDirectoryA
    return std::string();
}

#endif