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

    def test_gap_rejected(self):
        """A skipped segment number is rejected."""
        validator, stream, session = _ready_session(SupportedIngestInterface.cmaf)

        results = _upload_segments(validator, stream, session, [1001, 1003])

        assert results[0] == []
        assert any("expected segment 1002, received segment 1003" in e for e in results[1])

    def test_duplicate_rejected(self):
        """A repeated segment number is rejected."""
        validator, stream, session = _ready_session(SupportedIngestInterface.cmaf)

        results = _upload_segments(validator, stream, session, [1001, 1001])

        assert results[0] == []
        assert any("expected segment 1002, received segment 1001" in e for e in results[1])

    def test_rejected_segment_still_advances_count(self):
        """A rejected segment advances the count, as on DASH and HLS."""
        validator, stream, session = _ready_session(SupportedIngestInterface.cmaf)

        _upload_segments(validator, stream, session, [1001, 1003])

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
        """A skipped segment number is rejected."""
        validator, stream, session = _ready_session(interface)
        headers = HLS_HEADERS if interface == SupportedIngestInterface.hls else None

        results = _upload_segments(validator, stream, session, [1001, 1003], headers)

        assert results[0] == []
        assert any("expected segment 1002, received segment 1003" in e for e in results[1])

    @pytest.mark.parametrize("interface", [SupportedIngestInterface.dash, SupportedIngestInterface.hls])
    def test_rejected_segment_still_advances_count(self, interface):
        """A rejected segment still advances the count."""
        validator, stream, session = _ready_session(interface)
        headers = HLS_HEADERS if interface == SupportedIngestInterface.hls else None

        _upload_segments(validator, stream, session, [1001, 1003], headers)

        assert session.tracks["video"].segment_count == 2
