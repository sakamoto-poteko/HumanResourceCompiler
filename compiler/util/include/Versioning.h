#ifndef VERSIONING_H
#define VERSIONING_H

const char *git_branch();
const char *git_tag();
const char *git_commit_hash();
const char *git_dirty();
const char *build_timestamp();
const char *build_type();
const char *compiler_version();

#endif