
# This module defines
# SDL2_FOUND, if false, do not try to link to SDL2
# SDL2::SDL2, the import target for SDL2
#
# This module responds to the the flag:
# SDL2_BUILDING_LIBRARY
# If this is defined, then no SDL2main will be linked in because
# only applications need main().
# Otherwise, it is assumed you are building an application and this
# module will attempt to locate and set the the proper link flags
# as part of the returned SDL2_LIBRARY variable.
#
# Don't forget to include SDLmain.h and SDLmain.m your project for the
# OS X framework based version. (Other versions link to -lSDL2main which
# this module will try to find on your behalf.) Also for OS X, this
# module will automatically add the -framework Cocoa on your behalf.
#
# $SDL2DIR is an environment variable that would
# correspond to the ./configure --prefix=$SDL2DIR
# used in building SDL2.
# l.e.galup  9-20-02
#
# Modified by Eric Wing.
# Added code to assist with automated building by using environmental variables
# and providing a more controlled/consistent search behavior.
# Added new modifications to recognize OS X frameworks and
# additional Unix paths (FreeBSD, etc).
# Also corrected the header search path to follow "proper" SDL guidelines.
# Added a search for SDL2main which is needed by some platforms.
# Added a search for threads which is needed by some platforms.
# Added needed compile switches for MinGW.
#
# On OSX, this will prefer the Framework version (if found) over others.
# People will have to manually change the cache values of
# SDL2_LIBRARY to override this selection or set the CMake environment
# CMAKE_INCLUDE_PATH to modify the search paths.
#
# Note that the header path has changed from SDL2/SDL.h to just SDL.h
# This needed to change because "proper" SDL convention
# is #include "SDL.h", not <SDL2/SDL.h>. This is done for portability
# reasons because not all systems place things in SDL2/ (see FreeBSD).
#
# Modified by Alex Mayfield
# Create an imported target.  This approach is much cleaner.

#=============================================================================
# Copyright 2003-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

add_library(SDL2::SDL2 UNKNOWN IMPORTED)

SET(SDL2_SEARCH_PATHS
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local
	/usr
	/sw # Fink
	/opt/local # DarwinPorts
	/opt/csw # Blastwave
	/opt
	${SDL2_PATH}
)

FIND_PATH(SDL2_INCLUDE_DIR SDL.h
	HINTS
	$ENV{SDL2DIR}
	PATH_SUFFIXES include/SDL2 include
	PATHS ${SDL2_SEARCH_PATHS}
)

if(SDL2_INCLUDE_DIR)
	set_property(TARGET SDL2::SDL2 PROPERTY INTERFACE_INCLUDE_DIRECTORIES
		${SDL2_INCLUDE_DIR})
	if(EXISTS "${SDL2_INCLUDE_DIR}/SDL_version.h")
		file(STRINGS "${SDL2_INCLUDE_DIR}/SDL_version.h" SDL2_VERSION_MAJOR_LINE REGEX "^#define[ \t]+SDL_MAJOR_VERSION[ \t]+[0-9]+$")
		file(STRINGS "${SDL2_INCLUDE_DIR}/SDL_version.h" SDL2_VERSION_MINOR_LINE REGEX "^#define[ \t]+SDL_MINOR_VERSION[ \t]+[0-9]+$")
		file(STRINGS "${SDL2_INCLUDE_DIR}/SDL_version.h" SDL2_VERSION_PATCH_LINE REGEX "^#define[ \t]+SDL_PATCHLEVEL[ \t]+[0-9]+$")
		string(REGEX REPLACE "^#define[ \t]+SDL_MAJOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL2_VERSION_MAJOR "${SDL2_VERSION_MAJOR_LINE}")
		string(REGEX REPLACE "^#define[ \t]+SDL_MINOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL2_VERSION_MINOR "${SDL2_VERSION_MINOR_LINE}")
		string(REGEX REPLACE "^#define[ \t]+SDL_PATCHLEVEL[ \t]+([0-9]+)$" "\\1" SDL2_VERSION_PATCH "${SDL2_VERSION_PATCH_LINE}")
		set(SDL2_VERSION_STRING ${SDL2_VERSION_MAJOR}.${SDL2_VERSION_MINOR}.${SDL2_VERSION_PATCH})
		unset(SDL2_VERSION_MAJOR_LINE)
		unset(SDL2_VERSION_MINOR_LINE)
		unset(SDL2_VERSION_PATCH_LINE)
		unset(SDL2_VERSION_MAJOR)
		unset(SDL2_VERSION_MINOR)
		unset(SDL2_VERSION_PATCH)
	endif()
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8) 
	set(PATH_SUFFIXES lib64 lib/x64 lib)
else() 
	set(PATH_SUFFIXES lib/x86 lib)
endif() 

FIND_LIBRARY(SDL2_LIBRARY
	NAMES SDL2
	HINTS
	$ENV{SDL2DIR}
	PATH_SUFFIXES ${PATH_SUFFIXES}
	PATHS ${SDL2_SEARCH_PATHS}
)

IF(NOT SDL2_BUILDING_LIBRARY)
	IF(NOT ${SDL2_INCLUDE_DIR} MATCHES ".framework")
		# Non-OS X framework versions expect you to also dynamically link to
		# SDL2main. This is mainly for Windows and OS X. Other (Unix) platforms
		# seem to provide SDL2main for compatibility even though they don't
		# necessarily need it.

		# Backwards compatible with SDL2MAIN_LIBRARY
		if(NOT SDL2_MAIN_LIBRARY AND SDL2MAIN_LIBRARY)
			set(SDL2_MAIN_LIBRARY ${SDL2MAIN_LIBRARY}
				CACHE PATH "Path to a library.")
		endif()

		FIND_LIBRARY(SDL2_MAIN_LIBRARY
			NAMES SDL2main
			HINTS
			$ENV{SDL2DIR}
			PATH_SUFFIXES ${PATH_SUFFIXES}
			PATHS ${SDL2_SEARCH_PATHS}
		)
	ENDIF(NOT ${SDL2_INCLUDE_DIR} MATCHES ".framework")
ENDIF(NOT SDL2_BUILDING_LIBRARY)

# SDL2 may require threads on your system.
# The Apple build may not need an explicit flag because one of the
# frameworks may already provide it.
# But for non-OSX systems, I will use the CMake Threads package.
IF(NOT APPLE)
	FIND_PACKAGE(Threads)
	IF(CMAKE_USE_PTHREADS_INIT)
		set_property(TARGET SDL2::SDL2 PROPERTY INTERFACE_COMPILE_DEFINITIONS
			"_REENTRANT" APPEND)
	ENDIF(CMAKE_USE_PTHREADS_INIT)
ENDIF(NOT APPLE)

IF(SDL2_LIBRARY)
	set_property(TARGET SDL2::SDL2 PROPERTY IMPORTED_LOCATION ${SDL2_LIBRARY})

	# In MinGW, -lmingw32 always comes first
	if(MINGW)
		set_property(TARGET SDL2::SDL2 PROPERTY INTERFACE_LINK_LIBRARIES "mingw32" APPEND)
	endif()

	# For SDL2main
	IF(NOT SDL2_BUILDING_LIBRARY)
		IF(SDL2_MAIN_LIBRARY)
			set_property(TARGET SDL2::SDL2 PROPERTY INTERFACE_LINK_LIBRARIES
				${SDL2_MAIN_LIBRARY} APPEND)
			# Ensure proper link order by specifying the library twice
			set_property(TARGET SDL2::SDL2 PROPERTY INTERFACE_LINK_LIBRARIES
				${SDL2_LIBRARY} APPEND)
		ENDIF(SDL2_MAIN_LIBRARY)
	ENDIF(NOT SDL2_BUILDING_LIBRARY)

	# For OS X, SDL2 uses Cocoa as a backend so it must link to Cocoa.
	# CMake doesn't display the -framework Cocoa string in the UI even
	# though it actually is there if I modify a pre-used variable.
	# I think it has something to do with the CACHE STRING.
	# So I use a temporary variable until the end so I can set the
	# "real" variable in one-shot.
	IF(APPLE)
		set_property(TARGET SDL2::SDL2 PROPERTY INTERFACE_LINK_LIBRARIES
		"-framework Cocoa" APPEND)
	ENDIF(APPLE)

	# For threads, as mentioned Apple doesn't need this.
	# In fact, there seems to be a problem if I used the Threads package
	# and try using this line, so I'm just skipping it entirely for OS X.
	IF(NOT APPLE)
		set_property(TARGET SDL2::SDL2 PROPERTY INTERFACE_LINK_LIBRARIES 
			${CMAKE_THREAD_LIBS_INIT} APPEND)
	ENDIF(NOT APPLE)

	# For MinGW library
	IF(MINGW)
		set_property(TARGET SDL2::SDL2 PROPERTY INTERFACE_LINK_LIBRARIES 
			"-mwindows" APPEND)
	ENDIF(MINGW)
ENDIF(SDL2_LIBRARY)

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(SDL2 REQUIRED_VARS SDL2_LIBRARY SDL2_INCLUDE_DIR VERSION_VAR SDL2_VERSION_STRING)
