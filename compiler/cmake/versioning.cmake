# This CMake script generates a version header with Git and build info.

# Ensure Git is found
find_package(Git REQUIRED)

# Get the current git branch
execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get the latest git tag or commit description (use commit hash if no tag is present)
execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --tags --always --dirty
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_TAG
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# If git describe returns nothing, default to commit hash
if(GIT_TAG STREQUAL "")
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_TAG
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif()

# Get the git commit hash
execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Check if the working directory is dirty (i.e., uncommitted changes)
execute_process(
    COMMAND ${GIT_EXECUTABLE} diff --quiet
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    RESULT_VARIABLE GIT_DIFF_RESULT
)

if(NOT GIT_DIFF_RESULT EQUAL 0)
    set(GIT_DIRTY "-dirty")
else()
    set(GIT_DIRTY "")
endif()

# Get the current timestamp for when the build is happening
string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%d %H:%M:%S")

# Set compiler information
set(COMPILER_VERSION "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_VERSION}")

# Generate version.h from version.h.in
configure_file(
    ${PRE_CONFIGURE_FILE}
    ${POST_CONFIGURE_FILE}
    @ONLY
)
