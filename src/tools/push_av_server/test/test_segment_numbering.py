"""Test cases for segment number validation across ingest interfaces."""
import pytest
from models import Session, Stream, SupportedIngestInterface, Track, TrackState
from validation import MatterCMAFUploadValidator

SEGMENT_PATH = "session_1/video/segment_{}"
INIT_PATH = "session_1/video/video"
HLS_HEADERS = {"X-EXTINF-duration": "2.0"}


def _upload_segments(validator: MatterCMAFUploadValidator, stream: Stream, session: Session,
                     numbers: list[int], headers: dict | None = None) -> list[list[str]]:
    """Upload m4s segments in order and return the errors of each upload."""
    return [validator.validate_upload(stream, session, SEGMENT_PATH.format(n), "m4s", b"", headers or {})[0]
            for n in numbers]


def _ready_session(interface: SupportedIngestInterface) -> tuple[MatterCMAFUploadValidator, Stream, Session]:
    """Create a stream and a session whose video track is ready to receive segments."""
    validator = MatterCMAFUploadValidator()
    stream = Stream(id=1, interface=interface)
    session = stream.new_session()
    if interface == SupportedIngestInterface.hls:
        session.tracks["video"] = Track(name="video", state=TrackState.INITIAL_PLAYLIST_UPLOADED)
    errors, _ = validator.validate_upload(stream, session, INIT_PATH, "init", b"", {})
    assert errors == []
    return validator, stream, session


class TestCmafIngestSegmentNumbering:
    """Segment numbers on cmaf-ingest streams start at 1001 and increase by 1."""

    def test_consecutive_segments_accepted(self):
        """Consecutive segments from 1001 are accepted and counted."""
        validator, stream, session = _ready_session(SupportedIngestInterface.cmaf)

        results = _upload_segments(validator, stream, session, [1001, 1002, 1003])

        assert results == [[], [], []]
        assert session.tracks["video"].segment_count == 3

    def test_tracks_are_counted_independently(self):
        """Each track starts its own numbering at 1001."""
        validator, stream, session = _ready_session(SupportedIngestInterface.cmaf)
        errors, _ = validator.validate_upload(stream, session, "session_1/audio/audio", "init", b"", {})
        assert errors == []

        assert _upload_segments(validator, stream, session, [1001, 1002]) == [[], []]
        errors, _ = validator.validate_upload(stream, session, "session_1/audio/segment_1001", "m4s", b"", {})
        assert errors == []

    @pytest.mark.parametrize("second,expected", [(1003, 1002), (1001, 1002)])
    def test_gap_and_duplicate_rejected(self, second, expected):
        """A skipped or repeated segment number is rejected, and the rejected upload still advances the count."""
        validator, stream, session = _ready_session(SupportedIngestInterface.cmaf)

        results = _upload_segments(validator, stream, session, [1001, second])

        assert results[0] == []
        assert any(f"expected segment {expected}, received segment {second}" in e for e in results[1])
        assert session.tracks["video"].segment_count == 2


class TestDashAndHlsSegmentNumbering:
    """Existing DASH and HLS numbering behavior is unchanged."""

    @pytest.mark.parametrize("interface", [SupportedIngestInterface.dash, SupportedIngestInterface.hls])
    def test_consecutive_segments_accepted(self, interface):
        """Consecutive segments from 1001 are accepted and counted."""
        validator, stream, session = _ready_session(interface)
        headers = HLS_HEADERS if interface == SupportedIngestInterface.hls else None

        results = _upload_segments(validator, stream, session, [1001, 1002, 1003], headers)

        assert results == [[], [], []]
        assert session.tracks["video"].segment_count == 3

    @pytest.mark.parametrize("interface", [SupportedIngestInterface.dash, SupportedIngestInterface.hls])
    def test_gap_rejected(self, interface):
        """A skipped segment number is rejected, and the rejected upload still advances the count."""
        validator, stream, session = _ready_session(interface)
        headers = HLS_HEADERS if interface == SupportedIngestInterface.hls else None

        results = _upload_segments(validator, stream, session, [1001, 1003], headers)

        assert results[0] == []
        assert any("expected segment 1002, received segment 1003" in e for e in results[1])
        assert session.tracks["video"].segment_count == 2


def _api_upload(validator: MatterCMAFUploadValidator, stream: Stream, path: str, ext: str,
                body: bytes = b"") -> list[str]:
    """Upload a file the way the API does, starting from the stream's last in-progress session."""
    errors, _ = validator.validate_upload(stream, stream.last_in_progress_session(), path, ext, body, {})
    return errors


class TestCmafIngestWithoutManifest:
    """A cmaf-ingest stream has no manifest, so its sessions come from the upload paths."""

    def test_two_sessions_with_two_tracks_accepted(self):
        """Init and segments for video and audio are accepted in session_1 and then session_2."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.cmaf)

        for session_number in (1, 2):
            for track in ("video", "audio"):
                assert _api_upload(validator, stream, f"session_{session_number}/{track}/{track}", "init") == []
            for segment in (1001, 1002, 1003):
                for track in ("video", "audio"):
                    path = f"session_{session_number}/{track}/segment_{segment}"
                    assert _api_upload(validator, stream, path, "m4s") == []

        assert [s.id for s in stream.sessions] == [1, 2]
        assert stream.sessions[1].tracks["video"].segment_count == 3

    def test_segment_before_init_rejected(self):
        """A media segment is rejected when its track has no init segment yet."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.cmaf)

        errors = _api_upload(validator, stream, "session_1/video/segment_1001", "m4s")

        assert any("uploaded before init segment" in e for e in errors)

    def test_first_session_must_be_1(self):
        """The first upload of a stream is rejected when it is not in session_1."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.cmaf)

        errors = _api_upload(validator, stream, "session_7/video/video", "init")

        assert any("expected session 1, received session 7" in e for e in errors)
        assert stream.sessions == []

    def test_skipped_and_stale_session_rejected(self):
        """Session numbers must follow the current session by 1, and older ones are rejected."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.cmaf)
        assert _api_upload(validator, stream, "session_1/video/video", "init") == []

        errors = _api_upload(validator, stream, "session_3/video/video", "init")
        assert any("expected session 1, received session 3" in e for e in errors)

        assert _api_upload(validator, stream, "session_2/video/video", "init") == []
        errors = _api_upload(validator, stream, "session_1/video/segment_1001", "m4s")
        assert any("expected session 2, received session 1" in e for e in errors)
        assert len(stream.sessions) == 2

    def test_gap_rejected(self):
        """A skipped segment number is rejected without a manifest-created session."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.cmaf)
        assert _api_upload(validator, stream, "session_1/video/video", "init") == []
        assert _api_upload(validator, stream, "session_1/video/segment_1001", "m4s") == []

        errors = _api_upload(validator, stream, "session_1/video/segment_1003", "m4s")

        assert any("expected segment 1002, received segment 1003" in e for e in errors)

    def test_duplicate_init_rejected(self):
        """A second init upload for the same track in the same session is rejected."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.cmaf)
        assert _api_upload(validator, stream, "session_1/video/video", "init") == []

        errors = _api_upload(validator, stream, "session_1/video/video", "init")

        assert any("already uploaded, duplicate upload not allowed" in e for e in errors)
        assert len(stream.sessions) == 1

    def test_retry_in_current_session_keeps_session(self):
        """A repeated upload in the current session does not start a new session."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.cmaf)
        _api_upload(validator, stream, "session_1/video/video", "init")
        _api_upload(validator, stream, "session_1/video/segment_1001", "m4s")

        _api_upload(validator, stream, "session_1/video/segment_1001", "m4s")

        assert len(stream.sessions) == 1

    @pytest.mark.parametrize("ext,path", [("mpd", "session_1/index"), ("m3u8", "session_1/index"),
                                          ("m3u8", "session_1/video/index")])
    def test_manifest_rejected_without_session(self, ext, path):
        """A manifest upload is rejected and does not create a session."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.cmaf)

        errors = _api_upload(validator, stream, path, ext, b'<MPD type="static"/>' if ext == "mpd" else b"#EXTM3U\n")

        assert errors == ["Unsupported manifest object extension"]
        assert stream.sessions == []

    def test_stale_session_upload_changes_no_state(self):
        """An upload for an older session is rejected and leaves the current session's tracks untouched."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.cmaf)
        _api_upload(validator, stream, "session_1/video/video", "init")
        _api_upload(validator, stream, "session_1/video/segment_1001", "m4s")
        _api_upload(validator, stream, "session_2/video/video", "init")
        session_2 = stream.sessions[1]
        uploads_before = len(session_2.uploaded_segments)

        errors = _api_upload(validator, stream, "session_1/video/segment_1002", "m4s")
        assert any("expected session 2, received session 1" in e for e in errors)
        errors = _api_upload(validator, stream, "session_1/audio/audio", "init")
        assert any("expected session 2, received session 1" in e for e in errors)

        assert session_2.tracks["video"].segment_count == 0
        assert session_2.tracks["video"].state == TrackState.INIT_UPLOADED
        assert "audio" not in session_2.tracks
        assert len(session_2.uploaded_segments) == uploads_before
        assert _api_upload(validator, stream, "session_2/video/segment_1001", "m4s") == []
