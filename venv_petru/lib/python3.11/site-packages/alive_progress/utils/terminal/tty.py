import os
from types import SimpleNamespace


def new(original, max_cols):
    write = original.write
    flush = original.flush

    try:
        _fd = original.fileno()
    except OSError:
        _fd = 1

    def cols():
        try:
            return os.get_terminal_size(_fd)[0]
        except (ValueError, OSError):
            # original is closed, detached, or not a terminal, or
            # os.get_terminal_size() is unsupported
            return max_cols

    def _ansi_escape_sequence(code, param=''):
        def inner(_available=None):  # because of jupyter.
            write(inner.sequence)

        inner.sequence = f'\x1b[{param}{code}'
        return inner

    def factory_cursor_up(num):
        return _ansi_escape_sequence('A', num)  # sends cursor up: CSI {x}A.

    clear_line = _ansi_escape_sequence('2K\r')  # clears the entire line: CSI n K -> with n=2.
    clear_end_line = _ansi_escape_sequence('K')  # clears line from cursor: CSI K.
    clear_end_screen = _ansi_escape_sequence('J')  # clears screen from cursor: CSI J.
    hide_cursor = _ansi_escape_sequence('?25l')  # hides the cursor: CSI ? 25 l.
    show_cursor = _ansi_escape_sequence('?25h')  # shows the cursor: CSI ? 25 h.
    carriage_return = '\r'

    return SimpleNamespace(**locals())
