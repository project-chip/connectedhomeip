"""
Implements support for grapheme clusters and cells (columns on screen).
Graphemes are sequences of codepoints, which are interpreted together based on the Unicode
standard. Grapheme clusters are sequences of graphemes, glued together by Zero Width Joiners.
These graphemes may occupy one or two cells on screen, depending on their glyph size.

Support for these cool chars, like Emojis 😃, was so damn hard to implement because:
1. Python don't know chars that occupy two columns on screen, nor grapheme clusters that are
    rendered as a single char (wide or not), it only understands codepoints;
2. Alive-progress needs to visually align all frames, to keep its progress bars' lengths from
    spiking up and down while running. For this I must somehow know which chars are wide and
    counterbalance them;
3. To generate all those cool animations, I need several basic operations, like len, iterating,
    indexing, slicing, concatenating and reversing, which suddenly don't work anymore, since they
    do not know anything about these new concepts of graphemes and cells! Argh.
4. As the first step, I needed to parse the codepoints into Unicode graphemes. I tried to parse them
    myself, but soon realized it was tricky and finicky, in addition to changing every year...
5. Then I looked into some lib dependencies, tested several, created the validate tool to help me
    test some Unicode versions, and chose one lib to use;
6. I finally implemented the operations I needed, to the best of my current knowledge, but it
    still wouldn't work. So I tried several spinners to check their alignments, until I finally
    realized what was wrong: I actually needed to align cells, not lengths nor even graphemes!

    Look this for example: Note that in your editor both strings below are perfectly aligned,
    although they have 6 and 16 as their Python lengths!!! How come?
    Graphemes didn't help either, 6 and 3 respectively... Then how does the editor know that they
    align? I'm not sure exactly, but I created this "cell" concept to map this into, and finally
    they both have the same: 6 cells!! 💡😜

        string \\ length  python  graphemes  cells
             nonono          6        6        6
             🏴󠁧󠁢󠁥󠁮󠁧󠁿👉🏾🏴󠁧󠁢󠁥󠁮󠁧󠁿          16       3        6

7. With that knowledge, I implemented "wide" marks on graphemes (so I could know whether a grapheme
    glyph would occupy 1 or 2 cells on screen), and refactored all needed operations. It seemed fine
    but still didn't work... I then realized that my animations made those wide chars dynamically
    enter and leave the frame, which can split strings AT ANY POINT, even between the two cells of
    wide-graphemes, yikes!!! To make the animations as fluid as always, I had to continue moving
    only one cell per tick time, so somehow I would have to draw "half" flags and "half" smiling-
    face-with-smiling-eyes!!
8. So, I had to support printing "half-graphemes", so I could produce frames in an animation with
    always the same sizes!! This has led me to implement a fixer for dynamically broken graphemes,
    which detects whether the head or tail cells were missing, and inserted a space in its place!
9. It worked! But I would have to run that algorithm throughout the whole animation, in any and all
    displayed frame, in real time... I feared for the performance.
    I needed something that could cache and "see" all the frames at once, so I could equalize their
    sizes only once!! So I created the cool spinner compiler, an ingenious piece of software that
    generates the entire animation ahead of time, fixes all the frames, and leverages a super light
    and fast runner, which is able to "play" this compiled artifact!!
10. Finally, I refactored the frame spinner factory, the simplest one to test the idea, and WOW...
    It worked!!! The joy of success filled me..........
11. To make the others work, I created the check tool, another ingenious software, which allowed me
    to "see" a spinner's contents, in a tabular way, directly from the compiled data! Then I could
    visually ensure whether ALL generated frames of ALL animations I could think of, had the exact
    same size;
12. A lot of time later, everything was working! But look at that, the spinner compiler has enabled
    me to make several improvements in the spinners' codes themselves, since it ended up gaining
    other cool functionalities like reshaping and transposing data, or randomizing anything playing!
    The concepts of "styling" and "operational" parameters got stronger with new commands, which
    enabled simpler compound animations, without any code duplication!
    And this has culminated in the creation of the newer sequential and alongside spinners, way more
    advanced than before, with configurations like intermixing and pivoting of cycles!
13. Then, it was time I moved on to the missing components in this new Cell Architecture: the bar,
    title, exhibit, and of course the alive_bar rendering itself... All of them needed to learn this
    new architecture: mainly change ordinary strings into tuples of cells (marked graphemes)...
14. And finally... Profit!!! Only no, this project only feels my soul, not my pocket...
    But what a ride! 😅

"""

import unicodedata

from . import sanitize

VS_15 = '\ufe0e'


def print_cells(fragments, cols, term, last_line_len=0):
    """Print a tuple of fragments of tuples of cells on the terminal, until a given number of
    cols is achieved, slicing over cells when needed.

    Spaces used to be inserted automatically between fragments, but not anymore.

    Args:
        fragments (Tuple[Union[str, Tuple[str, ...]]): the fragments of message
        cols (int): maximum columns to use
        term: the terminal to be used
        last_line_len (int): if the fragments fit within the last line, send a clear end line

    Returns:
        the number of actually used cols.

    """
    available = cols
    term.write(term.carriage_return)
    for fragment in filter(None, fragments):
        if fragment == '\n':
            term.clear_end_line(available)
            available = cols
        elif available == 0:
            continue
        else:
            length = len(fragment)
            if length <= available:
                available -= length
            else:
                available, fragment = 0, fix_cells(fragment[:available])

        term.write(join_cells(fragment))

    if last_line_len and cols - available < last_line_len:
        term.clear_end_line(available)

    return cols - available


def join_cells(fragment):
    """Beware, this looses the cell information, converting to a simple string again.
    Don't use unless it is a special case."""
    return ''.join(strip_marks(fragment))


def combine_cells(*fragments):
    """Combine several fragments of cells into one.
    Remember that the fragments get a space between them, so this is mainly to avoid it when
    not desired."""
    return sum(fragments, ())  # this is way faster than tuple(chain.from_iterable()).


def is_wide(g):
    """Try to detect wide chars.

    This is tricky, I've seen several graphemes that have Neutral width (and thus use one
    cell), but actually render as two cells, like shamrock and heart ☘️❤️.
    I've talked to George Nachman, the creator of iTerm2, which has explained to me [1] the fix
    would be to insert a space after these cases, but I can't possibly know if this
    behavior is spread among all terminals, it probably has to do with the Unicode version too,
    so I'm afraid of fixing it.
    Use the `alive_progress.tools.print_chars` tool, and check the section around `0x1f300`
    for more examples.

    [1]: https://gitlab.com/gnachman/iterm2/-/issues/9185

    Args:
        g (str): the grapheme sequence to be tested

    """
    return g[-1] != VS_15 and (len(g) > 1 or unicodedata.east_asian_width(g) in ('W', 'F'))


def fix_cells(chars):
    """Fix truncated cells, removing whole clusters when needed."""
    if not chars:
        return chars

    start = (' ',) if chars[0] is None else ()
    end = (' ',) if chars[-1] is not None and is_wide(chars[-1]) else ()
    return (*start, *chars[bool(start):-1 if end else None], *end)  # noqa


def to_cells(text):
    return mark_graphemes(split_graphemes(sanitize(text)))


def split_graphemes(text):
    from grapheme import graphemes
    return tuple(graphemes(text))


def mark_graphemes(gs):
    return sum(((g, *((None,) if is_wide(g) else ())) for g in gs), ())


def strip_marks(chars):
    return (c for c in chars if c)


def has_wide(text):
    return any(is_wide(x) for x in text)
