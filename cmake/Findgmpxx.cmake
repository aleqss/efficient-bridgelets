# Copyright 2022 Aleksandr Popov
# This program is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version. This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
# details. You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

#[=======================================================================[.rst:
Findgmpxx
-------

Finds the GMPXX library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``gmp::gmpxx``
  The GMP C++ interface.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``gmpxx_FOUND``
	True if the system has the GMPXX library.
``gmpxx_VERSION``
	The version of the GMPXX library that was found.
``gmpxx_INCLUDE_DIRS``
	Include directories needed to use GMPXX.
``gmpxx_LIBRARIES``
	Libraries needed to link to GMPXX.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``gmpxx_INCLUDE_DIR``
	The directory containing ``gmpxx.h``.
``gmpxx_LIBRARY``
	The path to the GMPXX library.

#]=======================================================================]

# This file follows the example in ``man 7 cmake-developer``.

find_package(gmp QUIET)

if(gmp_FOUND)
	find_package(PkgConfig)
	pkg_check_modules(PC_gmpxx QUIET gmpxx)

	find_path(gmpxx_INCLUDE_DIR NAMES gmpxx.h PATHS ${PC_gmpxx_INCLUDE_DIRS})
	find_library(gmpxx_LIBRARY NAMES gmpxx PATHS ${PC_gmpxx_LIBRARY_DIRS})
	set(gmpxx_VERSION ${PC_gmpxx_VERSION})

	include(FindPackageHandleStandardArgs)
	find_package_handle_standard_args(gmpxx
		FOUND_VAR gmpxx_FOUND
		REQUIRED_VARS gmpxx_LIBRARY gmpxx_INCLUDE_DIR
		VERSION_VAR gmpxx_VERSION
	)

	if(gmpxx_FOUND AND NOT TARGET gmp::gmpxx)
		add_library(gmp::gmpxx UNKNOWN IMPORTED)
		set_target_properties(gmp::gmpxx PROPERTIES
			IMPORTED_LOCATION "${gmpxx_LIBRARY}"
			INTERFACE_COMPILE_OPTIONS "${PC_gmpxx_CFLAGS_OTHER}"
			INTERFACE_INCLUDE_DIRECTORIES "${gmpxx_INCLUDE_DIR}"
		)
	endif()

	mark_as_advanced(gmpxx_INCLUDE_DIR gmpxx_LIBRARY)
else()
	message(FATAL_ERROR "GMPXX requires GMP")
endif()
