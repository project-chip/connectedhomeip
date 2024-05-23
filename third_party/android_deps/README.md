# Third party Android dependencies

This directory contains files relevant to specifying Android dependencies,
downloading them, and generating GN build targets from them.

To add a new Android dependency, add a Gradle dependency to the `chipDeps`
configuration in `android_deps.gradle`. Then, run

```shell
./set_up_android_deps.py
```

to download dependencies to `artifacts/`, and generate GN build targets in
`BUILD.gn`.

The application code can then depend on
`${chip_root}/third_party/android_deps:my_dep`. The target name `my_dep` will be
generated based on the Gradle dependency name. For example,
"androidx.annotation:annotation:1.1.0" in `build.gradle` becomes the target name
"annotation" in `BUILD.gn`.

Changes to `BUILD.gn` should be committed to the repository, so developers can
track where dependencies lead. `artifacts/` should not, as it contains binaries.
