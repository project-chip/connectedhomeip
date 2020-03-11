Nest Labs Build - GNU Autotools
===============================

[![Build Status][nlbuild-autotools-travis-svg]][nlbuild-autotools-travis]

[nlbuild-autotools-travis]: https://travis-ci.org/nestlabs/nlbuild-autotools
[nlbuild-autotools-travis-svg]: https://travis-ci.org/nestlabs/nlbuild-autotools.svg?branch=master

# Introduction

The Nest Labs Build - GNU Autotools (nlbuild-autotools) provides a
customized, turnkey build system framework, based on GNU autotools, for
standalone Nest Labs (or other) software packages that need to support
not only building on and targeting against standalone build host
systems but also embedded target systems using GCC-based or
-compatible toolchains.

In addition, nlbuild-autotools endeavors to make it easy to support:

  * Unit and Functional Tests (via 'make check')
  * Code Coverage (via 'make coverage')
  * Code Formatting (via 'make pretty' or 'make pretty-check')
  * Documentation
  * Distribution Generation (via 'make dist' or 'make docdist')

# Users and System Integrators

## Getting Started

This project is typically subtreed (or git submoduled) into a target
project repository and serves as the seed for that project's build
system.

Assuming that you already have a project repository established in
git, perform the following in your project repository:

```
1. % git remote add nlbuild-autotools ssh://<PATH_TO_REPOSITORY>/nlbuild-autotools.git
2. % git fetch nlbuild-autotools
```

You can place the nlbuild-autotools package anywhere in your project;
however, by convention, "third_party/nlbuild-autotools/repo" is recommended:

```
3. % mkdir third_party
4. % git subtree add --prefix=third_party/nlbuild-autotools/repo --squash --message="Add subtree mirror of repository 'ssh://<PATH_TO_REPOSITORY>/nlbuild-autotools.git' branch 'master' at commit 'HEAD'." nlbuild-autotools HEAD
```

At this point, you now have the nlbuild-autotools package integrated
into your project. The next step is using the
nlbuild-autotools-provided examples as templates. To do this, a
convenience script has been provided that will get you started. You
can tune and customize the results, as needed, for your project. From
the top level of your project tree:

```
5. % third_party/nlbuild-autotools/repo/scripts/mkskeleton -I third_party/nlbuild-autotools/repo --package-description "My Fantastic Package" --package-name "mfp"
```

## Supported Build Host Systems

For Linux users, you likely already have GNU autotools installed through your system's distribution (e.g. Ubuntu). However, if your GNU autotools packages are incomplete or downrevision relative to what's required from nlbuild-autotools, nlbuild-autotools can be built and installed or can be downloaded and expanded in your local nlbuild-autotools tree.

The nlbuild-autotools system supports and has been tested against the
following POSIX-based build host systems:

  * i686-pc-cygwin
  * i686-pc-linux-gnu
  * x86_64-apple-darwin
  * x86_64-unknown-linux-gnu

### Build and Install {#Build_and_Install}

Simply invoke `make tools` at the top-level of your nlbuild-autotools
tree or, for example, from your package or project in which you have
integrated nlbuild-autotools:

```
make -C third_party/nlbuild-autotools/repo tools
```

### Download and Expand

Alongside nlbuild-autotools distributions are pre-built
architecture-independent and -dependent distributions of the form:

  * nlbuild-autotools-common-_version_.tar.{gz,xz}
  * nlbuild-autotools-_host_-_version_.tar.{gz,xz}

Find and download the appropriate pair of nlbuild-autotools-common and
nlbuild-autotools-_host_ for your system and then expand them from the
top-level of your nlbuild-autotools tree with a command similar to the
following example:

```
% tar --directory tools/host -zxvf nlbuild-autotools-common-1.1.tar.gz
% tar --directory tools/host -zxvf nlbuild-autotools-x86_64-unknown-linux-gnu-1.1.tar.gz
```

Please see the [FAQ](#FAQ) section for more background on why this package
provides options for these pre-built tools.

## Package Organization

The nlbuild-autotools package is laid out as follows:

| Directory                            | Description                                                                              |
|--------------------------------------|------------------------------------------------------------------------------------------|
| autoconf/                            | GNU autoconf infrastructure provided by nlbuild-autotools.                               |
| autoconf/m4/                         | GNU m4 macros for configure.ac provided by nlbuild-autotools.                            |
| automake/                            | GNU automake Makefile.am header and footer infrastructure provided by nlbuild-autotools. |
| automake/post/                       | GNU automake Makefile.am footers.                                                        |
| automake/post.am                     | GNU automake Makefile.am footer included by every makefile.                              |
| automake/pre/                        | GNU automake Makefile.am headers.                                                        |
| automake/pre.am                      | GNU automake Makefile.am header included by every makefile.                              |
| examples/                            | Example template files for starting your own nlbuild-autotools-based project.            |
| scripts/                             | Automation scripts for regenerating the build system and for managing package versions.  |
| tools/                               | Qualified packages of and pre-built instances of GNU autotools.                          |
| tools/host/                          | Pre-built instances of GNU autotools (if installed).                                     |
| tools/host/i686-pc-cygwin/           | Pre-built instances of GNU autotools for 32-bit Cygwin (if installed).                   |
| tools/host/i686-pc-linux-gnu/        | Pre-built instances of GNU autotools for 32-bit Linux (if installed).                    |
| tools/host/x86_64-apple-darwin/      | Pre-built instances of GNU autotools for 64-bit Mac OS X (if installed).                 |
| tools/host/x86_64-unknown-linux-gnu/ | Pre-built instances of GNU autotools for 64-bit Linux (if installed).                    |
| tools/packages/                      | Qualified packages for GNU autotools.                                                    |
## Internal Package Dependencies and Repositories

Your package may have dependencies on other packages that can either
be inlined into your package or can be specified externally. If your
package has such dependencies, nlbuild-autotools contains support to
facilitate easy standalone tests and a successful 'make distcheck'
target (which effectively reqires 'configure' with no arguments to
produce a successful build) without incurring the costs of inlining
these dependencies into your own package.

nlbuild-autotools supports this by providing a means to pull down
external git package repositories that your package depends on using
either git clone (default) or git submodules when you use and support
--with-<package>=internal as a location for your dependent packages.

The example 'Makefile-bootstrap' has been provided as infrastructure to
make this easy for you as a package maintainer and for your package
users. The bootstrap makefile supports both the generic 'repos' target
to pull down all repositories on which your project depends as well as
relative path targets to where those repositories might live in your
project when they are internal (e.g,
third_party/package/repo).

Consequently, you can, for example, invoke:

```
% make -f Makefile-bootstrap repos
```

or

```
% make -f Makefile-bootstrap third_party/package/repo
```

to pull down all repositories or just the repository, for example,
placed at 'third_party/package/repos'.

The bootstrap makefile generated for your package is yours to edit and
extend. In fact, hooks have been added so that you can do
package-specific work, including recursively pulling down repositories
for other packages.

However, an even better and easier approach for your package users is
to integrate the bootstrap makefile repository process into your
configure script such that when an "internal" package location is
detected, it invokes the bootstrap makefile to perform this work on
behalf of the user. For example:

```
# Depending on whether my-package has been configured for an internal
# location, its directory stem within this package needs to be set
# accordingly. In addition, if the location is internal, then we need
# to attempt to pull it down using the bootstrap makefile.

if test "${nl_with_my_package}" = "internal"; then
    maybe_my_package_dirstem="my-package/repo"
    my_package_dirstem="third_party/${maybe_my_package_dirstem}"

    AC_MSG_NOTICE([attempting to create internal ${my_package_dirstem}]) 
    ${MAKE-make} --no-print-directory -C ${ac_confdir} -f Makefile-bootstrap ${my_package_dirstem}

    if test $? -ne 0; then
        AC_MSG_ERROR([failed to create ${my_package_dirstem}. Please check your network connection or the correctness of 'repos.conf'])
    fi
else
    maybe_my_package_dirstem=""
fi

AC_SUBST(MY_PACKAGE_SUBDIRS, [${maybe_my_package_dirstem}])
AM_CONDITIONAL([PACKAGE_WITH_MY_PACKAGE_INTERNAL], [test "${nl_with_my_package}" = "internal"])
```

Note, the use of AC_SUBST on MY_PACKAGE_SUBDIRS to provide a mechanism
to conditionally populate SUBDIRS in the appropriate automake file
locations such that GNU autoconf does not generate syntax errors about
the potential absence of the subdirectory at bootstrap time.

Of course, in either case, network connectivity is required to reach
the external git server hosting the packages on which your project
depends.

In addition to the 'repos' target, the bootstrap makefile also
supports the 'clean-repos' target that undoes the work of the 'repos'
target. When using either the 'clone' or 'submodule' pull methods, it
will clean-up all of the synchronized repositories. When using the
'submodule' pull method, it also is careful to ensure it does not
disturb existing git or git submodule state your project might be
using.

The infrastructure all works, of course, whether you are working in or
out of git and whether you have colocated or non-colocated source and
build directories.

### Configuration

This dependent repository feature of nlbuild-autotools uses a file,
'repos.conf', at the top level of your project source directory to
determine what external project repositories your package might want
to pull down, the location of their git server, the branch you want to
pull, and the location in your project in which you want to place
them.

The format of 'repos.conf' _almost_ precisely follows that used by git
submodules with two notable additions, the 'pull' section and the
'commit' key. The 'pull' section allows you to optionally specify the
'method' key as 'clone' or 'submodule' (defaulting to 'clone' when the
key is not present). This selects whether 'git clone' or 'git
submodule' is used to pull down repositories. The 'commit' key allows
you to specify not only what branch to checkout but, more precisely,
what commit or tag to checkout rather than just _HEAD_ of a
branch. More information is available in 'Makefile-bootstrap' or with
`man gitmodules` or `git help gitmodules`.

# FAQ {#FAQ}

Q: Why does nlbuild-autotools have an option for its own built versions
   of GNU autotools rather than leveraging whatever versions exist on
   the build host system?

A: Some build host systems such as Mac OS X may not have GNU autotools
   at all. Other build host systems, such as Linux, may have different
   distributions or different versions of those distributions in which
   the versions of GNU autotools are apt to be different.

   This differences lead to different primary and secondary autotools
   output and, as a consequence, a divergent user and support
   experience. To avoid this, this package provides a pre-built,
   qualified set of GNU autotools along with a comprehensive,
   standalone set of scripts to drive them without reliance on those
   versions of the tools on the build host system.

Q: When I rebootstrap my package, I see that a number of files related to
   nlbuild-autotools have unexpectedly changed. Why is this happening?

A: From time to time, the packages that comprise GNU autotools change
   upstream. Frequently, common host operating systems (e.g., Ubuntu) take
   a stable snapshot of the current autotools for a major release (e.g.,
   Ubuntu 14). On the next major release (e.g., Ubuntu 16), another snapshot
   is taken.

   If your package was first bootstrapped with one version of
   autotools and those bootstrap-generated files checked-in but you
   later bootstrap with another version of autotools, then you will
   likely observe this behavior.

   There are two solutions to this problem. First, to ensure a
   consistent set of bootstrap-generated files, you can build the
   autotools included with nlbuild-autotools. The bootstrap process
   will always prefer to use those versions rather than those
   available on the build host when they are available. See the
   section [Build and Install](#Build_and_Install) for more
   information.

   The second way to ensure a consistent set of bootstrap-generated
   files is to not check them in. This does, however, require that
   package users, rather than package maintainers, perform the
   bootstrap process and does require that package users, rather than
   package maintainers, have autotools available on the build host.

## Maintainers

If you are maintaining nlbuild-autotools, you have several key things to know:

1. Generating nlbuild-autotools distributions
2. Generating optional nlbuild-autotools prebuilt binary distributions.
3. Upgrading GNU autotools packages.

### Generating Distributions

To generate a nlbuild-autotools distribution, simply invoke:

```
% make dist
```

The package version will come from tags in the source code control
system, if available; otherwise, from '.default-version'. The version
can be overridden from the PACKAGE_VERSION or VERSION environment
variables.

The resulting archive will be at the top of the package build
directory.

### Generating Optional Prebuilt Binary Distributions

To generate an optional nlbuild-autotools prebuilt binary
distribution, simply invoke:

```
% make toolsdist
```

The package version will come from the source code control system, if
available; otherwise, from `.default-version`. The version can be
overridden from the _PACKAGE_VERSION_ or _VERSION_ environment variables.

The resulting archives will be at the top of the package build
directory.

### Upgrading GNU Autotools Packages

To change or upgrade GNU autotools packages used by nlbuild-autotools,
edit the metadata for each package in tools/packages/_package_/
_package_.{url,version}.

# Versioning

nlbuild-autools follows the [Semantic Versioning guidelines](http://semver.org/) 
for release cycle transparency and to maintain backwards compatibility.

# License

nlbuild-autools is released under the [Apache License, Version 2.0 license](https://opensource.org/licenses/Apache-2.0). 
See the `LICENSE` file for more information.
