"""Test cases for HLS validation in the MatterCMAFUploadValidator."""
from models import Session, Stream, SupportedIngestInterface, Track, TrackState
from validation import MatterCMAFUploadValidator


class TestHLSMultiVariantPlaylist:
    """Test cases for HLS multi-variant playlist validation."""

    def test_valid_multi_variant_playlist_first_upload(self):
        """Test uploading a valid multi-variant playlist as the first upload."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.hls)

        body = """#EXTM3U
#EXT-X-VERSION:6
#EXT-X-MEDIA:TYPE=VIDEO,GROUP-ID="video",NAME="1080p",BANDWIDTH=5000000,RESOLUTION=1920x1080
#EXT-X-STREAM-INF:BANDWIDTH=5000000,RESOLUTION=1920x1080,CODECS="avc1.640028,mp4a.40.2"
stream_1080p.m3u8
"""
        errors, session = validator._validate_m3u8_upload(
            stream, None, "session_1/index", body.encode('utf-8')
        )

        assert len(errors) == 0
        assert session is not None
        assert session.id == 1
        assert session.hls_expected_track_count == 2
        assert len(session.uploaded_manifests) == 1

    def test_multi_variant_playlist_after_segments_error(self):
        """Test uploading multi-variant playlist after segments have been uploaded."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.hls)
        session = Session(id=1)
        session.uploaded_segments.append(("session_1/video/segment_1.m4s", "session_1/video/segment_1.m4s.crt"))

        body = """#EXTM3U
#EXT-X-VERSION:6
#EXT-X-STREAM-INF:BANDWIDTH=5000000
stream.m3u8
"""
        errors, _ = validator._validate_m3u8_upload(
            stream, session, "session_1/index", body.encode('utf-8')
        )

        assert any("Multi-variant playlist must be uploaded first" in error for error in errors)

    def test_multi_variant_playlist_no_tracks_error(self):
        """Test uploading multi-variant playlist with no track definitions."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.hls)

        body = """#EXTM3U
#EXT-X-VERSION:6
"""
        errors, session = validator._validate_m3u8_upload(
            stream, None, "session_1/index", body.encode('utf-8')
        )

        assert any("must contain at least one EXT-X-MEDIA or EXT-X-STREAM-INF" in error for error in errors)

    def test_invalid_path_for_multi_variant_playlist(self):
        """Test uploading multi-variant playlist with invalid path."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.hls)

        body = """#EXTM3U
#EXT-X-VERSION:6
#EXT-X-STREAM-INF:BANDWIDTH=5000000
stream.m3u8
"""
        errors, _ = validator._validate_m3u8_upload(
            stream, None, "invalid/path/index", body.encode('utf-8')
        )

        assert any("HLS manifest must be uploaded as session_X/index.m3u8" in error for error in errors)


class TestHLSMediaPlaylist:
    """Test cases for HLS media playlist validation."""

    def test_initial_media_playlist_valid(self):
        """Test uploading a valid initial media playlist."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.hls)
        session = Session(id=1, hls_expected_track_count=1)

        body = """#EXTM3U
#EXT-X-VERSION:6
#EXT-X-TARGETDURATION:10
"""
        errors, _ = validator._validate_m3u8_upload(
            stream, session, "session_1/video/index", body.encode('utf-8')
        )

        assert len(errors) == 0
        assert "video" in session.tracks
        assert session.tracks["video"].state == TrackState.INITIAL_PLAYLIST_UPLOADED

    def test_initial_media_playlist_no_session_error(self):
        """Test uploading initial media playlist without multi-variant playlist."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.hls)

        body = """#EXTM3U
#EXT-X-VERSION:6
#EXT-X-TARGETDURATION:10
"""
        errors, _ = validator._validate_m3u8_upload(
            stream, None, "session_1/video/index", body.encode('utf-8')
        )

        assert any("Multi-variant playlist must be uploaded before media playlists" in error for error in errors)

    def test_initial_media_playlist_with_playlist_type_error(self):
        """Test uploading initial media playlist with PLAYLIST-TYPE tag."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.hls)
        session = Session(id=1, hls_expected_track_count=1)

        body = """#EXTM3U
#EXT-X-VERSION:6
#EXT-X-TARGETDURATION:10
#EXT-X-PLAYLIST-TYPE:VOD
"""
        errors, _ = validator._validate_m3u8_upload(
            stream, session, "session_1/video/index", body.encode('utf-8')
        )

        assert any("Initial media playlist must NOT contain #EXT-X-PLAYLIST-TYPE" in error for error in errors)

    def test_initial_media_playlist_with_endlist_error(self):
        """Test uploading initial media playlist with ENDLIST tag."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.hls)
        session = Session(id=1, hls_expected_track_count=1)

        body = """#EXTM3U
#EXT-X-VERSION:6
#EXT-X-TARGETDURATION:10
#EXT-X-ENDLIST
"""
        errors, _ = validator._validate_m3u8_upload(
            stream, session, "session_1/video/index", body.encode('utf-8')
        )

        assert any("Initial media playlist must NOT contain #EXT-X-ENDLIST" in error for error in errors)

    def test_initial_media_playlist_with_extinf_error(self):
        """Test uploading initial media playlist with EXTINF tags."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.hls)
        session = Session(id=1, hls_expected_track_count=1)

        body = """#EXTM3U
#EXT-X-VERSION:6
#EXT-X-TARGETDURATION:10
#EXTINF:10.0,
segment_1.m4s
"""
        errors, _ = validator._validate_m3u8_upload(
            stream, session, "session_1/video/index", body.encode('utf-8')
        )

        assert any("Initial media playlist must NOT contain #EXTINF tags" in error for error in errors)

    def test_initial_media_playlist_with_segments_error(self):
        """Test uploading initial media playlist with segment lines."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.hls)
        session = Session(id=1, hls_expected_track_count=1)

        body = """#EXTM3U
#EXT-X-VERSION:6
#EXT-X-TARGETDURATION:10
segment_1.m4s
"""
        errors, _ = validator._validate_m3u8_upload(
            stream, session, "session_1/video/index", body.encode('utf-8')
        )

        assert any("Initial media playlist must NOT contain segment lines" in error for error in errors)

    def test_final_media_playlist_valid(self):
        """Test uploading a valid final media playlist."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.hls)
        session = Session(id=1, hls_expected_track_count=1, hls_completed_tracks=0)
        session.tracks["video"] = Track(name="video", state=TrackState.SEGMENTS_UPLOADING, segment_count=5)

        body = """#EXTM3U
#EXT-X-VERSION:6
#EXT-X-TARGETDURATION:10
#EXT-X-PLAYLIST-TYPE:VOD
#EXTINF:10.0,
segment_1.m4s
#EXTINF:10.0,
segment_2.m4s
#EXT-X-ENDLIST
"""
        errors, _ = validator._validate_m3u8_upload(
            stream, session, "session_1/video/index", body.encode('utf-8')
        )

        assert len(errors) == 0
        assert session.tracks["video"].state == TrackState.COMPLETED
        assert session.hls_completed_tracks == 1
        assert session.complete is True

    def test_final_media_playlist_without_playlist_type_error(self):
        """Test uploading final media playlist without PLAYLIST-TYPE tag."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.hls)
        session = Session(id=1, hls_expected_track_count=1, hls_completed_tracks=0)
        session.tracks["video"] = Track(name="video", state=TrackState.SEGMENTS_UPLOADING, segment_count=5)

        body = """#EXTM3U
#EXT-X-VERSION:6
#EXT-X-TARGETDURATION:10
#EXTINF:10.0,
segment_1.m4s
#EXT-X-ENDLIST
"""
        errors, _ = validator._validate_m3u8_upload(
            stream, session, "session_1/video/index", body.encode('utf-8')
        )

        assert any("Final media playlist must contain #EXT-X-PLAYLIST-TYPE:VOD" in error for error in errors)

    def test_final_media_playlist_not_vod_error(self):
        """Test uploading final media playlist without VOD type."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.hls)
        session = Session(id=1, hls_expected_track_count=1, hls_completed_tracks=0)
        session.tracks["video"] = Track(name="video", state=TrackState.SEGMENTS_UPLOADING, segment_count=5)

        body = """#EXTM3U
#EXT-X-VERSION:6
#EXT-X-TARGETDURATION:10
#EXT-X-PLAYLIST-TYPE:EVENT
#EXTINF:10.0,
segment_1.m4s
#EXT-X-ENDLIST
"""
        errors, _ = validator._validate_m3u8_upload(
            stream, session, "session_1/video/index", body.encode('utf-8')
        )

        assert any("Final media playlist must have #EXT-X-PLAYLIST-TYPE:VOD" in error for error in errors)

    def test_final_media_playlist_no_segments_error(self):
        """Test uploading initial media playlist with endlist."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.hls)
        session = Session(id=1, hls_expected_track_count=1, hls_completed_tracks=0)
        session.tracks["video"] = Track(name="video", state=TrackState.NOT_STARTED)

        body = """#EXTM3U
#EXT-X-VERSION:6
#EXT-X-TARGETDURATION:10
#EXT-X-PLAYLIST-TYPE:VOD
#EXTINF:10.0,
segment_1.m4s
#EXT-X-ENDLIST
"""
        errors, _ = validator._validate_m3u8_upload(
            stream, session, "session_1/video/index", body.encode('utf-8')
        )

        assert any("Initial media playlist must NOT contain #EXT-X-ENDLIST" in error for error in errors)

    def test_final_media_playlist_before_init_segment_error(self):
        """Test uploading final media playlist before init segment."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.hls)
        session = Session(id=1, hls_expected_track_count=1, hls_completed_tracks=0)
        session.tracks["video"] = Track(name="video", state=TrackState.INITIAL_PLAYLIST_UPLOADED)

        body = """#EXTM3U
#EXT-X-VERSION:6
#EXT-X-TARGETDURATION:10
#EXT-X-PLAYLIST-TYPE:VOD
#EXTINF:10.0,
segment_1.m4s
#EXT-X-ENDLIST
"""
        errors, _ = validator._validate_m3u8_upload(
            stream, session, "session_1/video/index", body.encode('utf-8')
        )

        assert any("Final media playlist uploaded for track 'video' before init segment and segments were uploaded" in error for error in errors)


class TestHLSSegmentValidation:
    """Test cases for HLS segment validation."""

    def test_init_segment_valid(self):
        """Test uploading a valid init segment."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.hls)
        session = Session(id=1)
        session.tracks["video"] = Track(name="video", state=TrackState.INITIAL_PLAYLIST_UPLOADED)

        errors, _ = validator._validate_segment_upload(
            stream, session, "session_1/video/init", "init", {}
        )

        assert len(errors) == 0
        assert session.tracks["video"].state == TrackState.INIT_UPLOADED
        assert len(session.uploaded_segments) == 1

    def test_init_segment_no_session_error(self):
        """Test uploading init segment without session."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.hls)

        errors, _ = validator._validate_segment_upload(
            stream, None, "session_1/video/init", "init", {}
        )

        assert any("segment uploaded before multi-variant playlist" in error for error in errors)

    def test_init_segment_before_initial_playlist_error(self):
        """Test uploading init segment before initial media playlist."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.hls)
        session = Session(id=1)
        session.tracks["video"] = Track(name="video", state=TrackState.NOT_STARTED)

        errors, _ = validator._validate_segment_upload(
            stream, session, "session_1/video/init", "init", {}
        )

        assert any("Init segment for track 'video' uploaded before initial media playlist" in error for error in errors)

    def test_init_segment_duplicate_error(self):
        """Test uploading duplicate init segment."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.hls)
        session = Session(id=1)
        session.tracks["video"] = Track(name="video", state=TrackState.INIT_UPLOADED)

        errors, _ = validator._validate_segment_upload(
            stream, session, "session_1/video/init", "init", {}
        )

        assert any("Init segment for track 'video' already uploaded" in error for error in errors)

    def test_init_segment_after_segments_error(self):
        """Test uploading init segment after segments started."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.hls)
        session = Session(id=1)
        session.tracks["video"] = Track(name="video", state=TrackState.SEGMENTS_UPLOADING, segment_count=3)

        errors, _ = validator._validate_segment_upload(
            stream, session, "session_1/video/init", "init", {}
        )

        assert any("Init segment for track 'video' uploaded after segments started" in error for error in errors)

    def test_init_segment_after_completed_error(self):
        """Test uploading init segment after track completion."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.hls)
        session = Session(id=1)
        session.tracks["video"] = Track(name="video", state=TrackState.COMPLETED)

        errors, _ = validator._validate_segment_upload(
            stream, session, "session_1/video/init", "init", {}
        )

        assert any("Init segment for track 'video' uploaded after track was completed" in error for error in errors)

    def test_media_segment_valid(self):
        """Test uploading a valid media segment."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.hls)
        session = Session(id=1)
        session.tracks["video"] = Track(name="video", state=TrackState.INIT_UPLOADED)

        errors, _ = validator._validate_segment_upload(
            stream, session, "session_1/video/segment_1", "m4s", {"X-EXTINF-duration": "10.5"}
        )

        assert len(errors) == 0
        assert session.tracks["video"].state == TrackState.SEGMENTS_UPLOADING
        assert session.tracks["video"].segment_count == 1
        assert len(session.uploaded_segments) == 1

    def test_media_segment_no_session_error(self):
        """Test uploading media segment without session."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.hls)

        errors, _ = validator._validate_segment_upload(
            stream, None, "session_1/video/segment_1", "m4s", {}
        )

        assert any("segment uploaded before multi-variant playlist" in error for error in errors)

    def test_media_segment_before_initial_playlist_error(self):
        """Test uploading media segment before initial media playlist."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.hls)
        session = Session(id=1)
        session.tracks["video"] = Track(name="video", state=TrackState.NOT_STARTED)

        errors, _ = validator._validate_segment_upload(
            stream, session, "session_1/video/segment_1", "m4s", {"X-EXTINF-duration": "10.5"}
        )

        assert any("Media segment for track 'video' uploaded before initial media playlist" in error for error in errors)

    def test_media_segment_before_init_error(self):
        """Test uploading media segment before init segment."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.hls)
        session = Session(id=1)
        session.tracks["video"] = Track(name="video", state=TrackState.INITIAL_PLAYLIST_UPLOADED)

        errors, _ = validator._validate_segment_upload(
            stream, session, "session_1/video/segment_1", "m4s", {"X-EXTINF-duration": "10.5"}
        )

        assert any("Media segment for track 'video' uploaded before init segment" in error for error in errors)

    def test_media_segment_after_completed_error(self):
        """Test uploading media segment after track completion."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.hls)
        session = Session(id=1)
        session.tracks["video"] = Track(name="video", state=TrackState.COMPLETED)

        errors, _ = validator._validate_segment_upload(
            stream, session, "session_1/video/segment_1", "m4s", {"X-EXTINF-duration": "10.5"}
        )

        assert any("Media segment for track 'video' uploaded after track was completed" in error for error in errors)

    def test_media_segment_missing_duration_header_error(self):
        """Test uploading media segment without duration header."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.hls)
        session = Session(id=1)
        session.tracks["video"] = Track(name="video", state=TrackState.INIT_UPLOADED)

        errors, _ = validator._validate_segment_upload(
            stream, session, "session_1/video/segment_1", "m4s", {}
        )

        assert any("HLS media segments must include X-EXTINF-duration header" in error for error in errors)

    def test_media_segment_invalid_duration_header_error(self):
        """Test uploading media segment with invalid duration header."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.hls)
        session = Session(id=1)
        session.tracks["video"] = Track(name="video", state=TrackState.INIT_UPLOADED)

        errors, _ = validator._validate_segment_upload(
            stream, session, "session_1/video/segment_1", "m4s", {"X-EXTINF-duration": "invalid"}
        )

        assert any("X-EXTINF-duration header must be a valid decimal floating-point value" in error for error in errors)

    def test_invalid_segment_path_error(self):
        """Test uploading segment with invalid path."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.hls)
        session = Session(id=1)
        session.tracks["video"] = Track(name="video", state=TrackState.INIT_UPLOADED)

        errors, _ = validator._validate_segment_upload(
            stream, session, "invalid/path/segment_1", "m4s", {"X-EXTINF-duration": "10.5"}
        )

        assert any("Path does not adhere to Matter's path format" in error for error in errors)


class TestHLSTrackNameValidation:
    """Test cases for HLS track name validation."""

    def test_track_name_match_media_playlist(self):
        """Test track name validation in media playlist."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.hls, track_name="video")
        session = Session(id=1, hls_expected_track_count=1)

        body = """#EXTM3U
#EXT-X-VERSION:6
#EXT-X-TARGETDURATION:10
"""
        errors, _ = validator._validate_m3u8_upload(
            stream, session, "session_1/video/index", body.encode('utf-8')
        )

        assert len(errors) == 0

    def test_track_name_mismatch_media_playlist_error(self):
        """Test track name mismatch in media playlist."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.hls, track_name="audio")
        session = Session(id=1, hls_expected_track_count=1)

        body = """#EXTM3U
#EXT-X-VERSION:6
#EXT-X-TARGETDURATION:10
"""
        errors, _ = validator._validate_m3u8_upload(
            stream, session, "session_1/video/index", body.encode('utf-8')
        )

        assert any("Track name mismatch" in error for error in errors)

    def test_track_name_match_segment(self):
        """Test track name validation in segment upload."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.hls, track_name="video")
        session = Session(id=1)
        session.tracks["video"] = Track(name="video", state=TrackState.INITIAL_PLAYLIST_UPLOADED)

        errors, _ = validator._validate_segment_upload(
            stream, session, "session_1/video/init", "init", {}
        )

        assert len(errors) == 0

    def test_track_name_mismatch_segment_error(self):
        """Test track name mismatch in segment upload."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.hls, track_name="audio")
        session = Session(id=1)
        session.tracks["video"] = Track(name="video", state=TrackState.INITIAL_PLAYLIST_UPLOADED)

        errors, _ = validator._validate_segment_upload(
            stream, session, "session_1/video/init", "init", {}
        )

        assert any("Track name mismatch" in error for error in errors)


class TestHLSCompleteSession:
    """Test cases for complete HLS session workflow."""

    def test_complete_hls_workflow(self):
        """Test a complete HLS upload workflow."""
        validator = MatterCMAFUploadValidator()
        stream = Stream(id=1, interface=SupportedIngestInterface.hls, track_name="video")

        # 1. Upload multi-variant playlist
        mv_body = """#EXTM3U
#EXT-X-VERSION:6
#EXT-X-MEDIA:TYPE=VIDEO,GROUP-ID="video",NAME="1080p"
video/index.m3u8"""

        errors, session = validator._validate_m3u8_upload(
            stream, None, "session_1/index", mv_body.encode('utf-8')
        )

        assert len(errors) == 0
        assert session.hls_expected_track_count == 1

        # 2. Upload initial media playlist
        initial_body = """#EXTM3U
#EXT-X-VERSION:6
#EXT-X-TARGETDURATION:10"""

        errors, session = validator._validate_m3u8_upload(
            stream, session, "session_1/video/index", initial_body.encode('utf-8')
        )

        assert len(errors) == 0
        assert session.tracks["video"].state == TrackState.INITIAL_PLAYLIST_UPLOADED

        # 3. Upload init segment
        errors, session = validator._validate_segment_upload(
            stream, session, "session_1/video/init", "init", {}
        )

        assert len(errors) == 0
        assert session.tracks["video"].state == TrackState.INIT_UPLOADED

        # 4. Upload media segments
        for i in range(1, 4):
            errors, session = validator._validate_segment_upload(
                stream, session, f"session_1/video/segment_{i}", "m4s", {"X-EXTINF-duration": "10.0"}
            )
            assert len(errors) == 0

        assert session.tracks["video"].segment_count == 3
        assert session.tracks["video"].state == TrackState.SEGMENTS_UPLOADING

        # 5. Upload final media playlist
        final_body = """#EXTM3U
#EXT-X-VERSION:6
#EXT-X-TARGETDURATION:10
#EXT-X-PLAYLIST-TYPE:VOD
#EXTINF:10.0,
segment_1.m4s
#EXTINF:10.0,
segment_2.m4s
#EXTINF:10.0,
segment_3.m4s
#EXT-X-ENDLIST"""

        errors, session = validator._validate_m3u8_upload(
            stream, session, "session_1/video/index", final_body.encode('utf-8')
        )

        assert len(errors) == 0
        assert session.tracks["video"].state == TrackState.COMPLETED
        assert session.hls_completed_tracks == 1
        assert session.complete is True
        assert len(session.uploaded_manifests) == 3
        assert len(session.uploaded_segments) == 4
