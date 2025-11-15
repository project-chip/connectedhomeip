import os
import random
import re
import sys
import time
from collections import namedtuple
from enum import Enum

from .internal import BARS, SPINNERS, THEMES
from ..animations.spinners import scrolling_spinner_factory, sequential_spinner_factory
from ..animations.utils import spinner_player
from ..core.configuration import config_handler
from ..utils.cells import print_cells
from ..utils import terminal

Show = Enum('Show', 'SPINNERS BARS THEMES')


def showtime(show=Show.SPINNERS, *, fps=None, length=None, pattern=None):
    """Start a show, rendering all styles simultaneously in your screen.

    Args:
        fps (float): the desired frames per second refresh rate
        show (Show): chooses which show will run
        length (int): the bar length, as in configuration options
        pattern (Pattern): to filter objects displayed

    """
    show_funcs = {
        Show.SPINNERS: show_spinners,
        Show.BARS: show_bars,
        Show.THEMES: show_themes,
    }
    assert show in show_funcs, 'Which show do you want? We have Show.SPINNERS, Show.BARS, and ' \
                               'Show.THEMES.'
    show_funcs[show](fps=fps, length=length, pattern=pattern)


Info = namedtuple('Info', 'title descr tech')


def show_spinners(*, fps=None, length=None, pattern=None):
    """Start a spinner show, rendering all styles simultaneously in your screen.

    Args:
        fps (float): the desired frames per second rendition
        length (int): the bar length, as in configuration options
        pattern (Pattern): to filter objects displayed

    """
    selected = _filter(SPINNERS, pattern)
    max_name_length = max(len(s) for s in selected) + 2
    max_natural = max(s.natural for s in selected.values()) + 2
    gens = [_spinner_gen(f'{k:^{max_name_length}}', s, max_natural) for k, s in selected.items()]
    info = Info(
        title=('Spinners', 'including their unknown bar renditions'),
        descr=('Spinners generate and run fluid animations, with a plethora of special effects,'
               ' including static frames, scrolling, bouncing, sequential, alongside or delayed!',
               'Each type supports several customization options that allow some very cool tricks,'
               ' so be creative 😜'),
        tech=('Spinners are advanced generators that dynamically output frames to generate some '
              'effect.',
              'These frames are gathered into full cycles, where the spinner yields. This enables'
              ' to mix and match them, without ever breaking animations.',
              'All spinners compile their full animations only once before displaying, so they are'
              ' faaaast!',
              'The spinner compiler brings the super cool `.check()` tool, check it out!',
              'A spinner have a specific "natural" length, and know how to spread its contents over'
              ' any desired space.',))
    _showtime_gen(fps, gens, info, length)


def show_bars(*, fps=None, length=None, pattern=None):
    """Start a bar show, rendering all styles simultaneously in your screen.

    Args:
        fps (float): the desired frames per second rendition
        length (int): the bar length, as in configuration options
        pattern (Pattern): to filter objects displayed

    """
    selected = _filter(BARS, pattern)
    max_name_length = max(len(s) for s in selected) + 2
    gens = [_bar_gen(f'{k:>{max_name_length}}', b) for k, b in selected.items()]
    info = Info(
        title=('Bars', 'playing all their hidden tricks'),
        descr=('A bar can render any percentage with a plethora of effects, including dynamic'
               ' chars, tips, backgrounds, transparent fills, underflows and overflows!',
               'Bars also support some advanced use cases, which do not go only forward...'
               ' Just use manual mode and be creative 😜'),
        tech=('Bars are advanced closures that render percentages with some effect, in a specific'
              ' fixed length.',
              'Bars are not compiled, but support the super cool `.check()` tool, check it out!',
              'Furthermore, bars can render any external spinners inside its own borders.'))
    _showtime_gen(fps, gens, info, length)


def show_themes(*, fps=None, length=None, pattern=None):
    """Start a theme show, rendering all styles simultaneously in your screen.

    Args:
        fps (float): the desired frames per second rendition
        length (int): the bar length, as in configuration options
        pattern (Pattern): to filter objects displayed

    """
    selected = _filter(THEMES, pattern)
    max_name_length = max(len(s) for s in selected) + 2
    themes = {k: config_handler(**v) for k, v in selected.items()}
    max_natural = max(t.spinner.natural for t in themes.values())
    gens = [_theme_gen(f'{k:>{max_name_length}}', c, max_natural) for k, c in themes.items()]
    info = Info(
        title=('Themes', 'featuring their bar, spinner and unknown bar companions'),
        descr=('A theme is an aggregator, it wraps styles that go well together.',),
        tech=('Themes are syntactic sugar, not actually configuration variables (they are elided'
              ' upon usage, only their contents go into the config).',
              'But you can surely customize them, just send any additional config parameters to'
              ' override anything.'))
    _showtime_gen(fps, gens, info, length)


def _filter(source, pattern):
    p = re.compile(pattern or '')
    selected = {k: v for k, v in source.items() if p.search(k)}
    if not selected:
        raise ValueError(f'Nothing was selected with pattern "{pattern}".')
    return selected


# unfortunately these cool infos increase considerably the animations' lengths, and I think it
# would be ugly to just truncate the animated gifs on the README to try to keep them within
# reasonable sizes (I can't afford to increase them, since they're very large already).
# so they will be kind of an easter egg, showing only on users' machines ;)
_INFO = os.getenv('ALIVE_BAR_EXHIBIT_FULL_INFO', '1') != '0'


def _showtime_gen(fps, gens, info, length):
    if not sys.stdout.isatty():
        raise UserWarning('This must be run on a tty connected terminal.')

    def title(t, r=False):
        return scrolling_spinner_factory(t, right=r, wrap=False).pause(center=12),  # 1-tuple.

    def message(m, s=None):
        return scrolling_spinner_factory(f'{m} 👏, {s}!' if s else m, right=False),  # 1-tuple.

    info_spinners = sequential_spinner_factory(
        *(title('Now on stage...')
          + message(*info.title)
          + sum((message(d) for d in info.descr), ())
          + title('Technical details')
          + sum((message(d) for d in info.tech), ())
          + title('Enjoy 🤩', True)),
        intermix=False
    )

    # initialize generators, retrieve their line lengths, and create information line.
    fps, length = min(60., max(2., float(fps or 15.))), length or 40
    cols = max(x for _, x in ((next(gen), gen.send((fps, length))) for gen in gens))
    fps_monitor = 'fps: {:.1f}'
    info_player = spinner_player(info_spinners(max(3, cols - len(fps_monitor.format(fps)) - 1)))

    logo = spinner_player(SPINNERS['waves']())
    start, sleep, frame, line_num = time.perf_counter(), 1. / fps, 0, 0
    start, current = start - sleep, start  # simulates the first frame took exactly "sleep" ms.
    term = terminal.get_term()
    term.hide_cursor()
    try:
        while True:
            cols, lines = os.get_terminal_size()

            title = 'Welcome to alive-progress!', ' ', next(logo)
            print_cells(title, cols, term)  # line 1.
            term.clear_end_line()
            print()

            info = fps_monitor.format(frame / (current - start)), ' ', next(info_player)
            print_cells(info, cols, term)  # line 2.
            term.clear_end_line()

            content = [next(gen) for gen in gens]  # always consume gens, to maintain them in sync.
            for line_num, fragments in enumerate(content, 3):
                if line_num > lines:
                    break
                print()
                print_cells(fragments, cols, term)
                term.clear_end_line()

            frame += 1
            current = time.perf_counter()
            time.sleep(max(0., start + frame * sleep - current))
            print(f'\x1b[{line_num - 1}A', end='\r')  # ANSI escape sequence for Cursor Up.
    except KeyboardInterrupt:
        pass
    finally:
        term.show_cursor()


def _spinner_gen(name, spinner_factory, max_natural):
    fps, length = yield
    blanks = (' ',) * (max_natural - spinner_factory.natural)
    spinner_gen = exhibit_spinner(spinner_factory())
    unknown_gen = exhibit_spinner(spinner_factory(length))
    yield len(blanks) + spinner_factory.natural + len(name) + length + 4 + 2  # borders/spaces.
    while True:
        yield (blanks, '|', next(spinner_gen), '| ', name, ' |', next(unknown_gen), '|')


def exhibit_spinner(spinner):
    player = spinner_player(spinner)
    while True:
        yield next(player)


def _bar_gen(name, bar_factory):
    fps, length = yield
    bar_gen = exhibit_bar(bar_factory(length), fps)
    yield len(name) + length + 2 + 1  # borders/spaces.
    while True:
        yield name, ' ', next(bar_gen)[0]


def exhibit_bar(bar, fps):
    total = int(fps * 5)
    while True:
        # standard use cases, increment till completion, underflow and overflow.
        for s, t in (0, total), (0, int(total * .5)), (int(total * .5), int(total + 1)):
            for pos in range(s, t):
                percent = pos / total
                yield bar(percent), percent
            # generates a small pause in movement between cases, based on the fps.
            percent = t / total
            for _ in range(int(fps * 2)):
                yield bar.end(percent), percent

        # advanced use cases, which do not go always forward.
        factor = random.random() + 1  # a number between 1.0 and 2.0.
        for percent in (1. - x * factor / total for x in range(total)):
            yield bar(percent), percent
        # generates a small giggle, like a real gauge.
        measure, giggle = random.random(), lambda: (random.random() - .5) * .2
        for _ in range(int(fps * 2)):
            percent = measure + giggle()
            yield bar(percent), percent
        # gradually comes to a full stop.
        for t in range(int(fps * 5)):  # smoother stabilization.
            percent = measure + giggle() / 1.04 ** t
            yield bar(percent), percent
        # enjoy the full stop for a while.
        for t in range(int(fps * 2)):
            yield bar(measure), measure


def _theme_gen(name, config, max_natural):
    fps, length = yield
    bar = config.bar(length, config.unknown)
    bar_std = exhibit_bar(bar, fps)
    bar_unknown = exhibit_bar(bar.unknown, fps)
    blanks = (' ',) * (max_natural - config.spinner.natural)
    spinner = exhibit_spinner(config.spinner())
    yield len(name) + 2 * length + max_natural + 4 + 3  # borders/spaces.
    while True:
        yield (name, ' ', next(bar_std)[0], ' ', next(spinner), blanks, ' ', next(bar_unknown)[0])
