from typing import Optional

from . import events
from .packet import (
    QuicErrorCode,
    QuicResetStreamFrame,
    QuicStopSendingFrame,
    QuicStreamFrame,
)
from .packet_builder import QuicDeliveryState
from .rangeset import RangeSet


class FinalSizeError(Exception):
    pass


class StreamFinishedError(Exception):
    pass


class QuicStreamReceiver:
    """
    The receive part of a QUIC stream.

    It finishes:
    - immediately for a send-only stream
    - upon reception of a STREAM_RESET frame
    - upon reception of a data frame with the FIN bit set
    """

    def __init__(self, stream_id: Optional[int], readable: bool) -> None:
        self.highest_offset = 0  # the highest offset ever seen
        self.is_finished = False
        self.stop_pending = False

        self._buffer = bytearray()
        self._buffer_start = 0  # the offset for the start of the buffer
        self._final_size: Optional[int] = None
        self._ranges = RangeSet()
        self._stream_id = stream_id
        self._stop_error_code: Optional[int] = None

    def get_stop_frame(self) -> QuicStopSendingFrame:
        self.stop_pending = False
        return QuicStopSendingFrame(
            error_code=self._stop_error_code,
            stream_id=self._stream_id,
        )

    def handle_frame(
        self, frame: QuicStreamFrame
    ) -> Optional[events.StreamDataReceived]:
        """
        Handle a frame of received data.
        """
        pos = frame.offset - self._buffer_start
        count = len(frame.data)
        frame_end = frame.offset + count

        # we should receive no more data beyond FIN!
        if self._final_size is not None:
            if frame_end > self._final_size:
                raise FinalSizeError("Data received beyond final size")
            elif frame.fin and frame_end != self._final_size:
                raise FinalSizeError("Cannot change final size")
        if frame.fin:
            self._final_size = frame_end
        if frame_end > self.highest_offset:
            self.highest_offset = frame_end

        # fast path: new in-order chunk
        if pos == 0 and count and not self._buffer:
            self._buffer_start += count
            if frame.fin:
                # all data up to the FIN has been received, we're done receiving
                self.is_finished = True
            return events.StreamDataReceived(
                data=frame.data, end_stream=frame.fin, stream_id=self._stream_id
            )

        # discard duplicate data
        if pos < 0:
            frame.data = frame.data[-pos:]
            frame.offset -= pos
            pos = 0
            count = len(frame.data)

        # marked received range
        if frame_end > frame.offset:
            self._ranges.add(frame.offset, frame_end)

        # add new data
        gap = pos - len(self._buffer)
        if gap > 0:
            self._buffer += bytearray(gap)
        self._buffer[pos : pos + count] = frame.data

        # return data from the front of the buffer
        data = self._pull_data()
        end_stream = self._buffer_start == self._final_size
        if end_stream:
            # all data up to the FIN has been received, we're done receiving
            self.is_finished = True
        if data or end_stream:
            return events.StreamDataReceived(
                data=data, end_stream=end_stream, stream_id=self._stream_id
            )
        else:
            return None

    def handle_reset(
        self, *, final_size: int, error_code: int = QuicErrorCode.NO_ERROR
    ) -> Optional[events.StreamReset]:
        """
        Handle an abrupt termination of the receiving part of the QUIC stream.
        """
        if self._final_size is not None and final_size != self._final_size:
            raise FinalSizeError("Cannot change final size")

        # we are done receiving
        self._final_size = final_size
        self.is_finished = True
        return events.StreamReset(error_code=error_code, stream_id=self._stream_id)

    def on_stop_sending_delivery(self, delivery: QuicDeliveryState) -> None:
        """
        Callback when a STOP_SENDING is ACK'd.
        """
        if delivery != QuicDeliveryState.ACKED:
            self.stop_pending = True

    def stop(self, error_code: int = QuicErrorCode.NO_ERROR) -> None:
        """
        Request the peer stop sending data on the QUIC stream.
        """
        self._stop_error_code = error_code
        self.stop_pending = True

    def _pull_data(self) -> bytes:
        """
        Remove data from the front of the buffer.
        """
        try:
            has_data_to_read = self._ranges[0].start == self._buffer_start
        except IndexError:
            has_data_to_read = False
        if not has_data_to_read:
            return b""

        r = self._ranges.shift()
        pos = r.stop - r.start
        data = bytes(self._buffer[:pos])
        del self._buffer[:pos]
        self._buffer_start = r.stop
        return data


class QuicStreamSender:
    """
    The send part of a QUIC stream.

    It finishes:
    - immediately for a receive-only stream
    - upon acknowledgement of a STREAM_RESET frame
    - upon acknowledgement of a data frame with the FIN bit set
    """

    def __init__(self, stream_id: Optional[int], writable: bool) -> None:
        self.buffer_is_empty = True
        self.highest_offset = 0
        self.is_finished = not writable
        self.reset_pending = False

        self._acked = RangeSet()
        self._acked_fin = False
        self._buffer = bytearray()
        self._buffer_fin: Optional[int] = None
        self._buffer_start = 0  # the offset for the start of the buffer
        self._buffer_stop = 0  # the offset for the stop of the buffer
        self._pending = RangeSet()
        self._pending_eof = False
        self._reset_error_code: Optional[int] = None
        self._stream_id = stream_id

    @property
    def next_offset(self) -> int:
        """
        The offset for the next frame to send.

        This is used to determine the space needed for the frame's `offset` field.
        """
        try:
            return self._pending[0].start
        except IndexError:
            return self._buffer_stop

    def get_frame(
        self, max_size: int, max_offset: Optional[int] = None
    ) -> Optional[QuicStreamFrame]:
        """
        Get a frame of data to send.
        """
        # get the first pending data range
        try:
            r = self._pending[0]
        except IndexError:
            if self._pending_eof:
                # FIN only
                self._pending_eof = False
                return QuicStreamFrame(fin=True, offset=self._buffer_fin)

            self.buffer_is_empty = True
            return None

        # apply flow control
        start = r.start
        stop = min(r.stop, start + max_size)
        if max_offset is not None and stop > max_offset:
            stop = max_offset
        if stop <= start:
            return None

        # create frame
        frame = QuicStreamFrame(
            data=bytes(
                self._buffer[start - self._buffer_start : stop - self._buffer_start]
            ),
            offset=start,
        )
        self._pending.subtract(start, stop)

        # track the highest offset ever sent
        if stop > self.highest_offset:
            self.highest_offset = stop

        # if the buffer is empty and EOF was written, set the FIN bit
        if self._buffer_fin == stop:
            frame.fin = True
            self._pending_eof = False

        return frame

    def get_reset_frame(self) -> QuicResetStreamFrame:
        self.reset_pending = False
        return QuicResetStreamFrame(
            error_code=self._reset_error_code,
            final_size=self.highest_offset,
            stream_id=self._stream_id,
        )

    def on_data_delivery(
        self, delivery: QuicDeliveryState, start: int, stop: int, fin: bool
    ) -> None:
        """
        Callback when sent data is ACK'd.
        """
        # If the frame had the FIN bit set, its end MUST match otherwise
        # we have a programming error.
        assert not fin or stop == self._buffer_fin

        if delivery == QuicDeliveryState.ACKED:
            if stop > start:
                # Some data has been ACK'd, discard it.
                self._acked.add(start, stop)
                first_range = self._acked[0]
                if first_range.start == self._buffer_start:
                    size = first_range.stop - first_range.start
                    self._acked.shift()
                    self._buffer_start += size
                    del self._buffer[:size]

            if fin:
                # The FIN has been ACK'd.
                self._acked_fin = True

            if self._buffer_start == self._buffer_fin and self._acked_fin:
                # All data and the FIN have been ACK'd, we're done sending.
                self.is_finished = True
        else:
            if stop > start:
                # Some data has been lost, reschedule it.
                self.buffer_is_empty = False
                self._pending.add(start, stop)

            if fin:
                # The FIN has been lost, reschedule it.
                self.buffer_is_empty = False
                self._pending_eof = True

    def on_reset_delivery(self, delivery: QuicDeliveryState) -> None:
        """
        Callback when a reset is ACK'd.
        """
        if delivery == QuicDeliveryState.ACKED:
            # the reset has been ACK'd, we're done sending
            self.is_finished = True
        else:
            self.reset_pending = True

    def reset(self, error_code: int) -> None:
        """
        Abruptly terminate the sending part of the QUIC stream.
        """
        assert self._reset_error_code is None, "cannot call reset() more than once"
        self._reset_error_code = error_code
        self.reset_pending = True

    def write(self, data: bytes, end_stream: bool = False) -> None:
        """
        Write some data bytes to the QUIC stream.
        """
        assert self._buffer_fin is None, "cannot call write() after FIN"
        assert self._reset_error_code is None, "cannot call write() after reset()"
        size = len(data)

        if size:
            self.buffer_is_empty = False
            self._pending.add(self._buffer_stop, self._buffer_stop + size)
            self._buffer += data
            self._buffer_stop += size
        if end_stream:
            self.buffer_is_empty = False
            self._buffer_fin = self._buffer_stop
            self._pending_eof = True


class QuicStream:
    def __init__(
        self,
        stream_id: Optional[int] = None,
        max_stream_data_local: int = 0,
        max_stream_data_remote: int = 0,
        readable: bool = True,
        writable: bool = True,
    ) -> None:
        self.is_blocked = False
        self.max_stream_data_local = max_stream_data_local
        self.max_stream_data_local_sent = max_stream_data_local
        self.max_stream_data_remote = max_stream_data_remote
        self.receiver = QuicStreamReceiver(stream_id=stream_id, readable=readable)
        self.sender = QuicStreamSender(stream_id=stream_id, writable=writable)
        self.stream_id = stream_id

    @property
    def is_finished(self) -> bool:
        return self.receiver.is_finished and self.sender.is_finished
