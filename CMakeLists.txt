# Set the name and the supported language of the project
project(gra C)
# Set the minimum version of cmake required to build this project
cmake_minimum_required(VERSION 2.8)

#compiler flags
set(CMAKE_C_FLAGS "-Wall")

# Use the package PkgConfig to detect GTK+ headers/library files
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
pkg_check_modules(SQLITE3 REQUIRED sqlite3)

# Setup CMake to use the right libraries.
# tell the compiler where to look for headers
# and to the linker where to look for libraries
include_directories(${GTK3_INCLUDE_DIRS} ${SQLITE3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS} ${SQLITE3_LIBRARY_DIRS})

# Add other flags to the compiler
add_definitions(${GTK3_CFLAGS_OTHER} ${SQLITE3_CFLAGS_OTHER})

# Add an executable compiled from hello.c
add_executable(gra main.c data.c paperwidget.c)
add_executable(dataTest dataTest.c data.c)

# Link the target to the GTK+ libraries
target_link_libraries(gra ${GTK3_LIBRARIES} ${SQLITE3_LIBRARIES})
target_link_libraries(dataTest ${GTK3_LIBRARIES} ${SQLITE3_LIBRARIES})