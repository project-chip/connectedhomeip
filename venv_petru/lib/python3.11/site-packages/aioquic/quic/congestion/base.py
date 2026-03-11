import abc
from typing import Any, Dict, Iterable, Optional, Protocol

from ..packet_builder import QuicSentPacket

K_GRANULARITY = 0.001  # seconds
K_INITIAL_WINDOW = 10
K_MINIMUM_WINDOW = 2


class QuicCongestionControl(abc.ABC):
    """
    Base class for congestion control implementations.
    """

    bytes_in_flight: int = 0
    congestion_window: int = 0
    ssthresh: Optional[int] = None

    def __init__(self, *, max_datagram_size: int) -> None:
        self.congestion_window = K_INITIAL_WINDOW * max_datagram_size

    @abc.abstractmethod
    def on_packet_acked(self, *, now: float, packet: QuicSentPacket) -> None:
        ...  # pragma: no cover

    @abc.abstractmethod
    def on_packet_sent(self, *, packet: QuicSentPacket) -> None:
        ...  # pragma: no cover

    @abc.abstractmethod
    def on_packets_expired(self, *, packets: Iterable[QuicSentPacket]) -> None:
        ...  # pragma: no cover

    @abc.abstractmethod
    def on_packets_lost(self, *, now: float, packets: Iterable[QuicSentPacket]) -> None:
        ...  # pragma: no cover

    @abc.abstractmethod
    def on_rtt_measurement(self, *, now: float, rtt: float) -> None:
        ...  # pragma: no cover

    def get_log_data(self) -> Dict[str, Any]:
        data = {"cwnd": self.congestion_window, "bytes_in_flight": self.bytes_in_flight}
        if self.ssthresh is not None:
            data["ssthresh"] = self.ssthresh
        return data


class QuicCongestionControlFactory(Protocol):
    def __call__(self, *, max_datagram_size: int) -> QuicCongestionControl:
        ...  # pragma: no cover


class QuicRttMonitor:
    """
    Roundtrip time monitor for HyStart.
    """

    def __init__(self) -> None:
        self._increases = 0
        self._last_time = None
        self._ready = False
        self._size = 5

        self._filtered_min: Optional[float] = None

        self._sample_idx = 0
        self._sample_max: Optional[float] = None
        self._sample_min: Optional[float] = None
        self._sample_time = 0.0
        self._samples = [0.0 for i in range(self._size)]

    def add_rtt(self, *, rtt: float) -> None:
        self._samples[self._sample_idx] = rtt
        self._sample_idx += 1

        if self._sample_idx >= self._size:
            self._sample_idx = 0
            self._ready = True

        if self._ready:
            self._sample_max = self._samples[0]
            self._sample_min = self._samples[0]
            for sample in self._samples[1:]:
                if sample < self._sample_min:
                    self._sample_min = sample
                elif sample > self._sample_max:
                    self._sample_max = sample

    def is_rtt_increasing(self, *, now: float, rtt: float) -> bool:
        if now > self._sample_time + K_GRANULARITY:
            self.add_rtt(rtt=rtt)
            self._sample_time = now

            if self._ready:
                if self._filtered_min is None or self._filtered_min > self._sample_max:
                    self._filtered_min = self._sample_max

                delta = self._sample_min - self._filtered_min
                if delta * 4 >= self._filtered_min:
                    self._increases += 1
                    if self._increases >= self._size:
                        return True
                elif delta > 0:
                    self._increases = 0
        return False


_factories: Dict[str, QuicCongestionControlFactory] = {}


def create_congestion_control(
    name: str, *, max_datagram_size: int
) -> QuicCongestionControl:
    """
    Create an instance of the `name` congestion control algorithm.
    """
    try:
        factory = _factories[name]
    except KeyError:
        raise Exception(f"Unknown congestion control algorithm: {name}")
    return factory(max_datagram_size=max_datagram_size)


def register_congestion_control(
    name: str, factory: QuicCongestionControlFactory
) -> None:
    """
    Register a congestion control algorithm named `name`.
    """
    _factories[name] = factory
