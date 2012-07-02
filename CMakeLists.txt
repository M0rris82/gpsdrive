project(gpsdrive)

# global needed variables
set(APPLICATION_NAME "gpsdrive")
set(APPLICATION_VERSION "2.12.svn")

# required cmake version
cmake_minimum_required(VERSION 2.4.3)
set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_VERBOSE_MAKEFILE OFF)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

# set gcc compiler flags for -DCMAKE_BUILD_TYPE=Debug
set(CMAKE_C_FLAGS_DEBUG "-g -Wall -Werror-implicit-function-declaration -Wextra -Wunused -fno-common")

# make some more macros available
include(MacroOptionalFindPackage)
include(MacroGeneratePoFiles)
include(MacroEnsureOutOfSourceBuild)
include(MacroGetSubversionRevision)
macro_get_subversion_revision(SVN_REVISION)

# disallow in-source build
macro_ensure_out_of_source_build("${APPLICATION_NAME} requires an out of source build. Please create a separate build directory and run 'cmake /path/to/gpsdrive [options]' there.")


include(DefineInstallationPaths)
include(DefineProjectDefaults)
include(DefineCompilerFlags)
include(DefineCPackDefaults)
include(ConfigureChecks.cmake)
include(DefineOptions.cmake)

# allow creation of source package with target 'package'
include(CPack)
set(CPACK_GENERATOR "TGZ")


configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_subdirectory(src)
add_subdirectory(data)
add_subdirectory(man)
add_subdirectory(po)
add_subdirectory(scripts)

if (MAEMO)
  add_subdirectory(contrib/maemo)
endif (MAEMO)
