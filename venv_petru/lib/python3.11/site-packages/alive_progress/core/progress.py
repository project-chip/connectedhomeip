"""
This module must always be importable, even without the required libs for install!
It's because I import metadata from main init, directly in setup.py, which imports this.
"""
import math
import threading
import time
import io
from contextlib import contextmanager
from typing import Any, Callable, Optional, TypeVar
from collections.abc import Collection, Iterable

from .calibration import calibrated_fps, custom_fps
from .configuration import config_handler
from .hook_manager import buffered_hook_manager, passthrough_hook_manager
from ..utils import terminal
from ..utils.cells import combine_cells, fix_cells, print_cells, to_cells
from ..utils.timing import eta_text, fn_simple_eta, gen_simple_exponential_smoothing, \
    time_display, RUN, END


def alive_bar(total: Optional[int] = None, *, calibrate: Optional[int] = None, **options: Any):
    """An alive progress bar to keep track of lengthy operations.
    It has a spinner indicator, elapsed time, throughput and ETA.
    When the operation finishes, a receipt is displayed with statistics.

    If the code is executed in a headless environment, ie without a
    connected tty, all features are disabled but the final receipt.

    Another cool feature is that it tracks the actual count in regard of the
    expected count. So it will look different if you send more (or less) than
    expected.

    Also, the bar installs a hook in the system print function that cleans
    any garbage out of the terminal, allowing you to print() effortlessly
    while using the bar.

    Use it like this:

    >>> from alive_progress import alive_bar
    ... with alive_bar(123, title='Title') as bar:  # <-- expected total and bar title
    ...     for item in <iterable>:
    ...         # process item
    ...         bar()  # makes the bar go forward

    The `bar()` method should be called whenever you want the bar to go forward.
    You usually call it in every iteration, but you could do it only when some
    criteria match, depending on what you want to monitor.

    While in a progress bar context, you have two ways to output messages:
      - the usual Python `print()` statement, which will properly clean the line,
        print an enriched message (including the current bar position) and
        continue the bar right below it;
      - the `bar.text('message')` call, which sets a situational message right within
        the bar, usually to display something about the items being processed or the
        phase the processing is in.

    If the bar is over or underused, it will warn you!
    To test all supported scenarios, you can do this:
    >>> for x in 1000, 1500, 700, 0:
    ...    with alive_bar(x) as bar:
    ...        for i in range(1000):
    ...            time.sleep(.005)
    ...            bar()
    Expected results are these (but you have to see them in motion!):
|████████████████████████████████████████| 1000/1000 [100%] in 6.0s (167.93/s)
|██████████████████████████▋⚠            | (!) 1000/1500 [67%] in 6.0s (167.57/s)
|████████████████████████████████████████✗ (!) 1000/700 [143%] in 6.0s (167.96/s)
|████████████████████████████████████████| 1000 in 5.8s (171.91/s)

    Args:
        total (Optional[int]): the total expected count
        calibrate (float): maximum theoretical throughput to calibrate animation speed
        **options: custom configuration options, which override the global configuration:
            title (Optional[str]): an optional, always visible bar title
            length (int): the number of cols to render the actual bar in alive_bar
            max_cols (int): the maximum cols to use if not possible to fetch it, like in jupyter
            spinner (Union[None, str, object]): the spinner style to be rendered next to the bar
                accepts a predefined spinner name, a custom spinner factory, or None
            bar (Union[None, str, object]): the bar style to be rendered in known modes
                accepts a predefined bar name, a custom bar factory, or None
            unknown (Union[str, object]): the bar style to be rendered in the unknown mode
                accepts a predefined spinner name, or a custom spinner factory (cannot be None)
            theme (str): a set of matching spinner, bar and unknown
                accepts a predefined theme name
            force_tty (Optional[int|bool]): forces a specific kind of terminal:
                False -> disables animations, keeping only the the final receipt
                True -> enables animations, and auto-detects Jupyter Notebooks!
                None (default) -> auto select, according to the terminal/Jupyter
            file (object): use `sys.stdout`, `sys.stderr`, or a similar `TextIOWrapper` object
            disable (bool): if True, completely disables all output, do not install hooks
            manual (bool): set to manually control the bar position
            enrich_print (bool): enriches print() and logging messages with the bar position
            enrich_offset (int): the offset to apply to enrich_print
            receipt (bool): prints the nice final receipt, disables if False
            receipt_text (bool): set to repeat the last text message in the final receipt
            monitor (bool|str): configures the monitor widget `152/200 [76%]`
                send a string with `{count}`, `{total}` and `{percent}` to customize it
            elapsed (bool|str): configures the elapsed time widget `in 12s`
                send a string with `{elapsed}` to customize it
            stats (bool|str): configures the stats widget `(~12s, 123.4/s)`
                send a string with `{rate}` and `{eta}` to customize it
            monitor_end (bool|str): configures the monitor widget within final receipt
                same as monitor, the default format is dynamic, it inherits monitor's one
            elapsed_end (bool|str): configures the elapsed time widget within final receipt
                same as elapsed, the default format is dynamic, it inherits elapsed's one
            stats_end (bool|str): configures the stats widget within final receipt
                send a string with `{rate}` to customize it (no relation to stats)
            title_length (int): fixes the title lengths, or 0 for unlimited
                title will be truncated if longer, and a cool ellipsis "…" will appear at the end
            spinner_length (int): forces the spinner length, or `0` for its natural one
            refresh_secs (int): forces the refresh period, `0` for the reactive visual feedback
            ctrl_c (bool): if False, disables CTRL+C (captures it)
            dual_line (bool): if True, places the text below the bar
            unit (str): any text that labels your entities
            scale (any): the scaling to apply to units: 'SI', 'IEC', 'SI2'
            precision (int): how many decimals do display when scaling
    """

    try:
        config = config_handler(**options)
    except Exception as e:
        raise type(e)(str(e)) from None
    return __alive_bar(config, total, calibrate=calibrate)


@contextmanager
def __alive_bar(config, total=None, *, calibrate=None,
                _cond=threading.Condition, _sampling=False, _testing=None):
    """Actual alive_bar handler, that exposes internal functions for configuration of
    both normal operation and sampling overhead."""

    if total is not None:
        if not isinstance(total, int):
            raise TypeError(f"integer argument expected, got '{type(total).__name__}'.")
        if total <= 0:
            total = None

    def run(spinner_player, spinner_suffix):
        with cond_refresh:
            while thread:
                event_renderer.wait()
                alive_repr(term, next(spinner_player), spinner_suffix)
                cond_refresh.wait(1. / fps(run.rate))

    run.rate, run.init, run.elapsed, run.percent = 0., 0., 0., 0.
    run.count, run.processed, run.last_len = 0, 0, 0
    run.text, run.title, run.suffix, ctrl_c = None, None, None, False
    run.monitor_text, run.eta_text, run.rate_text = '?', '?', '?'

    if _testing:  # it's easier than trying to mock these internal values.
        run.elapsed = 1.23
        run.rate = 9876.54

        main_update_hook = _noop
    else:
        def main_update_hook():
            run.elapsed = time.perf_counter() - run.init
            run.rate = gen_rate.send((processed(), run.elapsed))

    def alive_repr(out, spinner=None, spinner_suffix=None):
        main_update_hook()

        fragments = (run.title, bar_repr(run.percent), bar_suffix, spinner, spinner_suffix,
                     monitor(), elapsed(), stats(), *run.text)

        run.last_len = print_cells(fragments, out.cols(), out, run.last_len)
        out.write(run.suffix)
        out.flush()

    def set_text(text=None):
        if text and config.dual_line:
            run.text, run.suffix = ('\n', to_cells(str(text))), term.cursor_up_1.sequence
        else:
            run.text, run.suffix = (to_cells(None if text is None else str(text)),), ''  # 1-tuple.

    def set_title(title=None):
        run.title = _render_title(config, None if title is None else str(title))
        if run.title:
            run.title += (' ',)  # space separator for print_cells.

    if config.manual:
        def bar(percent):  # for manual mode (with total or not).
            hook_manager.flush_buffers()  # notify that the current index is about to change.
            run.percent = max(0., float(percent))  # absolute value can't be negative.
            bar_update_hook()
    elif not total:
        def bar(count=1):  # for unknown mode, i.e. not manual and not total.
            hook_manager.flush_buffers()  # notify that the current index is about to change.
            run.count += int(count)  # relative value can be negative.
            run.count = max(0, run.count)  # but absolute value can't.
            bar_update_hook()
    else:
        def bar(count=1, *, skipped=False):  # for definite mode, i.e. not manual and with total.
            hook_manager.flush_buffers()  # notify that the current index is about to change.
            count = int(count)  # relative value can be negative.
            run.count += count
            run.count = max(0, run.count)  # but absolute value can't.
            if not skipped:
                run.processed += count
                run.processed = max(0, run.processed)  # but absolute value can't.
            bar_update_hook()

    def start_monitoring(offset=0.):
        term.hide_cursor()
        hook_manager.install()
        bar_handle._handle = bar
        run.init = time.perf_counter() - offset
        event_renderer.set()

    def stop_monitoring():
        term.show_cursor()
        hook_manager.uninstall()
        bar_handle._handle = None
        return time.perf_counter() - run.init

    @contextmanager
    def pause_monitoring():
        event_renderer.clear()
        offset = stop_monitoring()
        alive_repr(term)
        term.write('\n')
        term.flush()
        try:
            yield
        finally:
            start_monitoring(offset)

    if total or not config.manual:  # we can count items.
        logic_total, current = total, lambda: run.count
        unit, factor, header = config.unit, 1.e6, 'on {:d}: '
    else:  # there's only a manual percentage.
        logic_total, current = 1., lambda: run.percent
        unit, factor, header = f'%{config.unit}', 1., 'on {:.1%}: '
    processed = (lambda: run.processed) if total and not config.manual else current

    thread, event_renderer, cond_refresh = None, threading.Event(), _cond()
    bar_repr, bar_suffix = _create_bars(config)
    fps = (custom_fps(config.refresh_secs) if config.refresh_secs
           else calibrated_fps(calibrate or factor))
    gen_rate = gen_simple_exponential_smoothing(.3, lambda pos, elapse: pos / elapse)
    gen_rate.send(None)

    if config.disable:
        term, hook_manager = terminal.get_void(), passthrough_hook_manager()
    else:
        term = terminal.get_term(config.file, config.force_tty, config.max_cols)
        hook_manager = buffered_hook_manager(header if config.enrich_print else '',
                                             current, config.enrich_offset, cond_refresh, term)

    if term.interactive:
        thread = threading.Thread(target=run, args=_create_spinner_player(config))
        thread.daemon = True
        thread.start()

    if not config.scale:
        def human_count(value, _precision=None):
            return f'{value}{config.unit}'

        def rate_text(precision):
            return f'{run.rate:.{precision}f}{unit}/s'
    else:
        import about_time  # must not be on top.
        d1024, iec = {
            'SI': (False, False),
            'SI2': (True, False),
            'IEC': (True, True),
        }[config.scale]
        fn_human_count = about_time.human_count.fn_human_count(False, d1024, iec)
        fn_human_throughput = about_time.human_throughput.fn_human_throughput(False, d1024, iec)

        def human_count(value, precision=None):
            return fn_human_count(value, unit, precision)

        def rate_text(precision):
            return fn_human_throughput(run.rate, unit, precision)

    def monitor_run(f, precision=config.precision):
        run.monitor_text = human_count(run.count, precision)
        return f.format(count=run.monitor_text, total=total_human, percent=run.percent)

    def monitor_end(f):
        warning = '(!) ' if total is not None and current() != logic_total else ''
        return f'{warning}{monitor_run(f, None)}'

    def elapsed_run(f):
        return f.format(elapsed=time_display(run.elapsed, RUN))

    def elapsed_end(f):
        return f.format(elapsed=time_display(run.elapsed, END))

    def stats_end(f):
        run.rate_text = rate_text(2)
        return f.format(rate=run.rate_text, unit=unit)

    if total or config.manual:  # we can track progress and therefore eta.
        def stats_run(f):
            run.rate_text = rate_text(1)  # although repeated below,
            run.eta_text = eta_text(gen_eta.send((current(), run.rate)))
            return f.format(rate=run.rate_text, unit=unit, eta=run.eta_text)

        gen_eta = gen_simple_exponential_smoothing(.5, fn_simple_eta(logic_total))
        gen_eta.send(None)
        stats_default = '({eta}, {rate})'
    else:  # unknown progress.
        def stats_run(f):
            run.rate_text = rate_text(1)  # it won't be calculated if not needed.
            return f.format(rate=run.rate_text, eta='?')

        bar_repr = bar_repr.unknown
        stats_default = '({rate})'

    if total:
        if config.manual:
            monitor_default = '{percent:.0%} [{count}/{total}]'

            def bar_update_hook():
                run.count = math.ceil(run.percent * total)
        else:
            monitor_default = '{count}/{total} [{percent:.0%}]'

            def bar_update_hook():
                run.percent = run.count / total
    else:
        def bar_update_hook():
            pass

        if config.manual:
            monitor_default = '{percent:.0%}'
        else:
            monitor_default = '{count}'

    total_human = human_count(total or 0)  # avoid converting it on all refreshes.

    monitor = _Widget(monitor_run, config.monitor, monitor_default)
    monitor_end = _Widget(monitor_end, config.monitor_end, monitor.f[:-1])  # space separator.
    elapsed = _Widget(elapsed_run, config.elapsed, 'in {elapsed}')
    elapsed_end = _Widget(elapsed_end, config.elapsed_end, elapsed.f[:-1])  # space separator.
    stats = _Widget(stats_run, config.stats, stats_default)
    stats_end = _Widget(stats_end, config.stats_end, '({rate})' if stats.f[:-1] else '')

    def get_receipt():
        buffer = io.StringIO()
        tbuf = terminal.get_term(buffer, True, 1000)  # large enough to not truncate.
        run.last_len = 0  # prevents the inclusion of the clear end line escape sequence.
        alive_repr(tbuf)
        return buffer.getvalue().strip()

    bar_handle = __AliveBarHandle(pause_monitoring, set_title, set_text,
                                  current, lambda: run.monitor_text, lambda: run.rate_text,
                                  lambda: run.eta_text, lambda: run.elapsed, get_receipt)
    set_text(), set_title()
    start_monitoring()
    try:
        yield bar_handle if not _sampling else locals()
    except KeyboardInterrupt:
        ctrl_c = True
        if config.ctrl_c:
            raise
    finally:
        stop_monitoring()
        if thread:  # lets the internal thread terminate gracefully.
            local_copy, thread = thread, None
            local_copy.join()

        # guarantees last_len is already set...
        if ctrl_c and term.cols() - run.last_len < 2:
            term.cursor_up_1()  # try to not duplicate last line when terminal prints "^C".

        if config.receipt:  # prints the nice but optional final receipt.
            elapsed, stats, monitor = elapsed_end, stats_end, monitor_end
            bar_repr, run.suffix = bar_repr.end, ''
            if not config.receipt_text:
                set_text()
            term.clear_end_screen()
            alive_repr(term)
            term.write('\n')
        else:
            term.clear_line()
        main_update_hook = _noop  # freeze the final elapsed, rate and eta values.
        term.flush()


class _Widget:  # pragma: no cover
    def __init__(self, func, value, default):
        self.func = func
        if isinstance(value, str):
            self.f = value
        elif value:
            self.f = default
        else:
            self.f = ''

        if self.f:
            self.f += ' '  # space separator for print_cells.

    def __call__(self):
        return self.func(self.f)


class _ReadOnlyProperty:  # pragma: no cover
    """A descriptor that provides a read-only property, which calls a getter function."""

    def __set_name__(self, owner, name):
        self.prop = f'_{name}'

    def __get__(self, obj, objtype=None):
        return getattr(obj, self.prop)()

    def __set__(self, obj, value):
        raise AttributeError(f'Cannot set "{self.prop[1:]}"')


class _GatedFunction(_ReadOnlyProperty):  # pragma: no cover
    """A gated descriptor that provides a function only while the bar is running."""

    def __get__(self, obj, objtype=None):
        if obj._handle:
            return getattr(obj, self.prop)
        return _noop


class _Function(_ReadOnlyProperty):  # pragma: no cover
    """An ungated descriptor that provides a function even after the bar has finished."""

    def __get__(self, obj, objtype=None):
        return getattr(obj, self.prop)


class _AssignFunction(_Function):  # pragma: no cover
    """An ungated descriptor that provides a setter function even after the bar has finished."""

    def __set__(self, obj, value):
        self.__get__(obj)(value)


class __AliveBarHandle:
    pause = _GatedFunction()
    current = _ReadOnlyProperty()
    text = _AssignFunction()
    title = _AssignFunction()
    monitor = _ReadOnlyProperty()
    rate = _ReadOnlyProperty()
    eta = _ReadOnlyProperty()
    elapsed = _ReadOnlyProperty()
    receipt = _Function()

    def __init__(self, pause, set_title, set_text, get_current, get_monitor, get_rate, get_eta,
                 get_elapsed, get_receipt):
        self._handle, self._pause, self._current = None, pause, get_current
        self._title, self._text = set_title, set_text
        self._monitor, self._rate, self._eta = get_monitor, get_rate, get_eta
        self._elapsed, self._receipt = get_elapsed, get_receipt

    # support for disabling the bar() implementation.
    def __call__(self, *args, **kwargs):
        if self._handle:
            self._handle(*args, **kwargs)


def _noop(*_args, **_kwargs):  # pragma: no cover
    pass


def _create_bars(config):
    bar = config.bar
    if bar is None:
        def obj(*_args, **_kwargs):
            pass

        obj.unknown, obj.end = obj, obj
        return obj, ''

    return bar(config.length, config.unknown), ' '


def _create_spinner_player(config):
    spinner = config.spinner
    if spinner is None:
        from itertools import repeat
        return repeat(''), ''

    from ..animations.utils import spinner_player
    return spinner_player(spinner(config.spinner_length)), ' '


def _render_title(config, title=None):
    title, length = to_cells(title is None and config.title or title), config.title_length
    if not length:
        return title

    len_title = len(title)
    if len_title <= length:
        # fixed left align implementation for now, there may be more in the future, like
        # other alignments, variable with a maximum size, and even scrolling and bouncing.
        return combine_cells(title, (' ',) * (length - len_title))

    if length == 1:
        return '…',  # 1-tuple

    return combine_cells(fix_cells(title[:length - 1]), ('…',))


T = TypeVar('T')


def alive_it(it: Collection[T], total: Optional[int] = None, *,
             finalize: Callable[[Any], None] = None,
             calibrate: Optional[int] = None, **options: Any) -> Iterable[T]:
    """New iterator adapter in 2.0, which makes it simpler to monitor any processing.

    Simply wrap your iterable with `alive_it`, and process your items normally!
    >>> from alive_progress import alive_it
    ...
    ... items = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
    ... for item in alive_it(items):
    ...     # process item.

    And the bar will just work, it's that simple!

    All `alive_bar` parameters apply as usual, except `total` (which is smarter: if not supplied
    it will be inferred from the iterable using len or length_hint), and `manual` (which can't
    be used in this mode at all).
    To force unknown mode, even when the total would be available, send `total=0`.

    If you want to use other alive_bar's more advanced features, like for example setting
    situational messages, you can simply assign it to a variable.

    >>> from alive_progress import alive_it
    ...
    ... items = range(100000)
    ... bar = alive_it(items)
    ... for item in bar:
    ...     bar.text = f'Wow, it works! Item: {item}'
    ...     # process item.

    You can also send a `finalize` function to set the final receipt title and text, and any other
    alive_bar options you'd like!

    >>> from alive_progress import alive_it
    ...
    ... def ending(bar):
    ...     bar.title = 'DB updated'
    ...     bar.text = f'{bar.current} entries changed'
    ...
    ... items = range(100000)
    ... for item in alive_it(items, finalize=ending, length=20, receipt_text=True)
    ...     # process item.

    This prints:
DB updated |████████████████████| 100k/100k [100%] in 2.6s (38.7k/s) 100000 entries changed

    Args:
        it: the input iterable to be processed
        total: same as alive_bar
        finalize: a function to be called when the bar is going to finalize
        calibrate: same as alive_bar
        options: same as alive_bar

    See Also:
        alive_bar

    Returns:
        Generator

    """
    try:
        config = config_handler(**options)
    except Exception as e:
        raise type(e)(str(e)) from None
    if config.manual:
        raise UserWarning("Manual mode can't be used in iterator adapter.")

    if total is None and hasattr(it, '__len__'):
        total = len(it)
    it = iter(it)
    if total is None and hasattr(it, '__length_hint__'):
        total = it.__length_hint__()
    return __AliveBarIteratorAdapter(it, finalize, __alive_bar(config, total, calibrate=calibrate))


class __AliveBarIteratorAdapter(Iterable[T]):
    def __init__(self, it, finalize, inner_bar):
        self._it, self._finalize, self._inner_bar = it, finalize, inner_bar

    def __iter__(self):
        if '_bar' in self.__dict__:  # this iterator has already initiated.
            return

        with self._inner_bar as self._bar:
            del self._inner_bar
            for item in self._it:
                yield item
                self._bar()
            if self._finalize:
                self._finalize(self._bar)

    def __call__(self, *args, **kwargs):
        raise UserWarning('The bar position is controlled automatically by `alive_it`.')

    def __getattr__(self, item):
        # makes this adapter work as the real bar.
        if '_bar' in self.__dict__:  # detects not yet started bar instances.
            return getattr(self._bar, item)
        raise UserWarning('Configure this bar either via `alive_it()` or after iterating it.')

    def __setattr__(self, key, value):
        # makes this adapter work as the real bar.
        if '_bar' in self.__dict__:
            return setattr(self._bar, key, value)
        return super().__setattr__(key, value)
