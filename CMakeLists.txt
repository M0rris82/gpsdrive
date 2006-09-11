project(gpsdrive)

# global needed variables
SET(APPLICATION_NAME "gpsdrive")
SET(APPLICATION_VERSION "2.10pre3")

# required cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.4.3)
SET(CMAKE_COLOR_MAKEFILE ON)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

# options
OPTION(WITH_PLUGINS
  "Enable gpsdrive plugins"
  ON)

# make some more macros available
INCLUDE(MacroOptionalFindPackage)
INCLUDE(MacroGeneratePoFiles)

# macro_optional_find_package() is the same as FIND_PACKAGE(<name>) but additionally creates an OPTION(WITH_<name>)
# so the checking for the software can be disabled via ccmake or -DWITH_<name>=OFF
MACRO_OPTIONAL_FIND_PACKAGE(DBUS)


include(DefineInstallationPaths)
include(DefineGPSDriveDefaults)
include(DefineCompilerFlags)
include(ConfigureChecks.cmake)

CONFIGURE_FILE (config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

SUBDIRS(
  cmake
  src
  data
  man
  scripts
)

