# Install script for directory: /workspace/hunspell/hunspell

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/hunspell" TYPE FILE FILES
    "/workspace/hunspell/hunspell/affentry.hxx"
    "/workspace/hunspell/hunspell/affixmgr.hxx"
    "/workspace/hunspell/hunspell/atypes.hxx"
    "/workspace/hunspell/hunspell/baseaffix.hxx"
    "/workspace/hunspell/hunspell/csutil.hxx"
    "/workspace/hunspell/hunspell/filemgr.hxx"
    "/workspace/hunspell/hunspell/hashmgr.hxx"
    "/workspace/hunspell/hunspell/htypes.hxx"
    "/workspace/hunspell/hunspell/hunspell.h"
    "/workspace/hunspell/hunspell/hunspell.hxx"
    "/workspace/hunspell/hunspell/hunvisapi.h"
    "/workspace/hunspell/hunspell/hunzip.hxx"
    "/workspace/hunspell/hunspell/langnum.hxx"
    "/workspace/hunspell/hunspell/phonet.hxx"
    "/workspace/hunspell/hunspell/replist.hxx"
    "/workspace/hunspell/hunspell/suggestmgr.hxx"
    "/workspace/hunspell/hunspell/w_char.hxx"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/workspace/build/hunspell/hunspell/libhunspell.a")
endif()

