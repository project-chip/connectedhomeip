"""
Stream management service for handling media streams and sessions.
"""

import contextlib
import json
import logging
from typing import Optional

from models import Stream, SupportedIngestInterface, UploadError, ValidUpload
from utils import WorkingDirectory

log = logging.getLogger(__name__)


class StreamService:
    """Service for managing media streams and their sessions."""

    def __init__(self, working_directory: WorkingDirectory):
        self.wd = working_directory
        self.streams = self._load_streams()

    def _load_streams(self) -> dict[str, Stream]:
        """Load all streams from disk."""
        streams: dict[str, Stream] = {}
        streams_dir = self.wd.path("streams")

        if not streams_dir.exists():
            return streams

        for stream_path in streams_dir.iterdir():
            if stream_path.is_dir():
                stream_file = stream_path / "stream.json"
                if stream_file.exists():
                    with open(stream_file, 'r', encoding='utf-8') as f:
                        stream_data = json.load(f)
                        streams[stream_path.name] = Stream.model_validate(stream_data)
        return streams

    def create_stream(self, stream_id: int, interface: SupportedIngestInterface, strict_mode: bool) -> Stream:
        """Create a new stream with the given configuration."""
        stream_id_str = str(stream_id)

        stream = Stream(
            id=stream_id,
            strict_mode=strict_mode,
            interface=interface,
        )

        self.streams[stream_id_str] = stream
        self.wd.mkdir("streams", str(stream_id))
        self._save_stream(stream)

        log.info(f"Stream created: id={stream_id}, interface={interface}")
        return stream

    def get_stream(self, stream_id: int) -> Optional[Stream]:
        """Get a stream by ID."""
        return self.streams.get(str(stream_id))

    def list_streams(self) -> list[Stream]:
        """List all streams."""
        return list(self.streams.values())

    @contextlib.contextmanager
    def open_stream(self, stream_id: int):
        """Context manager helper to save a stream after use.

        Note that any exceptions raised within the context will prevent streams from being saved to disk.
        """
        stream_id_str = str(stream_id)

        stream = self.streams.get(stream_id_str)

        yield stream

        if stream:
            self._save_stream(stream)

    def _save_stream(self, stream: Stream):
        """Save a stream to disk."""
        p = self.wd.path("streams", str(stream.id), "stream.json")
        with open(p, 'w', encoding='utf-8') as f:
            json.dump(stream.model_dump(), f, ensure_ascii=False, indent=4)

    def update_stream(self, stream: Stream):
        """Update a stream and save to disk."""
        self.streams[str(stream.id)] = stream
        self._save_stream(stream)

    def add_error_upload(self, stream: Stream, session_id: Optional[int], file_path: str, reasons: list[str]):
        """Add a file to error_uploads if it doesn't already exist."""
        if not self._is_file_in_error_uploads(stream, file_path):
            # Check if file exists in valid_uploads and remove it
            if self._is_file_in_valid_uploads(stream, file_path):
                stream.valid_uploads = [valid for valid in stream.valid_uploads if valid.file_path != file_path]
            stream.error_uploads.append(UploadError(session_id=session_id, file_path=file_path, reasons=reasons))

    def add_valid_upload(self, stream: Stream, session_id: Optional[int], file_path: str):
        """Add a file to valid_uploads if it doesn't already exist and isn't in error_uploads."""
        if not self._is_file_in_valid_uploads(stream, file_path) and not self._is_file_in_error_uploads(stream, file_path):
            stream.valid_uploads.append(ValidUpload(session_id=session_id, file_path=file_path))

    def _is_file_in_error_uploads(self, stream: Stream, file_path: str) -> bool:
        """Check if a file path already exists in error_uploads."""
        return any(error.file_path == file_path for error in stream.error_uploads)

    def _is_file_in_valid_uploads(self, stream: Stream, file_path: str) -> bool:
        """Check if a file path already exists in valid_uploads."""
        return any(valid.file_path == file_path for valid in stream.valid_uploads)

    def get_next_stream_id(self) -> int:
        """Get the next available stream ID."""
        stream_ids = [int(d.name) for d in self.wd.path("streams").iterdir() if d.is_dir() and d.name.isdigit()]
        last_stream = max(stream_ids) if stream_ids else 0
        return last_stream + 1
