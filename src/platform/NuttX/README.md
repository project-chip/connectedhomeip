# Overview of CHIP NuttX Adaption

This platform is based on Linux adaptation, The code introduction can be seen in
Linux's README.md:`src/platform/Linux/README.md`

To avoid integration errors caused by CI build breaks on the NuttX platform when
new features are added to the Linux code, the current Linux code was copied as
the base for NuttX to avoid a strong dependency between the two platforms.

The reason for adapting based on Linux is that NuttX is also a POSIX-compliant
operating system, and the code can be almost completely reused while keeping the
definitions and comments with the Linux prefix, which makes it easier to
cherry-pick modifications from the Linux platform to the NuttX platform in the
future.
