import sys
from types import SimpleNamespace


def get_from(parent):
    def cols():
        return sys.maxsize  # do not truncate when there's no tty.

    from .void import clear_end_line, clear_end_screen, clear_line  # noqa
    from .void import factory_cursor_up, hide_cursor, show_cursor  # noqa

    flush = parent.flush
    write = parent.write
    carriage_return = ''

    return SimpleNamespace(**locals())
