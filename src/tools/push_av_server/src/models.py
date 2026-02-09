"""
Data models for the Push AV Server.
"""

from enum import Enum
from typing import Dict, Optional, Tuple

from pydantic import BaseModel


class SupportedIngestInterface(str, Enum):
    """Supported ingest interfaces for media streams."""
    cmaf = "cmaf-ingest"  # Interface 1
    dash = "dash"  # Interface 2, DASH version
    hls = "hls"  # Interface 2, HLS version


class TrackState(str, Enum):
    """State of a track in the upload process."""
    NOT_STARTED = "not_started"
    INITIAL_PLAYLIST_UPLOADED = "initial_playlist_uploaded"  # HLS only
    INIT_UPLOADED = "init_uploaded"
    SEGMENTS_UPLOADING = "segments_uploading"
    COMPLETED = "completed"


class Track(BaseModel):
    """Represents a track within a session with upload state tracking."""
    name: str
    state: TrackState = TrackState.NOT_STARTED
    segment_count: int = 0


class UploadError(BaseModel):
    """Represents an upload error with associated metadata."""
    session_id: Optional[int]
    file_path: str
    reasons: list[str]


class ValidUpload(BaseModel):
    """Represents a valid upload with associated metadata."""
    session_id: Optional[int]
    file_path: str


class Session(BaseModel):
    """Represents a streaming session with upload tracking."""
    # The id is the index in the stream's list.
    # Keeping a duplicated value here to have it included in API responses.
    id: int
    uploaded_segments: list[Tuple[str, str]] = []
    uploaded_manifests: list[Tuple[str, str]] = []
    complete: bool = False
    # Track state management
    tracks: Dict[str, Track] = {}
    # HLS-specific tracking attributes
    hls_expected_track_count: Optional[int] = None
    hls_completed_tracks: int = 0


class Stream(BaseModel):
    """Represents a Push AV stream with configuration and tracking."""
    # Configuration of the PushAv stream
    id: int
    strict_mode: bool = True
    interface: SupportedIngestInterface
    track_name: Optional[str] = None
    # Keep track of the various sessions encountered
    sessions: list[Session] = []
    # tracking uploads with unique file paths
    error_uploads: list[UploadError] = []
    valid_uploads: list[ValidUpload] = []

    def new_session(self) -> Session:
        """Create a new session for this stream."""
        session_id = len(self.sessions) + 1
        session = Session(id=session_id)
        self.sessions.append(session)
        return session

    def last_in_progress_session(self) -> Optional[Session]:
        """Get the last session that is still in progress."""
        if len(self.sessions) == 0:
            return None
        last_session = self.sessions[-1]
        if not last_session.complete:
            return last_session
        return None


class SignClientCertificate(BaseModel):
    """Request model to sign a client certificate."""
    csr: str


class TrackNameRequest(BaseModel):
    """Request model to update track name for a stream."""
    track_name: str
