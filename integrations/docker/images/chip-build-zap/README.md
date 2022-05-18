# ZAP

-   This Dockerfile builds https://github.com/project-chip/zap.git, which gets
    copied into `chip-build-vscode`
-   ZAP is also a submodule in `third_party/zap/repo`. You can check the commit
    of ZAP used in the submodule by running
    `git ls-tree master third_party/zap/repo | awk '{print $3}'` from the
    repository root.
