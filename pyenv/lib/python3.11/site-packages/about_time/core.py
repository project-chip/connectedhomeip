import time
from contextlib import contextmanager

from .human_count import HumanCount
from .human_duration import HumanDuration
from .human_throughput import HumanThroughput


def about_time(func_or_it=None, *args, **kwargs):
    """Measure timing and throughput of code blocks, with beautiful
    human friendly representations.

    There are three modes of operation: context manager, callable and
    throughput.

    1. Use it like a context manager:

    >>> with about_time() as t:
    ....    # code block.

    2. Use it with a callable:

    >>> def func(a, b): ...
    >>> t = about_time(func, 1, b=2)  # send arguments at will.

    3. Use it with an iterable or generator:

    >>> t = about_time(it)  # any iterable or generator.
    >>> for item in t:
    ....    # use item
    """

    timings = [0.0, 0.0]

    # use as a context manager.
    if func_or_it is None:
        return _context_timing(timings, Handle(timings))

    # use as a callable.
    if callable(func_or_it):
        with _context_timing(timings):
            result = func_or_it(*args, **kwargs)
        return HandleResult(timings, result)

    try:
        it = iter(func_or_it)
    except TypeError:
        raise UserWarning('param should be callable or iterable.')

    # use as a counter/throughput iterator.
    def it_closure():
        with _context_timing(timings):
            for it_closure.count, elem in enumerate(it, 1):  # iterators are iterable.
                yield elem

    it_closure.count = 0  # the count will only be updated after starting iterating.
    return HandleStats(timings, it_closure)


@contextmanager
def _context_timing(timings, handle=None):
    timings[0] = time.perf_counter()
    yield handle
    timings[1] = time.perf_counter()


class Handle(object):
    def __init__(self, timings):
        self.__timings = timings

    @property
    def duration(self) -> float:
        """Return the actual duration in seconds.
        This is dynamically updated in real time.

        Returns:
            the number of seconds.

        """
        return (self.__timings[1] or time.perf_counter()) - self.__timings[0]

    @property
    def duration_human(self) -> HumanDuration:
        """Return a beautiful representation of the duration.
        It dynamically calculates the best unit to use.

        Returns:
            the human representation.

        """
        return HumanDuration(self.duration)


class HandleResult(Handle):
    def __init__(self, timings, result):
        super(HandleResult, self).__init__(timings)
        self.__result = result

    @property
    def result(self):
        """Return the result of the callable.

        Returns:
            the result of the callable.

        """
        return self.__result


class HandleStats(Handle):
    def __init__(self, timings, it_closure):
        super(HandleStats, self).__init__(timings)
        self.__it = it_closure

    def __iter__(self):
        return self.__it()

    @property
    def count(self) -> int:
        """Return the current iteration count.
        This is dynamically updated in real time.

        Returns:
            the current iteration count.

        """
        return self.__it.count

    @property
    def count_human(self) -> HumanCount:
        """Return a beautiful representation of the current iteration count.
        This is dynamically updated in real time.

        Returns:
            the human representation.

        """
        return self.count_human_as('')

    def count_human_as(self, unit: str) -> HumanCount:
        """Return a beautiful representation of the current iteration count.
        This is dynamically updated in real time.

        Args:
            unit: what is being measured

        Returns:
            the human representation.

        """
        return HumanCount(self.count, unit)

    @property
    def throughput(self) -> float:
        """Return the current throughput in items per second.
        This is dynamically updated in real time.

        Returns:
            the number of items per second.

        """
        try:
            return self.count / self.duration
        except ZeroDivisionError:  # pragma: no cover
            return float('nan')

    @property
    def throughput_human(self) -> HumanThroughput:
        """Return a beautiful representation of the current throughput.
        It dynamically calculates the best unit to use.

        Returns:
            the human representation.

        """
        return self.throughput_human_as('')

    def throughput_human_as(self, unit: str) -> HumanThroughput:
        """Return a beautiful representation of the current throughput.
        It dynamically calculates the best unit to use.

        Args:
            unit: what is being measured

        Returns:
            the human representation.

        """
        return HumanThroughput(self.throughput, unit)
