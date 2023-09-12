import time
from pathlib import Path
from typing import TextIO


def warn_if_file_not_growing() -> None:
    # TODO
    """Used to verify streamed files are growing in size and warn if not"""
    pass


def add_border(to_print: str) -> str:
    """Add star borders to important strings"""
    return '\n' + '*' * len(to_print) + '\n' + to_print


def create_file_timestamp() -> str:
    """Conventional file timestamp suffix"""
    return time.strftime("%Y%m%d_%H%M%S")


def create_standard_log_name(name: str, ext: str) -> str:
    """Returns the name argument wrapped as a standard log name"""
    ts = create_file_timestamp()
    return f'idt_{ts}_{name}.{ext}'


def safe_mkdir(dir_name: str) -> None:
    Path(dir_name).mkdir(parents=True, exist_ok=True)


def print_and_write(to_print: str, file: TextIO) -> None:
    print(to_print)
    file.write(to_print)


def border_print(to_print: str, important: bool = False) -> None:
    len_borders = 64
    border = f"\n{'_' * len_borders}\n"
    i_border = f"\n{'!' * len_borders}\n" if important else ""
    print(f"{border}{i_border}{to_print}{i_border}{border}")
