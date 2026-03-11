from typing import Any, Dict, Iterable

from ..packet_builder import QuicSentPacket
from .base import (
    K_INITIAL_WINDOW,
    K_MINIMUM_WINDOW,
    QuicCongestionControl,
    QuicRttMonitor,
    register_congestion_control,
)

# cubic specific variables (see https://www.rfc-editor.org/rfc/rfc9438.html#name-definitions)
K_CUBIC_C = 0.4
K_CUBIC_LOSS_REDUCTION_FACTOR = 0.7
K_CUBIC_MAX_IDLE_TIME = 2  # reset the cwnd after 2 seconds of inactivity


def better_cube_root(x: float) -> float:
    if x < 0:
        # avoid precision errors that make the cube root returns an imaginary number
        return -((-x) ** (1.0 / 3.0))
    else:
        return (x) ** (1.0 / 3.0)


class CubicCongestionControl(QuicCongestionControl):
    """
    Cubic congestion control implementation for aioquic
    """

    def __init__(self, max_datagram_size: int) -> None:
        super().__init__(max_datagram_size=max_datagram_size)
        # increase by one segment
        self.additive_increase_factor: int = max_datagram_size
        self._max_datagram_size: int = max_datagram_size
        self._congestion_recovery_start_time = 0.0

        self._rtt_monitor = QuicRttMonitor()

        self.rtt = 0.02  # starting RTT is considered to be 20ms

        self.reset()

        self.last_ack = 0.0

    def W_cubic(self, t) -> int:
        W_max_segments = self._W_max / self._max_datagram_size
        target_segments = K_CUBIC_C * (t - self.K) ** 3 + (W_max_segments)
        return int(target_segments * self._max_datagram_size)

    def is_reno_friendly(self, t) -> bool:
        return self.W_cubic(t) < self._W_est

    def is_concave(self) -> bool:
        return self.congestion_window < self._W_max

    def reset(self) -> None:
        self.congestion_window = K_INITIAL_WINDOW * self._max_datagram_size
        self.ssthresh = None

        self._first_slow_start = True
        self._starting_congestion_avoidance = False
        self.K: float = 0.0
        self._W_est = 0
        self._cwnd_epoch = 0
        self._t_epoch = 0.0
        self._W_max = self.congestion_window

    def on_packet_acked(self, *, now: float, packet: QuicSentPacket) -> None:
        self.bytes_in_flight -= packet.sent_bytes
        self.last_ack = packet.sent_time

        if self.ssthresh is None or self.congestion_window < self.ssthresh:
            # slow start
            self.congestion_window += packet.sent_bytes
        else:
            # congestion avoidance
            if self._first_slow_start and not self._starting_congestion_avoidance:
                # exiting slow start without having a loss
                self._first_slow_start = False
                self._W_max = self.congestion_window
                self._t_epoch = now
                self._cwnd_epoch = self.congestion_window
                self._W_est = self._cwnd_epoch
                # calculate K
                W_max_segments = self._W_max / self._max_datagram_size
                cwnd_epoch_segments = self._cwnd_epoch / self._max_datagram_size
                self.K = better_cube_root(
                    (W_max_segments - cwnd_epoch_segments) / K_CUBIC_C
                )

            # initialize the variables used at start of congestion avoidance
            if self._starting_congestion_avoidance:
                self._starting_congestion_avoidance = False
                self._first_slow_start = False
                self._t_epoch = now
                self._cwnd_epoch = self.congestion_window
                self._W_est = self._cwnd_epoch
                # calculate K
                W_max_segments = self._W_max / self._max_datagram_size
                cwnd_epoch_segments = self._cwnd_epoch / self._max_datagram_size
                self.K = better_cube_root(
                    (W_max_segments - cwnd_epoch_segments) / K_CUBIC_C
                )

            self._W_est = int(
                self._W_est
                + self.additive_increase_factor
                * (packet.sent_bytes / self.congestion_window)
            )

            t = now - self._t_epoch

            target: int = 0
            W_cubic = self.W_cubic(t + self.rtt)
            if W_cubic < self.congestion_window:
                target = self.congestion_window
            elif W_cubic > 1.5 * self.congestion_window:
                target = int(self.congestion_window * 1.5)
            else:
                target = W_cubic

            if self.is_reno_friendly(t):
                # reno friendly region of cubic
                # (https://www.rfc-editor.org/rfc/rfc9438.html#name-reno-friendly-region)
                self.congestion_window = self._W_est
            elif self.is_concave():
                # concave region of cubic
                # (https://www.rfc-editor.org/rfc/rfc9438.html#name-concave-region)
                self.congestion_window = int(
                    self.congestion_window
                    + (
                        (target - self.congestion_window)
                        * (self._max_datagram_size / self.congestion_window)
                    )
                )
            else:
                # convex region of cubic
                # (https://www.rfc-editor.org/rfc/rfc9438.html#name-convex-region)
                self.congestion_window = int(
                    self.congestion_window
                    + (
                        (target - self.congestion_window)
                        * (self._max_datagram_size / self.congestion_window)
                    )
                )

    def on_packet_sent(self, *, packet: QuicSentPacket) -> None:
        self.bytes_in_flight += packet.sent_bytes
        if self.last_ack == 0.0:
            return
        elapsed_idle = packet.sent_time - self.last_ack
        if elapsed_idle >= K_CUBIC_MAX_IDLE_TIME:
            self.reset()

    def on_packets_expired(self, *, packets: Iterable[QuicSentPacket]) -> None:
        for packet in packets:
            self.bytes_in_flight -= packet.sent_bytes

    def on_packets_lost(self, *, now: float, packets: Iterable[QuicSentPacket]) -> None:
        lost_largest_time = 0.0
        for packet in packets:
            self.bytes_in_flight -= packet.sent_bytes
            lost_largest_time = packet.sent_time

        # start a new congestion event if packet was sent after the
        # start of the previous congestion recovery period.
        if lost_largest_time > self._congestion_recovery_start_time:
            self._congestion_recovery_start_time = now

            # Normal congestion handle, can't be used in same time as fast convergence
            # self._W_max = self.congestion_window

            # fast convergence
            if self._W_max is not None and self.congestion_window < self._W_max:
                self._W_max = int(
                    self.congestion_window * (1 + K_CUBIC_LOSS_REDUCTION_FACTOR) / 2
                )
            else:
                self._W_max = self.congestion_window

            # normal congestion MD
            flight_size = self.bytes_in_flight
            new_ssthresh = max(
                int(flight_size * K_CUBIC_LOSS_REDUCTION_FACTOR),
                K_MINIMUM_WINDOW * self._max_datagram_size,
            )
            self.ssthresh = new_ssthresh
            self.congestion_window = max(
                self.ssthresh, K_MINIMUM_WINDOW * self._max_datagram_size
            )

            # restart a new congestion avoidance phase
            self._starting_congestion_avoidance = True

    def on_rtt_measurement(self, *, now: float, rtt: float) -> None:
        self.rtt = rtt
        # check whether we should exit slow start
        if self.ssthresh is None and self._rtt_monitor.is_rtt_increasing(
            rtt=rtt, now=now
        ):
            self.ssthresh = self.congestion_window

    def get_log_data(self) -> Dict[str, Any]:
        data = super().get_log_data()

        data["cubic-wmax"] = int(self._W_max)

        return data


register_congestion_control("cubic", CubicCongestionControl)
