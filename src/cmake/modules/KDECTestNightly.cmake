# This file provides macros which make running Nightly builds easier,
# see KDE/kdelibs/KDELibsNightly.cmake or kdesupport/automoc/Automoc4Nightly.cmake
# as example.
# It can be used completely independent from KDE, the "KDE_" prefix is just to 
# show that the macros and variables or not ctest-builtin ones, but coming
# from this script.
#
#
# Beside the variables and macros this script provides, it also does one more thing,
# it converts command line arguments into ctest (cmake) variables.
# CMake has the option -D to set variables, which ctest is currently missing.
# But there is a work around, invoke ctest like this:
#  $ ctest -S myscript.cmake,VAR1=foo,VAR2=bar
#
# By including this (KDECTestNightly.cmake) script, the variables VAR1 will be 
# set to "foo" and VAR2 to "bar" respectively. This works for any number of variables.
# Processing lists (which have semicolons) may have issues.
#
#
# KDECTestNightly.cmake uses the following variables, which you may want to set 
# before calling any macros from this file:
#
#  KDE_CTEST_BUILD_SUFFIX - this string will be appended to CMAKE_SYSTEM_NAME to 
#                           when setting CTEST_BUILD_NAME. 
#
#  KDE_CTEST_VCS - set it to "svn" (for KDE) or "cvs".
#                  Send me a mail (or even better a patch) if you need support for git.
#
#  KDE_CTEST_VCS_REPOSITORY - set this to the VCS repository of the project, e.g.
#                       "https://svn.kde.org/home/kde/trunk/KDE/kdelibs" for kdelibs.
#
#  KDE_CTEST_VCS_PATH - the path inside the repository. This can be empty for svn, 
#                       but you have to set it for cvs.
#
#  KDE_CTEST_AVOID_SPACES - by default, the source directory for the Nightly build
#                           will contains spaces ("src dir/") and also the build 
#                           directory. If this breaks the compilation of your software,
#                           either fix your software, or set KDE_CTEST_AVOID_SPACES 
#                           to TRUE.
#
#
# The following macros are provided:
#
#  KDE_CTEST_SETUP(<file>) - This macro does most of the work you would otherwise 
#                         have to do manually. <file> should always be the full path
#                         to the calling CTest script, i.e. ${CMAKE_CURRENT_LIST_FILE}.
#    This macro sets up the following variables:
#      CTEST_BUILD_NAME - unchanged if already set, otherwise set to CMAKE_SYSTEM_NAME, with
#                         KDE_CTEST_BUILD_SUFFIX appended (if set)
#      CTEST_BINARY_DIRECTORY - unchanged if already set, otherwise set to $HOME/Dashboards/<projectname>/build dir-${CTEST_BUILD_NAME}/
#                         Set KDE_CTEST_AVOID_SPACES to TRUE to avoid the space in the path
#      CTEST_SOURCE_DIRECTORY - unchanged if already set, otherwise set to $HOME/Dashboards/<projectname>/src dir/
#                         Set KDE_CTEST_AVOID_SPACES to TRUE to avoid the space in the path
#      CTEST_SITE, CTEST_BUILD_COMMAND, CTEST_CHECKOUT_COMMAND, CTEST_UPDATE_COMMAND
#
#  KDE_CTEST_INSTALL(<dir>) - Installs the project in the given directory. 
#                         <dir> should always be ${CTEST_BINARY_DIRECTORY}.
#
#  KDE_CTEST_WRITE_INITIAL_CACHE(<dir> [var1 var2 ... varN] ) - Writes an initial 
#                         CMakeCache.txt to the directory <dir> (which should always 
#                         be ${CTEST_BINARY_DIRECTORY}). This CMakeCache.txt will contain
#                         the listed variables and their values. If any of the variables
#                         have not been set, they won't be written into this cache.
#                         This is the only way to get predefined values to variables 
#                         during the CMake run, as e.g. CMAKE_INSTALL_PREFIX.
#
#
# Alex <neundorf AT kde.org>

# Copyright (c) 2009 Alexander Neundorf, <neundorf@kde.org>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

###########################################################
# generic code
###########################################################

cmake_minimum_required(VERSION 2.6)

if(POLICY CMP0011)
   cmake_policy(SET CMP0011 NEW)
endif(POLICY CMP0011)


# transform ctest script arguments of the form script.ctest,var1=value1,var2=value2 
# to variables with the respective names set to the respective values
string(REPLACE "," ";" scriptArgs "${CTEST_SCRIPT_ARG}")
foreach(currentVar ${scriptArgs})
   if ("${currentVar}" MATCHES "^([^=]+)=(.+)$" )
      set("${CMAKE_MATCH_1}" "${CMAKE_MATCH_2}")
   endif ("${currentVar}" MATCHES "^([^=]+)=(.+)$" )
endforeach(currentVar ${scriptArgs})


macro(KDE_CTEST_WRITE_INITIAL_CACHE _dir)
   if(NOT EXISTS "${_dir}")
      file(MAKE_DIRECTORY "${_dir}")
   endif(NOT EXISTS "${_dir}")
   file(WRITE "${_dir}/CMakeCache.txt" "# CMakeCache.txt created by KDE_CTEST_WRITE_INITIAL_CACHE() macro\n" )

   foreach(currentArg ${ARGN})
      if(DEFINED ${currentArg})
         file(APPEND "${_dir}/CMakeCache.txt" "${currentArg}:STRING=${${currentArg}}\n\n")
      endif(DEFINED ${currentArg})
   endforeach(currentArg ${ARGN})
endmacro(KDE_CTEST_WRITE_INITIAL_CACHE)


macro(KDE_CTEST_INSTALL _dir)
   if(EXISTS ${_dir})
      if (EXISTS ${_dir}/cmake_install.cmake)
         execute_process(COMMAND ${CMAKE_COMMAND} -P ${_dir}/cmake_install.cmake)
      else (EXISTS ${_dir}/cmake_install.cmake)
         message("Nothing found to install in ${_dir}")
      endif (EXISTS ${_dir}/cmake_install.cmake)
   else(EXISTS ${_dir})
      message("KDE_CTEST_INSTALL(): directory ${_dir} does not exist")
   endif(EXISTS ${_dir})
endmacro(KDE_CTEST_INSTALL _dir)


macro(KDE_CTEST_SETUP absCtestScript)

   include(CMakeDetermineSystem)
   if(CMAKE_HOST_UNIX)
      include(Platform/UnixPaths)
   endif(CMAKE_HOST_UNIX)

   if(CMAKE_HOST_WIN32)
      include(Platform/WindowsPaths)
   endif(CMAKE_HOST_WIN32)

   get_filename_component(currentDirectory "${absCtestScript}" PATH)

   if(NOT EXISTS "${currentDirectory}/CMakeLists.txt")
      get_filename_component(fileName "${absCtestScript}" NAME)
      message(FATAL_ERROR "${fileName} must be in the source tree of your project.")
   endif(NOT EXISTS "${currentDirectory}/CMakeLists.txt")

   # these must be read here since the Nightly start time is required to check out the project
   # project, but without having 
   include("${currentDirectory}/CTestConfig.cmake")

   site_name(CTEST_SITE)

   if(KDE_CTEST_BUILD_SUFFIX)
      set(KDE_CTEST_BUILD_SUFFIX "-${KDE_CTEST_BUILD_SUFFIX}")
   endif(KDE_CTEST_BUILD_SUFFIX)

   if(NOT CTEST_BUILD_NAME)
      set(CTEST_BUILD_NAME ${CMAKE_SYSTEM_NAME}${KDE_CTEST_BUILD_SUFFIX})
   endif(NOT CTEST_BUILD_NAME)


   if("${CTEST_CMAKE_GENERATOR}" MATCHES Makefile)
      find_program(MAKE_EXECUTABLE make gmake)
      set(CTEST_BUILD_COMMAND    "${MAKE_EXECUTABLE}" )
   else("${CTEST_CMAKE_GENERATOR}" MATCHES Makefile)
      if(NOT DEFINED CTEST_BUILD_COMMAND)
         message(FATAL_ERROR "CTEST_CMAKE_GENERATOR is set to \"${CTEST_CMAKE_GENERATOR}\", but CTEST_BUILD_COMMAND has not been set")
      endif(NOT DEFINED CTEST_BUILD_COMMAND)
   endif("${CTEST_CMAKE_GENERATOR}" MATCHES Makefile)


   ############# set up CTEST_SOURCE_DIRECTORY and CTEST_BINARY_DIRECTORY #############
   set(DASHBOARD_DIR "$ENV{HOME}/Dashboards" )

   if(NOT DEFINED CTEST_SOURCE_DIRECTORY)
      if(KDE_CTEST_AVOID_SPACES)
         set(CTEST_SOURCE_DIRECTORY "${DASHBOARD_DIR}/${CTEST_PROJECT_NAME}/srcdir" )
      else(KDE_CTEST_AVOID_SPACES)
         set(CTEST_SOURCE_DIRECTORY "${DASHBOARD_DIR}/${CTEST_PROJECT_NAME}/src dir" )
      endif(KDE_CTEST_AVOID_SPACES)
   endif(NOT DEFINED CTEST_SOURCE_DIRECTORY)

   if(NOT DEFINED CTEST_BINARY_DIRECTORY)
      if(KDE_CTEST_AVOID_SPACES)
         set(CTEST_BINARY_DIRECTORY "${DASHBOARD_DIR}/${CTEST_PROJECT_NAME}/builddir-${CTEST_BUILD_NAME}" )
      else(KDE_CTEST_AVOID_SPACES)
         set(CTEST_BINARY_DIRECTORY "${DASHBOARD_DIR}/${CTEST_PROJECT_NAME}/build dir-${CTEST_BUILD_NAME}" )
      endif(KDE_CTEST_AVOID_SPACES)
   endif(NOT DEFINED CTEST_BINARY_DIRECTORY)


   ############### set up VCS support ###################

   string(TOLOWER ${KDE_CTEST_VCS} _ctest_vcs)
   set(_have_vcs FALSE)
   # only set this if there is no checkout yet
   set(CTEST_CHECKOUT_COMMAND)

   if ("${_ctest_vcs}" STREQUAL svn)
      find_program(SVN_EXECUTABLE svn)
      if (NOT SVN_EXECUTABLE)
         message(FATAL_ERROR "Error: KDE_CTEST_VCS is svn, but could not find svn executable")
      endif (NOT SVN_EXECUTABLE)
      set(CTEST_UPDATE_COMMAND ${SVN_EXECUTABLE})
      if(NOT EXISTS "${CTEST_SOURCE_DIRECTORY}/.svn/entries")
         set(CTEST_CHECKOUT_COMMAND "${SVN_EXECUTABLE} co ${KDE_CTEST_VCS_REPOSITORY}/${KDE_CTEST_VCS_PATH} \"${CTEST_SOURCE_DIRECTORY}\"")
      endif(NOT EXISTS "${CTEST_SOURCE_DIRECTORY}/.svn/entries")
      set(_have_vcs TRUE)
   endif ("${_ctest_vcs}" STREQUAL svn)

   if ("${_ctest_vcs}" STREQUAL cvs)
      find_program(CVS_EXECUTABLE cvs cvsnt)
      if (NOT CVS_EXECUTABLE)
         message(FATAL_ERROR "Error: KDE_CTEST_VCS is cvs, but could not find cvs or cvsnt executable")
      endif (NOT CVS_EXECUTABLE)
      set(CTEST_UPDATE_COMMAND ${CVS_EXECUTABLE})
      if(NOT EXISTS "${CTEST_SOURCE_DIRECTORY}/CVS/Entries")
         set(CTEST_CHECKOUT_COMMAND "${CVS_EXECUTABLE} -d ${KDE_CTEST_VCS_REPOSITORY} co  -d \"${CTEST_SOURCE_DIRECTORY}\" ${KDE_CTEST_VCS_PATH}")
      endif(NOT EXISTS "${CTEST_SOURCE_DIRECTORY}/CVS/Entries")
      set(_have_vcs TRUE)
   endif ("${_ctest_vcs}" STREQUAL cvs)

endmacro(KDE_CTEST_SETUP)
