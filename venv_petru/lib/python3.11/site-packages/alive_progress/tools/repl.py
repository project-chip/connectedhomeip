"""
Tools intended to be used inside a Python REPL, like ipython or python itself.
"""


def print_chars(line_length=32, max_char=0x20000):
    """Print all chars in the terminal, to help you find that cool one to put in your
    customized spinner or bar. Also useful to determine if your terminal do support them.

    Args:
        line_length (int): the desired characters per line
        max_char (int): the last character in the unicode table to show
            this goes up to 0x10ffff, but after the default value it seems to return
            only question marks, increase it if would like to see more.

    """
    max_char = min(0x10ffff, max(0, max_char))
    for i in range(0x20, max_char + line_length, line_length):
        print(f'0x{i:05x}', end=': ')
        for j in range(line_length):
            if j & 0xf == 0:
                print(' ', end='')
            try:
                print(chr(i + j), end=' ')
            except UnicodeEncodeError:
                print('?', end=' ')
        print()
