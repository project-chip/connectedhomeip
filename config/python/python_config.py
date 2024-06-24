#!/usr/bin/env python3

import os
import json
import subprocess
import shutil


class Config:
    def __init__(self):
        cipd_install_dir = os.getenv("PW_PYTHON_CIPD_INSTALL_DIR")
        self.python_exe = os.path.join(cipd_install_dir, "bin", "python3")
        if not os.path.exists(self.python_exe):
            self.python_exe = shutil.which("python3")
            if not self.python_exe:
                raise FileNotFoundError("Python3 not found")

    def get_version(self):
        out = subprocess.check_output(
            [self.python_exe, "-c", "import sys; print(sys.version_info.major, sys.version_info.minor)"], text=True
        )
        return out.strip().split(" ")


if __name__ == "__main__":
    config = Config()
    version = config.get_version()
    print(
        json.dumps(
            {
                "version": {
                    "major": version[0],
                    "minor": version[1],
                }
            }
        )
    )
