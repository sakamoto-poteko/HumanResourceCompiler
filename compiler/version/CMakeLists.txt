cmake_minimum_required(VERSION 3.4)
project(hrc_version)


set(PRE_CONFIGURE_FILE "git.c.in")
set(POST_CONFIGURE_FILE "${CMAKE_CURRENT_BINARY_DIR}/git.c")
include(${CMAKE_SOURCE_DIR}/cmake/git_watcher.cmake)

add_library(${PROJECT_NAME} ${POST_CONFIGURE_FILE})
add_dependencies(${PROJECT_NAME} check_git)
