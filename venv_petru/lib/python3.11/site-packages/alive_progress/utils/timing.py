from collections import namedtuple
from math import floor

from typing import Callable

TimeDisplay = namedtuple('TimeDisplay',
                         'round, sec_prec, min_prec, hour_prec, prefix, '
                         'round_sec_on_min, clear_sec_on_hour')
RUN = TimeDisplay(0, .0, 2.0, 2.0, '', False, False)
END = TimeDisplay(1, .1, 4.1, 4.1, '', False, False)
ETA = RUN._replace(prefix='~', round_sec_on_min=True, clear_sec_on_hour=True)


def time_display(seconds: float, conf: TimeDisplay) -> str:
    seconds = round(seconds, conf.round)
    if seconds < 60.:
        return '{}{:{}f}s'.format(conf.prefix, seconds, conf.sec_prec)

    minutes, seconds = divmod(seconds, 60.)
    if minutes < 60.:
        if conf.round_sec_on_min:
            seconds = floor(seconds / 10) * 10
        return '{}{:.0f}:{:0{}f}'.format(conf.prefix, minutes, seconds, conf.min_prec)

    hours, minutes = divmod(minutes, 60.)
    if conf.clear_sec_on_hour:
        seconds = 0
    return '{}{:.0f}:{:02.0f}:{:0{}f}'.format(conf.prefix, hours, minutes, seconds, conf.hour_prec)


def eta_text(seconds: float) -> str:
    if seconds < 0.:
        return '?'
    return time_display(seconds, ETA)


def fn_simple_eta(logic_total):
    def simple_eta(pos, rate):
        return (logic_total - pos) / rate

    return simple_eta


def gen_simple_exponential_smoothing(alpha: float, fn: Callable[[float, float], float]):
    """Implements a generator with a simple exponential smoothing of some function.
    Given alpha and y_hat (t-1), we can calculate the next y_hat:
        y_hat = alpha * y + (1 - alpha) * y_hat
        y_hat = alpha * y + y_hat - alpha * y_hat
        y_hat = y_hat + alpha * (y - y_hat)

    Args:
        alpha: the smoothing coefficient
        fn: the function

    Returns:

    """
    p = (0.,)
    while any(x == 0. for x in p):
        p = yield 0.
    y_hat = fn(*p)
    while True:
        p = yield y_hat
        y = fn(*p)
        y_hat += alpha * (y - y_hat)
