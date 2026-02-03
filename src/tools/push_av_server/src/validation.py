"""
Validation service for media uploads.
"""

import logging
import re
import xml.etree.ElementTree
from typing import Optional

from models import Session, Stream, SupportedIngestInterface

log = logging.getLogger(__name__)


class ValidationService:
    """Service for validating media uploads according to Matter specifications."""

    def __init__(self):
        # Path regex patterns
        self.dash_manifest_path_regex = re.compile(r"^session_\d+/index$")
        self.hls_multi_variant_path_regex = re.compile(r"^session_\d+/index$")
        self.hls_media_playlist_path_regex = re.compile(r"^session_\d+/(?P<trackName>[^/]+)/index$")
        self.segment_path_regex = re.compile(r"^session_\d+/(?P<trackName>[^/]+)/segment_\d+$")
        self.init_path_regex = re.compile(r"^session_\d+/(?P<trackName>[^/]+)/[^/]+")

    def validate_upload(
        self,
        stream: Stream,
        session: Optional[Session],
        file_path: str,
        ext: str,
        body: bytes,
        headers: dict
    ) -> tuple[list[str], Optional[Session]]:
        """
        Validate a media upload.
        Returns a tuple of (errors, session).
        """
        errors = []

        if ext == "mpd":
            errors, session = self._validate_mpd_upload(stream, session, file_path, body)
        elif ext == "m3u8":
            errors, session = self._validate_m3u8_upload(stream, session, file_path, body)
        elif ext in ("m4s", "init"):
            errors, session = self._validate_segment_upload(stream, session, file_path, ext, headers)
        else:
            errors.append(f"Invalid extension: {ext}, valid extensions are mpd, m3u8, m4s, init")

        return errors, session

    def _validate_mpd_upload(
        self,
        stream: Stream,
        session: Optional[Session],
        file_path: str,
        body: bytes
    ) -> tuple[list[str], Optional[Session]]:
        """Validate DASH manifest upload."""
        errors = []

        if stream.interface != SupportedIngestInterface.dash:
            errors.append("Unsupported manifest object extension")

        if session is None:
            session = stream.new_session()

        root = xml.etree.ElementTree.fromstring(body)
        mpd_type = root.attrib.get('type')

        if mpd_type == "dynamic" and len(session.uploaded_segments) > 0:
            errors.append("Dynamic MPD cannot be uploaded after segments have been uploaded")

        if mpd_type == "static" and len(session.uploaded_segments) == 0:
            errors.append("Static MPD cannot be uploaded before segments have been uploaded")

        if mpd_type == "static":
            session.complete = True

        if not self.dash_manifest_path_regex.match(file_path):
            errors.append("DASH manifest must be uploaded as session_X/index.mpd")

        session.uploaded_segments.append((f"{file_path}.mpd", f"{file_path}.mpd.crt"))
        return errors, session

    def _validate_m3u8_upload(
        self,
        stream: Stream,
        session: Optional[Session],
        file_path: str,
        body: bytes
    ) -> tuple[list[str], Optional[Session]]:
        """Validate HLS manifest upload."""
        errors = []

        if stream.interface != SupportedIngestInterface.hls:
            errors.append("Unsupported manifest object extension")

        # Parse the m3u8 content
        body_str = body.decode('utf-8', errors='replace')
        lines = body_str.split('\n')

        # Check if this is a multi-variant playlist or a media playlist
        is_multi_variant = self.hls_multi_variant_path_regex.match(file_path)
        is_media_playlist = self.hls_media_playlist_path_regex.match(file_path)

        if is_multi_variant:
            errors, session = self._validate_hls_multi_variant(stream, session, lines)
        elif is_media_playlist:
            errors, session = self._validate_hls_media_playlist(stream, session, file_path, is_media_playlist, lines, body_str)
        else:
            errors.append("HLS manifest must be uploaded as session_X/index.m3u8 (multi-variant) or session_X/track_name/index.m3u8 (media playlist)")

        if session:
            session.uploaded_segments.append((f"{file_path}.m3u8", f"{file_path}.m3u8.crt"))

        return errors, session

    def _validate_hls_multi_variant(
        self,
        stream: Stream,
        session: Optional[Session],
        lines: list[str]
    ) -> tuple[list[str], Optional[Session]]:
        """Validate HLS multi-variant playlist."""
        errors = []

        if session is None:
            session = stream.new_session()

        # Multi-variant playlist must be uploaded first
        if len(session.uploaded_segments) > 0:
            errors.append("Multi-variant playlist must be uploaded first, before any tracks")

        # Count tracks by counting EXT-X-MEDIA and EXT-X-STREAM-INF lines
        track_count = 0
        for line in lines:
            if line.startswith('#EXT-X-MEDIA') or line.startswith('#EXT-X-STREAM-INF'):
                track_count += 1

        if track_count == 0:
            errors.append("Multi-variant playlist must contain at least one EXT-X-MEDIA or EXT-X-STREAM-INF tag")

        # Store track count in session for later validation
        if not hasattr(session, 'hls_expected_track_count'):
            session.hls_expected_track_count = track_count
        elif session.hls_expected_track_count != track_count:
            errors.append(f"Multi-variant playlist track count mismatch: expected {session.hls_expected_track_count}, got {track_count}")

        return errors, session

    def _validate_hls_media_playlist(
        self,
        stream: Stream,
        session: Optional[Session],
        file_path: str,
        is_media_playlist: re.Match,
        lines: list[str],
        body_str: str
    ) -> tuple[list[str], Optional[Session]]:
        """Validate HLS media playlist."""
        errors = []

        if session is None:
            errors.append("No active session when uploading media playlist")
            session = stream.new_session()

        # Validate track name
        track_name_in_path = is_media_playlist.group("trackName")
        track_name = stream.track_name
        if track_name and track_name != track_name_in_path:
            errors.append(
                "Track name mismatch: "
                f"{track_name_in_path} != {track_name}, "
                "must match TrackName provided in ContainerOptions"
            )

        # Check if this is initial or final media playlist
        has_ext_x_playlist_type = any(line.startswith('#EXT-X-PLAYLIST-TYPE') for line in lines)
        has_ext_x_endlist = any(line.startswith('#EXT-X-ENDLIST') for line in lines)
        has_extinf = any(line.startswith('#EXTINF') for line in lines)
        has_segment_lines = any(not line.startswith('#') and line.strip() != '' for line in lines)

        # Check if multi-variant playlist was uploaded first
        if not hasattr(session, 'hls_expected_track_count'):
            errors.append("Multi-variant playlist must be uploaded before media playlists")

        if has_ext_x_endlist:
            # Final media playlist
            if not has_ext_x_playlist_type:
                errors.append("Final media playlist must contain #EXT-X-PLAYLIST-TYPE:VOD")
            if 'VOD' not in body_str:
                errors.append("Final media playlist must have #EXT-X-PLAYLIST-TYPE:VOD")
            if not has_extinf:
                errors.append("Final media playlist must contain #EXTINF tags")
            if not has_segment_lines:
                errors.append("Final media playlist must contain segment lines")

            # Track completion
            if not hasattr(session, 'hls_completed_tracks'):
                session.hls_completed_tracks = 0
            session.hls_completed_tracks += 1

            # Check if all tracks are completed
            if session.hls_completed_tracks == session.hls_expected_track_count:
                session.complete = True
        else:
            # Initial media playlist
            if has_ext_x_playlist_type:
                errors.append("Initial media playlist must NOT contain #EXT-X-PLAYLIST-TYPE")
            if has_ext_x_endlist:
                errors.append("Initial media playlist must NOT contain #EXT-X-ENDLIST")
            if has_extinf:
                errors.append("Initial media playlist must NOT contain #EXTINF tags")
            if has_segment_lines:
                errors.append("Initial media playlist must NOT contain segment lines")

        return errors, session

    def _validate_segment_upload(
        self,
        stream: Stream,
        session: Optional[Session],
        file_path: str,
        ext: str,
        headers: dict
    ) -> tuple[list[str], Optional[Session]]:
        """Validate segment upload (m4s or init)."""
        errors = []

        if session is not None:
            session.uploaded_segments.append((f"{file_path}.{ext}", f"{file_path}.{ext}.crt"))
        else:
            if stream.interface == SupportedIngestInterface.dash:
                errors.append(f"No active session when uploading {file_path}.{ext}, segment uploaded before mpd")
            elif stream.interface == SupportedIngestInterface.hls:
                errors.append(f"No active session when uploading {file_path}.{ext}, segment uploaded before multi-variant playlist")

        # Validate path format
        path_regex = self.segment_path_regex if ext == "m4s" else self.init_path_regex
        match = path_regex.match(file_path)

        if not match:
            errors.append("Path does not adhere to Matter's path format")
        else:
            # Validate track name
            track_name_in_path = match.group("trackName")
            track_name = stream.track_name
            if track_name and track_name != track_name_in_path:
                errors.append(
                    "Track name mismatch: "
                    f"{track_name_in_path} != {track_name}, "
                    "must match TrackName provided in ContainerOptions"
                )

            # HLS-specific validation for media segments
            if stream.interface == SupportedIngestInterface.hls and ext == "m4s":
                extinf_duration = headers.get('X-EXTINF-duration')
                if extinf_duration is None:
                    errors.append("HLS media segments must include X-EXTINF-duration header")
                else:
                    try:
                        float(extinf_duration)
                    except ValueError:
                        errors.append(f"X-EXTINF-duration header must be a valid decimal floating-point value, got: {extinf_duration}")

        return errors, session
