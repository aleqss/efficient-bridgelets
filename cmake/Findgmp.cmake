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
Findgmp
-------

Finds the GMP library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``gmp::gmp``
  The GMP library.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``gmp_FOUND``
	True if the system has the GMP library.
``gmp_VERSION``
	The version of the GMP library that was found.
``gmp_INCLUDE_DIRS``
	Include directories needed to use GMP.
``gmp_LIBRARIES``
	Libraries needed to link to GMP.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``gmp_INCLUDE_DIR``
	The directory containing ``gmp.h``.
``gmp_LIBRARY``
	The path to the GMP library.

#]=======================================================================]

# This file follows the example in ``man 7 cmake-developer``.

find_package(PkgConfig)
pkg_check_modules(PC_gmp QUIET gmp)

find_path(gmp_INCLUDE_DIR NAMES gmp.h PATHS ${PC_gmp_INCLUDE_DIRS})
find_library(gmp_LIBRARY NAMES gmp PATHS ${PC_gmp_LIBRARY_DIRS})
set(gmp_VERSION ${PC_gmp_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(gmp
	FOUND_VAR gmp_FOUND
	REQUIRED_VARS gmp_LIBRARY gmp_INCLUDE_DIR
	VERSION_VAR gmp_VERSION
)

if(gmp_FOUND AND NOT TARGET gmp::gmp)
	add_library(gmp::gmp UNKNOWN IMPORTED)
	set_target_properties(gmp::gmp PROPERTIES
		IMPORTED_LOCATION "${gmp_LIBRARY}"
		INTERFACE_COMPILE_OPTIONS "${PC_gmp_CFLAGS_OTHER}"
		INTERFACE_INCLUDE_DIRECTORIES "${gmp_INCLUDE_DIR}"
		IMPORTED_LINK_INTERFACE_LANGUAGES "C"
	)
endif()

mark_as_advanced(gmp_INCLUDE_DIR gmp_LIBRARY)
