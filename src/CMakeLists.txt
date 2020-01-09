project(kdesdk)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

# search packages used by KDE
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
include(CheckIncludeFile)
include(CheckIncludeFileCXX)
include(CheckLibraryExists)

macro_optional_find_package(KdepimLibs)

# The FindKDE4.cmake module sets _KDE4_PLATFORM_DEFINITIONS with
# definitions like _GNU_SOURCE that are needed on each platform.
set(CMAKE_REQUIRED_DEFINITIONS ${_KDE4_PLATFORM_DEFINITIONS} -DQT_STRICT_ITERATORS)

find_package(Strigi REQUIRED)

macro_optional_find_package(LibXslt)
macro_log_feature(LIBXSLT_FOUND "LibXSLT" "A library to transform XMLfiles into other XML files" "http://xmlsoft.org/XSLT" FALSE "" "Required to build Umbrello.")
macro_optional_find_package(LibXml2)
macro_log_feature(LIBXML2_FOUND "LibXML2" "Libraries used to develop XML applications" "http://xmlsoft.org" FALSE "" "Required to build Umbrello.")

set(Boost_MINIMUM_VERSION 1.33.1)
macro_optional_find_package(Boost)
macro_log_feature(Boost_FOUND "boost" "Boost C++ Libraries" "http://www.boost.org" FALSE "1.33.1" "Required to build Umbrello.")

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${KDEPIMLIBS_INCLUDE_DIR})

macro_optional_add_subdirectory(kate)
if( UNIX )
  macro_optional_add_subdirectory(cervisia)
endif( UNIX )
macro_optional_add_subdirectory(doc)
macro_optional_add_subdirectory(kapptemplate)
macro_optional_add_subdirectory(lokalize)
if(KDEPIMLIBS_FOUND)
macro_optional_add_subdirectory(kbugbuster)
macro_optional_add_subdirectory(kdeaccounts-plugin)
endif(KDEPIMLIBS_FOUND)

if(NOT WIN32)
   macro_optional_add_subdirectory(kcachegrind)
endif(NOT WIN32)

macro_optional_add_subdirectory(kdepalettes)
macro_optional_add_subdirectory(kpartloader)
macro_optional_add_subdirectory(strigi-analyzer)
macro_optional_add_subdirectory(kioslave)

if (CMAKE_SYSTEM_NAME MATCHES Linux)
  #kmtrace is highly non-portable and requires glibc
  #TODO: replace with a check for glibc
  macro_optional_add_subdirectory(kmtrace)
endif (CMAKE_SYSTEM_NAME MATCHES Linux)

macro_optional_add_subdirectory(kompare)
macro_optional_add_subdirectory(kprofilemethod)

message(STATUS "remember to port kspy to Qt4's metaobjects")
#add_subdirectory(kspy)

# By default in kde3 kstartpref was not compiled, but well, if it compiles...
check_include_file(ltdl.h HAVE_LTDL_H)
if(HAVE_LTDL_H AND NOT APPLE AND NOT WIN32)
  macro_optional_add_subdirectory(kstartperf)
endif(HAVE_LTDL_H AND NOT APPLE AND NOT WIN32)

macro_optional_add_subdirectory(kuiviewer)
message(STATUS "remember to port kunittest")
#add_subdirectory(kunittest)
macro_optional_add_subdirectory(poxml)
message(STATUS "remember to port scheck (check style)")
#add_subdirectory(scheck)
macro_optional_add_subdirectory(scripts)

if(LIBXSLT_FOUND AND LIBXML2_FOUND AND Boost_FOUND)
  macro_optional_add_subdirectory(umbrello)
endif(LIBXSLT_FOUND AND LIBXML2_FOUND AND Boost_FOUND)

macro_display_feature_log()
