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
    assert str(p).startswith(str(root.resolve()))
    assert p.exists()


def test_nested_relative_segment_is_allowed(wd):
    working, root = wd
    p = working.path("streams", "1", "cmaf/example/video.m4s")
    assert str(p).startswith(str(root.resolve()))


def test_absolute_segment_is_rejected(wd, tmp_path):
    working, _ = wd
    outside = tmp_path.parent / "outside.txt"
    with pytest.raises(ValueError):
        working.path("streams", "1", str(outside))


def test_parent_segment_cannot_reach_sibling_dir(wd):
    # streams/<id> must not climb into certs/ even though it stays under root.
    working, _ = wd
    with pytest.raises(ValueError):
        working.path("streams", "1", "../../certs/server/server.key")


def test_parent_segment_cannot_escape_root(wd):
    working, _ = wd
    with pytest.raises(ValueError):
        working.path("streams", "1", "../../../../some/other/path")
