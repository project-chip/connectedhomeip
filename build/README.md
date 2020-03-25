# /build directory

The /build directory serves 2 purposes in the tree:

1. some top-level makefiles and developer workflows use build/XXX as the configured build tree
2. some parts of the CHIP build system live here

For this reason, most of this directory is "ignore".

If you update the contents of this directory, please also update the top level
.gitignore file.
