#include "Versioning.h"
#include "version.cmake.h"

// Function definitions
const char *git_branch()
{
    return GIT_BRANCH;
}

const char *git_tag()
{
    return GIT_TAG;
}

const char *git_commit_hash()
{
    return GIT_COMMIT_HASH;
}

const char *git_dirty()
{
    return GIT_DIRTY;
}

const char *build_timestamp()
{
    return BUILD_TIMESTAMP;
}

const char *compiler_version()
{
    return COMPILER_VERSION;
}

const char *build_type()
{
    return BUILD_TYPE;
}
