# Install script for directory: /home/jstar/Desktop/github-nos3/components/cryptolib/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/jstar/Desktop/github-nos3/fsw/build/exe")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "debug")
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
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/cpu1/cf/cryptolib.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/cpu1/cf/cryptolib.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/cpu1/cf/cryptolib.so"
         RPATH "/home/jstar/Desktop/github-nos3/fsw/build/exe/cpu1/cf")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/cpu1/cf" TYPE MODULE FILES "/home/jstar/Desktop/github-nos3/fsw/build/amd64-nos3/default_cpu1/apps/cryptolib/src/cryptolib.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/cpu1/cf/cryptolib.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/cpu1/cf/cryptolib.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/cpu1/cf/cryptolib.so"
         OLD_RPATH ":::::::::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/home/jstar/Desktop/github-nos3/fsw/build/exe/cpu1/cf")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/cpu1/cf/cryptolib.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/home/jstar/Desktop/github-nos3/fsw/build/exe/cpu1/cf/cryptolib.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/jstar/Desktop/github-nos3/fsw/build/exe/cpu1/cf/cryptolib.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/home/jstar/Desktop/github-nos3/fsw/build/exe/cpu1/cf/cryptolib.so"
         RPATH "/home/jstar/Desktop/github-nos3/fsw/build/exe/cpu1/cf")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/jstar/Desktop/github-nos3/fsw/build/exe/cpu1/cf/cryptolib.so")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/jstar/Desktop/github-nos3/fsw/build/exe/cpu1/cf" TYPE MODULE FILES "/home/jstar/Desktop/github-nos3/fsw/build/amd64-nos3/default_cpu1/apps/cryptolib/src/cryptolib.so")
  if(EXISTS "$ENV{DESTDIR}/home/jstar/Desktop/github-nos3/fsw/build/exe/cpu1/cf/cryptolib.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/jstar/Desktop/github-nos3/fsw/build/exe/cpu1/cf/cryptolib.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}/home/jstar/Desktop/github-nos3/fsw/build/exe/cpu1/cf/cryptolib.so"
         OLD_RPATH ":::::::::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/home/jstar/Desktop/github-nos3/fsw/build/exe/cpu1/cf")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/home/jstar/Desktop/github-nos3/fsw/build/exe/cpu1/cf/cryptolib.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/jstar/Desktop/github-nos3/fsw/build/exe/include/standalone.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/jstar/Desktop/github-nos3/fsw/build/exe/include" TYPE FILE FILES "/home/jstar/Desktop/github-nos3/components/cryptolib/src/../support/standalone/standalone.h")
endif()

