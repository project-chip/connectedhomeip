from types import SimpleNamespace


def get_from(parent):
    def cols():
        # it seems both `jupyter notebook` and `jupyter-lab` do not return cols, only 80 default.
        return parent.max_cols

    def clear_line():
        write(_clear_line)
        flush()

    def clear_end_line(available=None):
        for _ in range(available or 0):
            write(' ')
        flush()

    clear_end_screen = clear_end_line

    # it seems spaces are appropriately handled to not wrap lines.
    _clear_line = f'\r{" " * cols()}\r'

    from .void import factory_cursor_up, hide_cursor, show_cursor  # noqa

    flush = parent.flush
    write = parent.write
    carriage_return = parent.carriage_return

    return SimpleNamespace(**locals())
