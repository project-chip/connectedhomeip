"""
Utility functions and constants for the Push AV Server.
"""

import logging
import os.path
import pathlib
import tempfile
from pathlib import Path
from typing import Optional

log = logging.getLogger(__name__)

# Module directory paths
module_dir_path = os.path.dirname(os.path.realpath(__file__))
templates_path = os.path.join(module_dir_path, "templates")
static_path = os.path.join(module_dir_path, "static")

# Valid file extensions for media uploads
# The initialisation segments must have .init extension as per CMAF-Ingest requirements.
# https://dashif.org/Ingest/#interface-2-naming
VALID_EXTENSIONS = ["mpd", "m3u8", "m4s", "init"]


class WorkingDirectory:
    """
    Collection of utilities to add convention to the files used by this program.
    """
    tmp = None

    def __init__(self, directory: Optional[str] = None) -> None:
        if directory is None:
            self.tmp = tempfile.TemporaryDirectory(prefix="TC_PAVS_1_0")
        else:
            d = pathlib.Path(directory)
            d.mkdir(parents=True, exist_ok=True)
            self.directory = d

    def __enter__(self):
        return self

    def __exit__(self, exc, value, tb):
        self.cleanup()

    def cleanup(self):
        """Clean up temporary directory if it exists."""
        if self.tmp:
            self.tmp.cleanup()

    def root_dir(self) -> Path:
        """Get the root directory path."""
        return Path(self.tmp.name) if self.tmp else self.directory

    def path(self, *paths: str) -> Path:
        """Get a path relative to the root directory."""
        return Path(os.path.join(self.root_dir(), *paths))

    def mkdir(self, *paths: str, is_file=False) -> Path:
        """
        Create a directory using the given path rooted in the working directory.
        If a file is provided, the directory up to that file will be created instead.
        Returns the full path.
        """
        p = self.path(*paths)
        # Let's create the parent directories exist
        p2 = pathlib.Path(p)
        if is_file:
            p2 = p2.parent
        p2.mkdir(parents=True, exist_ok=True)
        return p

    def print_tree(self):
        """Print a visual tree representation of the directory structure."""
        # TODO Convert this helper to build a HTML representation for use in the UI
        def tree(dir_path: pathlib.Path, prefix: str = ""):
            """A recursive generator, given a directory Path object
            will yield a visual tree structure line by line
            with each line prefixed by the same characters
            """
            # prefix components:
            space = "    "
            branch = "│   "
            # pointers:
            tee = "├── "
            last = "└── "
            contents = list(dir_path.iterdir())
            # contents each get pointers that are ├── with a final └── :
            pointers = [tee] * (len(contents) - 1) + [last]
            for pointer, path in zip(pointers, contents):
                is_dir = path.is_dir()
                yield prefix + pointer + path.name + ("/" if is_dir else "")
                if path.is_dir():  # extend the prefix and recurse:
                    extension = branch if pointer == tee else space
                    # i.e. space because last, └── , above so no more |
                    yield from tree(path, prefix=prefix + extension)
        root = self.root_dir()
        print(root)
        for line in tree(pathlib.Path(root)):
            print(line)
