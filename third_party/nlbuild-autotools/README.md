This project uses nlbuild-autotools (see repo/README.md) as its build system.
The nlbuild-autotools package is inlined into the project to avoid a bootstrap
dependency issue for users who want to build the project.

# Updating nlbuild-autotools

## Via git subtree

### Initial Add

See "Getting Started" in repo/README.md.

### Subsequent Update

Assuming the project copy of nlbuild-autotools was created with git  
subtree, a subsequent update can be accomplished with the following, replacing
HEAD with a named, tagged version if the HEAD commit is not what is desired.

```
% git subtree pull --prefix=third_party/nlbuild-autotools/repo --squash --message="Update subtree mirror of repository 'https://github.com/nestlabs/nlbuild-autotools.git' branch 'master' at commit 'HEAD'." nlbuild-autotools HEAD
```

The downside of the subtree approach is that some of the subtree state does not
flow along with the project upstream. Consequently, if the same maintainer is
not updating the subtree from the same clone instance on each update, the update
process may be more difficult or may fail. In addition, any attempt to rebase
while a subtree update is in flight will result in the loss of the subtree.

Finally, subtree is an optional, contributor package in git and is not included
in all git distributions. Consequently, it may not be available to all
maintainers.

## Via git, tar, and Bare File System Operations

For better or worse, the failsafe way to update the project instance is to do so
with git clone, tar, and bare file system operations.

_Clone the Project Repository_

    % git clone https://github.com/nestlabs/nlbuild-autotools.git nlbuild-autotools

_Create an Archive of the nlbuild-autotools Project_

    % git archive --format=tgz `cat .default-version` > nlbuild-autotools-`cat .default-version`.tgz

Replace the `cat .default-version` with HEAD or some other named, tagged version
if you don't want the latest version.

_Clean-out the Existing In-project Copy_

    % cd <this project>
    % rm -rf third_party/nlbuild-autotools/repo/* third_party/nlbuild-autotools/repo/.[a-zA-Z]*

_Overlay the nlbuild-autotools Project Archive_

    % tar --directory=third_party/nlbuild-autotools/repo -zxf <path to nlbuild-autotools-x.y.z.tgz>

_Add New Files and Remove Old Files_

    % git add third_party/nlbuild-autotools/repo
    % git rm `git status -uno -s | sed -n -e '/^ D / { s,^ D \(.\+$\),\1,g; p; }'`

_Commit the Changes_

    % git commit --message="Update subtree mirror of repository 'https://github.com/nestlabs/nlbuild-autotools.git' branch 'master' at commit 'HEAD'." third_party/nlbuild-autotools/repo
