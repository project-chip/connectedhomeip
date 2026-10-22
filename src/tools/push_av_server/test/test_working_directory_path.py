"""Tests for WorkingDirectory.path() path resolution.

The file-serving helpers join a caller-supplied segment onto the working
directory. These tests pin the invariant that the resolved path stays within
the directory selected by the fixed prefix, regardless of the segment's shape.
"""

import pytest
from utils import WorkingDirectory


@pytest.fixture
def wd(tmp_path):
    root = tmp_path
    (root / "streams" / "1").mkdir(parents=True)
    (root / "certs" / "server").mkdir(parents=True)
    (root / "streams" / "1" / "seg.m4s").touch()
    (root / "certs" / "server" / "server.key").touch()
    return WorkingDirectory(str(root)), root


def test_relative_segment_resolves_inside_root(wd):
    working, root = wd
    p = working.path("streams", "1", "seg.m4s")
    assert p.is_relative_to(root.resolve())
    assert p.exists()


def test_nested_relative_segment_is_allowed(wd):
    working, root = wd
    p = working.path("streams", "1", "cmaf/example/video.m4s")
    assert p.is_relative_to(root.resolve())


def test_absolute_segment_is_rejected(wd, tmp_path):
    working, _ = wd
    with pytest.raises(ValueError):
        working.path("streams", "1", str(tmp_path.parent / "outside.txt"))


def test_parent_segment_cannot_reach_sibling_dir(wd):
    # streams/<id> must not climb into certs/ even though it stays under root.
    working, _ = wd
    with pytest.raises(ValueError):
        working.path("streams", "1", "../../certs/server/server.key")


def test_parent_segment_cannot_escape_root(wd):
    working, _ = wd
    with pytest.raises(ValueError):
        working.path("streams", "1", "../../../../some/other/path")


def test_windows_drive_relative_segment_is_rejected(wd):
    working, _ = wd
    with pytest.raises(ValueError):
        working.path("streams", "1", "C:certs/server/server.key")


def test_windows_rooted_relative_segment_is_rejected(wd):
    # "\\foo" is neither absolute nor drive-qualified but is still anchored.
    working, _ = wd
    with pytest.raises(ValueError):
        working.path("streams", "1", "\\certs\\server\\server.key")


def test_symlink_escape_is_rejected(wd, tmp_path):
    # A symlink inside root pointing outside must not widen reachable files.
    working, root = wd
    outside_dir = tmp_path.parent / "outside_dir"
    outside_dir.mkdir()
    (outside_dir / "secret").touch()
    link = root / "streams" / "1" / "link"
    link.symlink_to(outside_dir)
    with pytest.raises(ValueError):
        working.path("streams", "1", "link/secret")
