"""Tests for WorkingDirectory.path() path resolution.

The file-serving helpers join a caller-supplied segment onto the working
directory. These tests pin the invariant that the resolved path always stays
within the working directory, regardless of the segment's shape.
"""
import os
import tempfile

import pytest
from utils import WorkingDirectory


def _wd():
    root = tempfile.mkdtemp(prefix="pavs_test_")
    wd = WorkingDirectory(root)
    os.makedirs(os.path.join(root, "streams", "1"), exist_ok=True)
    os.makedirs(os.path.join(root, "certs", "server"), exist_ok=True)
    open(os.path.join(root, "streams", "1", "seg.m4s"), "w").close()
    open(os.path.join(root, "certs", "server", "server.key"), "w").close()
    return wd, root


def test_relative_segment_resolves_inside_root():
    wd, root = _wd()
    p = wd.path("streams", "1", "seg.m4s")
    assert str(p).startswith(os.path.realpath(root))
    assert p.exists()


def test_absolute_segment_stays_within_root():
    # An absolute segment is treated as relative, so it maps to a path inside
    # root rather than resolving to the absolute location on disk.
    wd, root = _wd()
    outside = tempfile.NamedTemporaryFile(delete=False)
    outside.close()
    p = wd.path("streams", "1", outside.name)
    assert str(p).startswith(os.path.realpath(root))
    assert not p.exists()


def test_absolute_segment_does_not_reach_other_root_subdir():
    wd, root = _wd()
    key = os.path.join(root, "certs", "server", "server.key")
    p = wd.path("streams", "1", key)
    assert str(p).startswith(os.path.realpath(root))
    assert not p.exists()


def test_parent_segment_stays_within_root():
    wd, _ = _wd()
    with pytest.raises(ValueError):
        wd.path("streams", "1", "../../../../some/other/path")
