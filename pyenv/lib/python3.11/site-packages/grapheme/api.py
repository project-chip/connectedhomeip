from grapheme.finder import GraphemeIterator, get_last_certain_break_index

UNICODE_VERSION = "16.0.0"


def graphemes(string):
    """
    Returns an iterator of all graphemes of given string.

    >>> rainbow_flag = "🏳️‍🌈"
    >>> [codepoint for codepoint in rainbow_flag]
    ['🏳', '️', '\u200d', '🌈']
    >>> list(grapheme.graphemes("multi codepoint: " + rainbow_flag))
    ['m', 'u', 'l', 't', 'i', ' ', 'c', 'o', 'd', 'e', 'p', 'o', 'i', 'n', 't', ':', ' ', '🏳️‍🌈']
    """
    return iter(GraphemeIterator(string))


def length(string, until=None):
    """
    Returns the number of graphemes in the string.

    Note that this functions needs to traverse the full string to calculate the length,
    unlike `len(string)` and it's time consumption is linear to the length of the string
    (up to the `until` value).

    Only counts up to the `until` argument, if given. This is useful when testing
    the length of a string against some limit and the excess length is not interesting.

    >>> rainbow_flag = "🏳️‍🌈"
    >>> len(rainbow_flag)
    4
    >>> graphemes.length(rainbow_flag)
    1
    >>> graphemes.length("".join(str(i) for i in range(100)), 30)
    30
    """
    if until is None:
        return sum(1 for _ in GraphemeIterator(string))

    iterator = graphemes(string)
    count = 0
    while True:
        try:
            if count >= until:
                break
            next(iterator)
        except StopIteration:
            break
        else:
            count += 1

    return count


# TODO: should probably use an optimized iterator that only deals with code point counts
def grapheme_lengths(string):
    """
    Returns an iterator of number of code points in each grapheme of the string.
    """
    return iter(len(g) for g in graphemes(string))


def slice(string, start=None, end=None):
    """
    Returns a substring of the given string, counting graphemes instead of codepoints.

    Negative indices is currently not supported.
    >>> string = "tamil நி (ni)"

    >>> string[:7]
    'tamil ந'
    >>> grapheme.slice(string, end=7)
    'tamil நி'
    >>> string[7:]
    'ி (ni)'
    >>> grapheme.slice(string, 7)
    ' (ni)'
    """

    if start is None:
        start = 0
    if end is not None and start >= end:
        return ""

    if start < 0:
        raise NotImplementedError("Negative indexing is currently not supported.")

    sum_ = 0
    start_index = None
    for grapheme_index, grapheme_length in enumerate(grapheme_lengths(string)):
        if grapheme_index == start:
            start_index = sum_
        elif grapheme_index == end:
            return string[start_index:sum_]
        sum_ += grapheme_length

    if start_index is not None:
        return string[start_index:]

    return ""


def contains(string, substring):
    """
    Returns true if the sequence of graphemes in substring is also present in string.

    This differs from the normal python `in` operator, since the python operator will return
    true if the sequence of codepoints are withing the other string without considering
    grapheme boundaries.

    Performance notes: Very fast if `substring not in string`, since that also means that
    the same graphemes can not be in the two strings. Otherwise this function has linear time
    complexity in relation to the string length. It will traverse the sequence of graphemes until
    a match is found, so it will generally perform better for grapheme sequences that match early.

    >>> "🇸🇪" in "🇪🇸🇪🇪"
    True
    >>> grapheme.contains("🇪🇸🇪🇪", "🇸🇪")
    False
    """
    if substring not in string:
        return False

    substr_graphemes = list(graphemes(substring))

    if len(substr_graphemes) == 0:
        return True
    elif len(substr_graphemes) == 1:
        return substr_graphemes[0] in graphemes(string)
    else:
        str_iter = graphemes(string)
        str_sub_part = []
        for _ in range(len(substr_graphemes)):
            try:
                str_sub_part.append(next(str_iter))
            except StopIteration:
                return False

        for g in str_iter:
            if str_sub_part == substr_graphemes:
                return True

            str_sub_part.append(g)
            str_sub_part.pop(0)
        return str_sub_part == substr_graphemes


def startswith(string, prefix):
    """
    Like str.startswith, but also checks that the string starts with the given prefixes sequence of
    graphemes.

    str.startswith may return true for a prefix that is not visually represented as a prefix if a
    grapheme cluster is continued after the prefix ends.

    >>> grapheme.startswith("✊🏾", "✊")
    False
    >>> "✊🏾".startswith("✊")
    True
    """
    return string.startswith(prefix) and safe_split_index(string, len(prefix)) == len(prefix)


def endswith(string, suffix):
    """
    Like str.endswith, but also checks that the string endswith the given prefixes sequence of
    graphemes.

    str.endswith may return true for a suffix that is not visually represented as a suffix if a
    grapheme cluster is initiated before the suffix starts.

    >>> grapheme.endswith("🏳️‍🌈", "🌈")
    False
    >>> "🏳️‍🌈".endswith("🌈")
    True
    """
    expected_index = len(string) - len(suffix)
    return string.endswith(suffix) and safe_split_index(string, expected_index) == expected_index


def safe_split_index(string, max_len):
    """
    Returns the highest index up to `max_len` at which the given string can be sliced,
    without breaking a grapheme.

    This is useful for when you want to split or take a substring from a string,
    and don't really care about
    the exact grapheme length, but don't want to risk breaking existing graphemes.

    This function does normally not traverse the full grapheme sequence up to the given length,
    so it can be used for arbitrarily long strings and high `max_len`.
    However, some grapheme boundaries depend on the previous state,
    so the worst case performance is O(n). In practice, it's only very long non-broken sequences
    of country flags (represented as Regional Indicators) that will perform badly.

    The return value will always be between `0` and `len(string)`.

    >>> string = "tamil நி (ni)"
    >>> i = grapheme.safe_split_index(string, 7)
    >>> i
    6
    >>> string[:i]
    'tamil '
    >>> string[i:]
    'நி (ni)'
    """
    last_index = get_last_certain_break_index(string, max_len)
    for i in grapheme_lengths(string[last_index:]):
        if last_index + i > max_len:
            break
        last_index += i
    return last_index
