cmake_minimum_required(VERSION 3.10)

project(mrdle)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(mrdle main.cpp mrdle.cpp PlayerStats.cpp word_list.cpp os.cpp)

# let the preprocessor know about the system name
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_compile_definitions(mrdle PUBLIC "MRDLE_PLAT_LINUX")
endif()
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_compile_definitions(mrdle PUBLIC "MRDLE_PLAT_WINDOWS")
endif()

# https://fmt.dev/latest/index.html
add_subdirectory(fmt)
target_link_libraries(mrdle fmt::fmt)
