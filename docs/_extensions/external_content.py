"""
External content
################

Copyright (c) 2021 Nordic Semiconductor ASA
SPDX-License-Identifier: Apache-2.0

Introduction
============

This extension allows to import sources from directories out of the Sphinx
source directory. They are copied to the source directory before starting the
build. Note that the copy is *smart*, that is, only updated files are actually
copied. Therefore, incremental builds detect changes correctly and behave as
expected.

Links to external content not included in the generated documentation are
transformed to external links as needed.

Configuration options
=====================

- ``external_content_contents``: A list of external contents. Each entry is
  a tuple with two fields: the external base directory and a file glob pattern.
- ``external_content_link_repositories``: A list of base directories out of scope.
  All links to content within these directories are made external.
- ``external_content_link_extensions``: A list of file extensions in scope of
  the documentation. All links to content without these file extensions are
  made external.
- ``external_content_keep``: A list of file globs (relative to the destination
  directory) that should be kept even if they do not exist in the source
  directory. This option can be useful for auto-generated files in the
  destination directory.
"""

import filecmp
import os
from pathlib import Path
import re
import shutil
import tempfile
from typing import Dict, Any, List, Optional

from sphinx.application import Sphinx

__version__ = "0.1.0"

EXTERNAL_LINK_URL_PREFIX = (
    "https://github.com/project-chip/connectedhomeip/blob/master/"
)


def adjust_includes(
    fname: Path,
    basepath: Path,
    encoding: str,
    link_folders: List[str],
    extensions: List[str],
    dstpath: Optional[Path] = None,
) -> None:
    """Adjust included content paths.

    Args:
        fname: File to be processed.
        basepath: Base path to be used to resolve content location.
        encoding: Sources encoding.
        link_folders: Folders links to which are made external.
        extensions: Filename extensions links to which are not made external.
        dstpath: Destination path for fname if its path is not the actual destination.
    """

    if fname.suffix != ".md":
        return

    dstpath = dstpath or fname.parent

    def _adjust_links(m):
        displayed, fpath = m.groups()

        # ignore absolute paths, section links, hyperlinks and same folder
        if fpath.startswith(("/", "#", "http", "www")) or not "/" in fpath:
            fpath_adj = fpath
        else:
            fpath_adj = Path(os.path.relpath(basepath / fpath, dstpath)).as_posix()

        return f"[{displayed}]({fpath_adj})"

    def _adjust_external(m):
        displayed, folder, target = m.groups()
        return f"[{displayed}]({EXTERNAL_LINK_URL_PREFIX}{folder}/{target})"

    def _adjust_filetype(m):
        displayed, target, extension = m.groups()
        if extension.lower() in extensions or target.startswith("http"):
            return m.group(0)

        return f"[{displayed}]({EXTERNAL_LINK_URL_PREFIX}{target})"

    def _remove_section_links(m):
        (file_link,) = m.groups()
        return file_link + ")"

    rules = [
        # Find any links and adjust the path
        (r"\[([^\[\]]*)\]\s*\((.*)\)", _adjust_links),

        # Find links that lead to an external folder and transform it
        # into an external link.
        (
            r"\[([^\[\]]*)\]\s*\((?:\.\./)*(" + "|".join(link_folders) + r")/(.*)\)",
            _adjust_external,
        ),

        # Find links that lead to a section within another file and
        # remove the section part of the link.
        (r"(\[[^\[\]]*\]\s*\([^)]*\.md)#.*\)", _remove_section_links),

        # Find links that lead to a non-presentable filetype and transform
        # it into an external link.
        (
            r"\[([^\[\]]*)\]\s*\((?:\.\./)*((?:[^()]+?/)*[^.()]+?(\.[^)/]+))\)",
            _adjust_filetype,
        ),
    ]

    with open(fname, "r+", encoding=encoding) as f:
        content = f.read()
        modified = False

        for pattern, sub_func in rules:
            content, changes_made = re.subn(pattern, sub_func, content)
            modified = modified or changes_made

        if modified:
            f.seek(0)
            f.write(content)
            f.truncate()


def sync_contents(app: Sphinx) -> None:
    """Synhronize external contents.

    Args:
        app: Sphinx application instance.
    """

    srcdir = Path(app.srcdir).resolve()
    to_copy = []
    to_delete = set(f for f in srcdir.glob("**/*") if not f.is_dir())
    to_keep = set(
        f
        for k in app.config.external_content_keep
        for f in srcdir.glob(k)
        if not f.is_dir()
    )

    for content in app.config.external_content_contents:
        prefix_src, glob = content
        for src in prefix_src.glob(glob):
            if src.is_dir():
                to_copy.extend(
                    [(f, prefix_src) for f in src.glob("**/*") if not f.is_dir()]
                )
            else:
                to_copy.append((src, prefix_src))

    for entry in to_copy:
        src, prefix_src = entry
        dst = (srcdir / src.relative_to(prefix_src)).resolve()

        if dst in to_delete:
            to_delete.remove(dst)

        if not dst.parent.exists():
            dst.parent.mkdir(parents=True)

        # just copy if it does not exist
        if not dst.exists():
            shutil.copy(src, dst)
            adjust_includes(
                dst,
                src.parent,
                app.config.source_encoding,
                app.config.external_content_link_repositories,
                app.config.external_content_link_extensions,
            )
        # if origin file is modified only copy if different
        elif src.stat().st_mtime > dst.stat().st_mtime:
            with tempfile.TemporaryDirectory() as td:
                # adjust origin includes before comparing
                src_adjusted = Path(td) / src.name
                shutil.copy(src, src_adjusted)
                adjust_includes(
                    src_adjusted,
                    src.parent,
                    app.config.source_encoding,
                    app.config.external_content_link_repositories,
                    app.config.external_content_link_extensions,
                    dstpath=dst.parent,
                )

                if not filecmp.cmp(src_adjusted, dst):
                    dst.unlink()
                    shutil.move(os.fspath(src_adjusted), os.fspath(dst))

    # remove any previously copied file not present in the origin folder,
    # excepting those marked to be kept.
    for file in to_delete - to_keep:
        file.unlink()


def setup(app: Sphinx) -> Dict[str, Any]:
    app.add_config_value("external_content_contents", [], "env")
    app.add_config_value("external_content_keep", [], "")
    app.add_config_value("external_content_link_repositories", [], "env")
    app.add_config_value("external_content_link_extensions", [], "env")

    app.connect("builder-inited", sync_contents)

    return {
        "version": __version__,
        "parallel_read_safe": True,
        "parallel_write_safe": True,
    }
