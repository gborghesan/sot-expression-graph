# Copyright 2013, Francois Keith CNRS/LIRMM
#
# This file is part of sot-hrp4
# sot-hrp4 is not a free software

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

INCLUDE(cmake/base.cmake)
INCLUDE(cmake/boost.cmake)
INCLUDE(cmake/cpack.cmake)
INCLUDE(cmake/ros.cmake)

SET(PROJECT_NAME sot-expression-graph)
SET(PROJECT_DESCRIPTION "")
SET(PROJECT_URL "")

SET(CUSTOM_HEADER_DIR "${PROJECT_NAME}")

SET(CXX_DISABLE_WERROR True)

SET(PKG_CONFIG_ADDITIONAL_VARIABLES
  ${PKG_CONFIG_ADDITIONAL_VARIABLES}
  plugindirname
  plugindir
  )

SETUP_PROJECT()

# Those packages are not directly linked, but are used in the 
#  python scripts
ADD_REQUIRED_DEPENDENCY("dynamic-graph")
ADD_REQUIRED_DEPENDENCY("dynamic-graph-python")
ADD_REQUIRED_DEPENDENCY("sot-core >= 2.7")
find_package(Eigen REQUIRED)
include_directories(${Eigen_INCLUDE_DIRS})
ADD_ROSPACK_DEPENDENCY("expressiongraph")

# Search for dependencies.
# Boost
SET(BOOST_COMPONENTS filesystem system thread)
SEARCH_FOR_BOOST()

# Handle rpath necessary to handle ROS multiplace packages
# libraries inclusion
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Add subdirectories.
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(python)

SETUP_PROJECT_FINALIZE()
SETUP_PROJECT_CPACK()
