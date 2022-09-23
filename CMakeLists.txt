cmake_minimum_required(VERSION 3.10)

project(mrdle)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(mrdle	main.cpp mrdle.cpp word_list.cpp	mrdle.h util.h)

# https://fmt.dev/latest/index.html
add_subdirectory(fmt)
find_package(fmt)
target_link_libraries(mrdle fmt::fmt)

add_dependencies(mrdle fmt)
