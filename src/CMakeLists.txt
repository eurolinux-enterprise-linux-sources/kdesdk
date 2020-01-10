# SELF-CONTAINED SUBMODULES, FOR PREPARATION OF MIGRATION TO GIT

# Using macro_optional_add_subdirectory(...) does not work, because that
# needs at least "include(MacroLibrary)" before. But that would defeat the goal
# to only include the submodules here and do nothing else.
# So we create our own clone of macro_optional_add_subdirectory here,

# to continue support for everybody who does not have a complete checkout of
# the KDESDK module

# clone of macro_optional_add_subdirectory(...)
# Copyright (c) 2007, Alexander Neundorf, <neundorf@kde.org>
MACRO (KDESDK_OPTIONAL_ADD_SUBDIRECTORY _dir )
   GET_FILENAME_COMPONENT(_fullPath ${_dir} ABSOLUTE)
   IF(EXISTS ${_fullPath}/CMakeLists.txt)
      IF(DISABLE_ALL_OPTIONAL_SUBDIRECTORIES)
         SET(_DEFAULT_OPTION_VALUE FALSE)
      ELSE(DISABLE_ALL_OPTIONAL_SUBDIRECTORIES)
         SET(_DEFAULT_OPTION_VALUE TRUE)
      ENDIF(DISABLE_ALL_OPTIONAL_SUBDIRECTORIES)

      IF(DISABLE_ALL_OPTIONAL_SUBDIRS  AND NOT DEFINED  BUILD_${_dir})
         SET(_DEFAULT_OPTION_VALUE FALSE)
      ENDIF(DISABLE_ALL_OPTIONAL_SUBDIRS  AND NOT DEFINED  BUILD_${_dir})

      OPTION(BUILD_${_dir} "Build directory ${_dir}" ${_DEFAULT_OPTION_VALUE})
      IF(BUILD_${_dir})
         ADD_SUBDIRECTORY(${_dir})
      ENDIF(BUILD_${_dir})
   ENDIF(EXISTS ${_fullPath}/CMakeLists.txt)
ENDMACRO (KDESDK_OPTIONAL_ADD_SUBDIRECTORY)


cmake_minimum_required(VERSION 2.6)
project(kdesdk)

# list of self-contained submodules
kdesdk_optional_add_subdirectory(cervisia)
kdesdk_optional_add_subdirectory(dolphin-plugins)
kdesdk_optional_add_subdirectory(kapptemplate)
kdesdk_optional_add_subdirectory(kcachegrind)
kdesdk_optional_add_subdirectory(kde-dev-scripts)
kdesdk_optional_add_subdirectory(kde-dev-utils)
kdesdk_optional_add_subdirectory(kdesdk-kioslaves)
kdesdk_optional_add_subdirectory(kdesdk-strigi-analyzers)
kdesdk_optional_add_subdirectory(kdesdk-thumbnailers)
kdesdk_optional_add_subdirectory(kompare)
kdesdk_optional_add_subdirectory(lokalize)
kdesdk_optional_add_subdirectory(okteta)
kdesdk_optional_add_subdirectory(poxml)
kdesdk_optional_add_subdirectory(umbrello)
