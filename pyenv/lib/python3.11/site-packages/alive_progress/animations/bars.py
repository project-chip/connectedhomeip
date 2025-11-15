import math
import time

from about_time import about_time

from .utils import bordered, extract_fill_graphemes, fix_signature, spinner_player
from ..utils import terminal
from ..utils.cells import VS_15, combine_cells, fix_cells, has_wide, is_wide, join_cells, \
    mark_graphemes, split_graphemes, strip_marks, to_cells
from ..utils.colors import BLUE, BLUE_BOLD, CYAN, DIM, GREEN, ORANGE, ORANGE_BOLD, RED, YELLOW_BOLD


def bar_factory(chars=None, *, tip=None, background=None, borders=None, errors=None):
    """Create a factory of a bar with the given styling parameters.
    Supports unicode grapheme clusters and emoji chars (those that has length one but when on
    screen occupies two cells).

    Now supports transparent fills! Just send a tip, and leave `chars` as None.
    Also tips are now considered for the 100%, which means it smoothly enters and exits the
    frame to get to 100%!! The effect is super cool, use a multi-char tip to see.

    Args:
        chars (Optional[str]): the sequence of increasing glyphs to fill the bar
            can be None for a transparent fill, unless tip is also None.
        tip (Optional[str): the tip in front of the bar
            can be None, unless chars is also None.
        background (Optional[str]): the pattern to be used underneath the bar
        borders (Optional[Union[str, Tuple[str, str]]): the pattern or patterns to be used
            before and after the bar
        errors (Optional[Union[str, Tuple[str, str]]): the pattern or patterns to be used
            when an underflow or overflow occurs

    Returns:
        a styled bar factory

    """

    @bar_controller
    def inner_bar_factory(length, spinner_factory=None):
        if chars:
            if is_wide(chars[-1]):  # previous chars can be anything.
                def fill_style(complete, filling):  # wide chars fill.
                    odd = bool(complete % 2)
                    fill = (None,) if odd != bool(filling) else ()  # odd XOR filling.
                    fill += (chars[-1], None) * int(complete / 2)  # already marked wide chars.
                    if filling and odd:
                        fill += mark_graphemes((chars[filling - 1],))
                    return fill
            else:  # previous chars cannot be wide.
                def fill_style(complete, filling):  # narrow chars fill.
                    fill = (chars[-1],) * complete  # unneeded marks here.
                    if filling:
                        fill += (chars[filling - 1],)  # no widies here.
                    return fill
        else:
            def fill_style(complete, filling):  # invisible fill.
                return fix_cells(padding[:complete + bool(filling)])

        def running(fill):
            return None, (fix_cells(padding[len(fill) + len_tip:]),)  # this is a 1-tuple.

        def ended(fill):
            border = None if len(fill) + len(underflow) <= length else underflow
            texts = *(() if border else (underflow,)), blanks
            return border, texts

        @bordered(borders, '||')
        def draw_known(apply_state, percent):
            virtual_fill = round(virtual_length * max(0., min(1., percent)))
            fill = fill_style(*divmod(virtual_fill, num_graphemes))
            border, texts = apply_state(fill)
            border = overflow if percent > 1. else None if percent == 1. else border
            return fix_cells(combine_cells(fill, tip, *texts)[len_tip:length + len_tip]), border

        if spinner_factory:
            @bordered(borders, '||')
            def draw_unknown(_percent=None):
                return next(player), None

            player = spinner_player(spinner_factory(length))
        else:
            draw_unknown = None

        padding = (' ',) * len_tip + background * math.ceil((length + len_tip) / len(background))
        virtual_length, blanks = num_graphemes * (length + len_tip), (' ',) * length
        return draw_known, running, ended, draw_unknown

    assert chars or tip, 'tip is mandatory for transparent bars'
    assert not (chars and not is_wide(chars[-1]) and has_wide(chars)), \
        'cannot use grapheme with a narrow last char'

    chars = split_graphemes(chars or '')  # the only one not yet marked.
    tip, background = (to_cells(x) for x in (tip, background or ' '))
    underflow, overflow = extract_fill_graphemes(errors, (f'⚠{VS_15}', f'✗{VS_15}'))
    num_graphemes, len_tip = len(chars) or 1, len(tip)
    return inner_bar_factory


def bar_controller(inner_bar_factory):
    def bar_assembler_factory(length, spinner_factory=None):
        """Assembles this bar into an actual bar renderer.

        Args:
            length (int): the bar rendition length (excluding the borders)
            spinner_factory (Optional[spinner_factory]): enable this bar to act in unknown mode

        Returns:
            a bar renderer

        """
        with about_time() as t_compile:
            draw_known, running, ended, draw_unknown = inner_bar_factory(length, spinner_factory)

        def draw(percent):
            return draw_known(running, percent)

        def draw_end(percent):
            return draw_known(ended, percent)

        def bar_check(*args, **kwargs):  # pragma: no cover
            return check(draw, t_compile, *args, **kwargs)

        draw.__dict__.update(
            end=draw_end, unknown=draw_unknown,
            check=fix_signature(bar_check, check, 2),
        )

        if draw_unknown:
            def draw_unknown_end(_percent=None):
                return draw_end(1.)

            draw_unknown.end = draw_unknown_end

        return draw

    def compile_and_check(*args, **kwargs):  # pragma: no cover
        """Compile this bar factory at some length, and..."""
        # since a bar does not have a natural length, I have to choose one...
        bar_assembler_factory(40).check(*args, **kwargs)  # noqa

    bar_assembler_factory.__dict__.update(
        check=fix_signature(compile_and_check, check, 2),
    )

    return bar_assembler_factory


def check(bar, t_compile, verbosity=0, *, steps=20):  # noqa  # pragma: no cover
    """Check the data, codepoints, and even the animation of this bar.

    Args:
        verbosity (int): change the verbosity level
                         0 for brief data only (default)
                               /                 \\
                              /           3 to include animation
                             /                      \\
            1 to unfold bar data   ----------   4 to unfold bar data
                            |                        |
            2 to reveal codepoints   --------   5 to reveal codepoints
        steps (int): number of steps to display the bar progress

    """
    verbosity = max(0, min(5, verbosity or 0))
    if verbosity in (1, 2, 4, 5):
        render_data(bar, verbosity in (2, 5), steps)
    else:
        spec_data(bar)  # spec_data here displays only brief data, shown only if not full.

    duration = t_compile.duration_human
    print(f'\nBar style compiled in: {GREEN(duration)}')
    print(f'(call {HELP_MSG[verbosity]})')

    if verbosity in (3, 4, 5):
        animate(bar)


def __check(p):
    return f'{BLUE(f".{check.__name__}(")}{BLUE_BOLD(p)}{BLUE(")")}'


SECTION = ORANGE_BOLD
HELP_MSG = {
    0: f'{__check(1)} to unfold bar data, or {__check(3)} to include animation',
    1: f'{__check(2)} to reveal codepoints, or {__check(4)} to include animation,'
       f' or {__check(0)} to fold up bar data',
    2: f'{__check(5)} to include animation, or {__check(1)} to hide codepoints',
    3: f'{__check(4)} to unfold bar data, or {__check(0)} to omit animation',
    4: f'{__check(5)} to reveal codepoints, or {__check(1)} to omit animation,'
       f' or {__check(3)} to fold up bar data',
    5: f'{__check(2)} to omit animation, or {__check(4)} to hide codepoints',
}


def spec_data(bar):  # pragma: no cover
    def info(field, p, b):
        return f'{YELLOW_BOLD(field, "<11")}: {" ".join(bar_repr(b, p)[1:])}'

    print(f'\n{SECTION("Brief bar data")}')
    print('\n'.join(info(n, p, bar) for n, p in (
        ('starting', 0.), ('in progress', .5), ('completed', 1.), ('overflow', 1.2)
    )))
    print(info('underflow', .5, bar.end))


def format_codepoints(frame):  # pragma: no cover
    codes = '|'.join((ORANGE if is_wide(g) else BLUE)(
        ' '.join(hex(ord(c)).replace('0x', '') for c in g)) for g in frame)
    return f" -> {RED(sum(len(fragment) for fragment in frame))}:[{codes}]"


def render_data(bar, show_codepoints, steps):  # pragma: no cover
    print(f'\n{SECTION("Full bar data")}', end='')
    codepoints = format_codepoints if show_codepoints else lambda _: ''
    for name, b in ('in progress', bar), ('completed', bar.end):
        print(f'\n{name}')
        for p in (x / steps for x in range(steps + 2)):
            frame, joined, perc = bar_repr(b, p)
            print(joined, perc, codepoints(frame))


def bar_repr(bar, p):  # pragma: no cover
    frame = tuple(strip_marks(bar(p)))
    return frame, ''.join(frame), DIM(f'{p:6.1%}')


def animate(bar):  # pragma: no cover
    print(f'\n{SECTION("Animation")}')
    from ..styles.exhibit import exhibit_bar
    bar_gen = exhibit_bar(bar, 15)
    term = terminal.get_term()
    term.hide_cursor()
    try:
        while True:
            rendition, percent = next(bar_gen)
            print(f'\r{join_cells(rendition)}', CYAN(max(0., percent), "6.1%"))
            print(DIM('(press CTRL+C to stop)'), end='')
            term.clear_end_line()
            time.sleep(1 / 15)
            term.cursor_up_1()
    except KeyboardInterrupt:
        pass
    finally:
        term.show_cursor()
