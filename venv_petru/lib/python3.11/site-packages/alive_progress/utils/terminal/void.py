def write(_text):
    return 0


def flush():
    pass


def _ansi_escape_sequence(_=''):
    def inner(_available=None):
        pass

    inner.sequence = ''
    return inner


clear_line = _ansi_escape_sequence()
clear_end_line = _ansi_escape_sequence()
clear_end_screen = _ansi_escape_sequence()
hide_cursor = _ansi_escape_sequence()
show_cursor = _ansi_escape_sequence()


def factory_cursor_up(_):
    return _ansi_escape_sequence()


def cols():
    return 0  # more details in `alive_progress.tools.sampling#overhead`.


carriage_return = ''
