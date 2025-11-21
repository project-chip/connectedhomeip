import math
from itertools import chain

from .spinner_compiler import spinner_controller
from .utils import combinations, overlay_sliding_window, round_even, spinner_player, \
    split_options, spread_weighted, static_sliding_window
from ..utils.cells import combine_cells, fix_cells, has_wide, mark_graphemes, strip_marks, to_cells


def frame_spinner_factory(*frames):
    """Create a factory of a spinner that delivers frames in sequence, split by cycles.
    Supports unicode grapheme clusters and emoji chars (those that has length one but when on
    screen occupies two cells), as well as all other spinners.

    Args:
        frames (Union[str, Tuple[str, ...]): the frames to be displayed, split by cycles
            if sent only a string, it is interpreted as frames of a single char each.

    Returns:
        a styled spinner factory

    Examples:
        To define one cycle:
        >>> frame_spinner_factory(('cool',))  # only one frame.
        >>> frame_spinner_factory(('ooo', '---'))  # two frames.
        >>> frame_spinner_factory('|/_')  # three frames of one char each, same as below.
        >>> frame_spinner_factory(('|', '/', '_'))

        To define two cycles:
        >>> frame_spinner_factory(('super',), ('cool',))  # one frame each.
        >>> frame_spinner_factory(('ooo', '-'), ('vvv', '^'))  # two frames each.
        >>> frame_spinner_factory('|/_', '▁▄█')  # three frames each, same as below.
        >>> frame_spinner_factory(('|', '/', '_'), ('▁', '▄', '█'))

        Mix and match at will:
        >>> frame_spinner_factory(('oo', '-'), 'cool', ('it', 'is', 'alive!'))

    """
    # shortcut for single char animations.
    frames = (tuple(cycle) if isinstance(cycle, str) else cycle for cycle in frames)

    # support for unicode grapheme clusters and emoji chars.
    frames = tuple(tuple(to_cells(frame) for frame in cycle) for cycle in frames)

    @spinner_controller(natural=max(len(frame) for cycle in frames for frame in cycle))
    def inner_spinner_factory(actual_length=None):
        actual_length = actual_length or inner_spinner_factory.natural
        max_ratio = math.ceil(actual_length / min(len(frame) for cycle in frames
                                                  for frame in cycle))

        def frame_data(cycle):
            for frame in cycle:
                # differently sized frames and repeat support.
                yield (frame * max_ratio)[:actual_length]

        return (frame_data(cycle) for cycle in frames)

    return inner_spinner_factory


def scrolling_spinner_factory(chars, length=None, block=None, background=None, *,
                              right=True, hide=True, wrap=True, overlay=False):
    """Create a factory of a spinner that scrolls characters from one side to
    the other, configurable with various constraints.
    Supports unicode grapheme clusters and emoji chars, those that has length one but when on
    screen occupies two cells.

    Args:
        chars (str): the characters to be scrolled, either together or split in blocks
        length (Optional[int]): the natural length that should be used in the style
        block (Optional[int]): if defined, split chars in blocks with this size
        background (Optional[str]): the pattern to be used besides or underneath the animations
        right (bool): the scroll direction to animate
        hide (bool): controls whether the animation goes through the borders or not
        wrap (bool): makes the animation wrap borders or stop when not hiding.
        overlay (bool): fixes the background in place if overlay, scrolls it otherwise

    Returns:
        a styled spinner factory

    """
    assert not (overlay and not background), 'overlay needs a background'
    assert not (overlay and has_wide(background)), 'unsupported overlay with grapheme background'
    chars, rounder = to_cells(chars), round_even if has_wide(chars) else math.ceil

    @spinner_controller(natural=length or len(chars))
    def inner_spinner_factory(actual_length=None):
        actual_length = actual_length or inner_spinner_factory.natural
        ratio = actual_length / inner_spinner_factory.natural

        initial, block_size = 0, rounder((block or 0) * ratio) or len(chars)
        if hide:
            gap = actual_length
        else:
            gap = max(0, actual_length - block_size)
            if right:
                initial = -block_size if block else abs(actual_length - block_size)

        if block:
            def get_block(g):
                return fix_cells((mark_graphemes((g,)) * block_size)[:block_size])

            contents = map(get_block, strip_marks(reversed(chars) if right else chars))
        else:
            contents = (chars,)

        window_impl = overlay_sliding_window if overlay else static_sliding_window
        infinite_ribbon = window_impl(to_cells(background or ' '),
                                      gap, contents, actual_length, right, initial)

        def frame_data():
            for i, fill in zip(range(gap + block_size), infinite_ribbon):
                if i <= size:
                    yield fill

        size = gap + block_size if wrap or hide else abs(actual_length - block_size)
        cycles = len(tuple(strip_marks(chars))) if block else 1
        return (frame_data() for _ in range(cycles))

    return inner_spinner_factory


def bouncing_spinner_factory(chars, length=None, block=None, background=None, *,
                             right=True, hide=True, overlay=False):
    """Create a factory of a spinner that scrolls characters from one side to
    the other and bounce back, configurable with various constraints.
    Supports unicode grapheme clusters and emoji chars, those that has length one but when on
    screen occupies two cells.

    Args:
        chars (Union[str, Tuple[str, str]]): the characters to be scrolled, either
            together or split in blocks. Also accepts a tuple of two strings,
            which are used one in each direction.
        length (Optional[int]): the natural length that should be used in the style
        block (Union[int, Tuple[int, int], None]): if defined, split chars in blocks
        background (Optional[str]): the pattern to be used besides or underneath the animations
        right (bool): the scroll direction to start the animation
        hide (bool): controls whether the animation goes through the borders or not
        overlay (bool): fixes the background in place if overlay, scrolls it otherwise

    Returns:
        a styled spinner factory

    """
    chars_1, chars_2 = split_options(chars)
    block_1, block_2 = split_options(block)
    scroll_1 = scrolling_spinner_factory(chars_1, length, block_1, background, right=right,
                                         hide=hide, wrap=False, overlay=overlay)
    scroll_2 = scrolling_spinner_factory(chars_2, length, block_2, background, right=not right,
                                         hide=hide, wrap=False, overlay=overlay)
    return sequential_spinner_factory(scroll_1, scroll_2)


def sequential_spinner_factory(*spinner_factories, intermix=True):
    """Create a factory of a spinner that combines other spinners together, playing them
    one at a time sequentially, either intermixing their cycles or until depletion.

    Args:
        spinner_factories (spinner): the spinners to be combined
        intermix (bool): intermixes the cycles if True, generating all possible combinations;
            runs each one until depletion otherwise.

    Returns:
        a styled spinner factory

    """

    @spinner_controller(natural=max(factory.natural for factory in spinner_factories))
    def inner_spinner_factory(actual_length=None):
        actual_length = actual_length or inner_spinner_factory.natural
        spinners = [factory(actual_length) for factory in spinner_factories]

        def frame_data(spinner):
            yield from spinner()

        if intermix:
            cycles = combinations(spinner.cycles for spinner in spinners)
            gen = ((frame_data(spinner) for spinner in spinners)
                   for _ in range(cycles))
        else:
            gen = ((frame_data(spinner) for _ in range(spinner.cycles))
                   for spinner in spinners)

        return (c for c in chain.from_iterable(gen))  # transforms the chain to a gen exp.

    return inner_spinner_factory


def alongside_spinner_factory(*spinner_factories, pivot=None):
    """Create a factory of a spinner that combines other spinners together, playing them
    alongside simultaneously. Each one uses its own natural length, which is spread weighted
    to the available space.

    Args:
        spinner_factories (spinner): the spinners to be combined
        pivot (Optional[int]): the index of the spinner to dictate the animation cycles
            if None, the whole animation will be compiled into a unique cycle.

    Returns:
        a styled spinner factory

    """

    @spinner_controller(natural=sum(factory.natural for factory in spinner_factories))
    def inner_spinner_factory(actual_length=None, offset=0):
        if actual_length:
            lengths = spread_weighted(actual_length, [f.natural for f in spinner_factories])
            actual_pivot = None if pivot is None or not lengths[pivot] \
                else spinner_factories[pivot](lengths[pivot])
            spinners = [factory(length) for factory, length in
                        zip(spinner_factories, lengths) if length]
        else:
            actual_pivot = None if pivot is None else spinner_factories[pivot]()
            spinners = [factory() for factory in spinner_factories]

        def frame_data(cycle_gen):
            yield from (combine_cells(*fragments) for _, *fragments in cycle_gen)

        frames = combinations(spinner.total_frames for spinner in spinners)
        spinners = [spinner_player(spinner) for spinner in spinners]
        [[next(player) for _ in range(i * offset)] for i, player in enumerate(spinners)]

        if actual_pivot is None:
            breaker, cycles = lambda: range(frames), 1
        else:
            breaker, cycles = lambda: actual_pivot(), \
                frames // actual_pivot.total_frames * actual_pivot.cycles
        return (frame_data(zip(breaker(), *spinners)) for _ in range(cycles))

    return inner_spinner_factory


def delayed_spinner_factory(spinner_factory, copies, offset=1, *, dynamic=True):
    """Create a factory of a spinner that combines itself several times alongside,
    with an increasing iteration offset on each one.

    Args:
        spinner_factory (spinner): the source spinner
        copies (int): the number of copies
        offset (int): the offset to be applied incrementally to each copy
        dynamic (bool): dynamically changes the number of copies based on available space

    Returns:
        a styled spinner factory

    """

    if not dynamic:
        factories = (spinner_factory,) * copies
        return alongside_spinner_factory(*factories, pivot=0).op(offset=offset)

    @spinner_controller(natural=spinner_factory.natural * copies, skip_compiler=True)
    def inner_spinner_factory(actual_length=None):
        n = math.ceil(actual_length / spinner_factory.natural) if actual_length else copies
        return delayed_spinner_factory(spinner_factory, n, offset, dynamic=False)(actual_length)

    return inner_spinner_factory
