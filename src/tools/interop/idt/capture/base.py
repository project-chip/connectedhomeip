from abc import ABC, abstractmethod


class PlatformLogStreamer(ABC):
    """
    The abstract base class for a platform transport, subclassed by sub packages of platform
    """

    @abstractmethod
    def __init__(self, artifact_dir: str) -> None:
        """
         artifact_dir: the fully qualified path of the output directory. This directory already exists
        """

    @abstractmethod
    def start_streaming(self) -> None:
        """
        Begin streaming logs
        Start should be able to be called repeatedly without restarting streaming
        """
        pass

    @abstractmethod
    def stop_streaming(self) -> None:
        """
        Stop streaming logs
        Stop should not cause an error even if the stream is not running
        """
        pass


class UnsupportedCapturePlatformException(Exception):
    """EcosystemCapture should raise this for unsupported platform"""

    def __init__(self, message: str):
        super().__init__(message)


class EcosystemCapture(ABC):
    """
    The abstract base class for an ecosystem capture and analysis, subclassed by sub packages of ecosystem
    """

    @abstractmethod
    def __init__(
            self,
            platform: PlatformLogStreamer,
            artifact_dir: str) -> None:
        """
        platform: the instance of the log streamer for the selected platform
        artifact_dir: the fully qualified path of the output directory. This directory already exists.
        """
        pass

    @abstractmethod
    def start_capture(self) -> None:
        """
        Start the capture
        """
        pass

    @abstractmethod
    def stop_capture(self) -> None:
        """
        Stop the capture
        """
        pass

    @abstractmethod
    def analyze_capture(self) -> None:
        """
        Parse the capture and create + display helpful analysis artifacts that are unique to the ecosystem
        Write analysis artifacts to artifact_dir
        """
        pass
