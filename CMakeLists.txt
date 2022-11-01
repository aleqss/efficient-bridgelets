# Copyright 2022 Aleksandr Popov
# This program is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version. This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
# details. You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.13)
project(randomwalks LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

# CMake sets -g by default for Debug build
set(DEBUG_OPTIONS "-Og")
set(RELEASE_OPTIONS "-O3")

option(ASAN "Use address sanitizer" Off)
option(UBSAN "Use UB sanitizer" Off)
option(PROF "Set up for use with gprof" Off)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
# Debug, Release, RelWithDebugInfo, MinSizeRel

add_compile_options("$<IF:$<CONFIG:DEBUG>,${DEBUG_OPTIONS},${RELEASE_OPTIONS}>")

if(PROF)
    add_compile_options("-pg")
    add_link_options("-pg")
endif()

if(NOT MSVC AND (ASAN OR UBSAN))
    set(SANITIZER "$<IF:$<BOOL:${ASAN}>,address,undefined>")
    set(SAN_OPTS "-fno-omit-frame-pointer" "-fsanitize=${SANITIZER}")
    add_compile_options("$<$<CONFIG:DEBUG>:${SAN_OPTS}>")
    add_link_options("$<$<CONFIG:DEBUG>:${SAN_OPTS}>")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(gmpxx REQUIRED)

# Main executable
add_executable(randomwalks main.cpp defs.cpp dp.cpp explicit.cpp problems.cpp)

set(GNU_OPTIONS
    "-pedantic" "-Wall" "-Wextra" "-Wcast-align" "-Wcast-qual" "-Wlogical-op"
    "-Wctor-dtor-privacy" "-Wdisabled-optimization" "-Wformat=2" "-Winit-self"
    "-Wmissing-declarations" "-Wshadow" "-Wmissing-include-dirs" "-Wnoexcept"
    "-Wold-style-cast" "-Woverloaded-virtual" "-Wredundant-decls" "-Wsign-promo"
    "-Wsign-conversion" "-Wstrict-null-sentinel" "-Wstrict-overflow=2" "-Wundef"
    "-Wswitch-default")

set(CLANG_OPTIONS
    "-Weverything" "-Wno-c++98-compat" "-Wno-covered-switch-default"
    "-Wno-float-equal" "-Wno-double-promotion" "-Wno-unused-template"
    "-Wno-c++98-compat-pedantic" "-Wno-padded" "-Wno-weak-template-vtables")

set(MSVC_OPTIONS "/W4")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(randomwalks PRIVATE ${GNU_OPTIONS})
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    target_compile_options(randomwalks PRIVATE ${CLANG_OPTIONS})
    target_link_options(randomwalks PUBLIC "-stdlib=libstdc++")
    # Disable this if you want to use libc++
else()
    target_compile_options(randomwalks PRIVATE ${MSVC_OPTIONS})
endif()

target_link_libraries(randomwalks PUBLIC gmp::gmpxx gmp::gmp)
