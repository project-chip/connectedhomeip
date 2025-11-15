"""
Used for locating a file in the data directory.
"""

from importlib.resources import files

def data_filename(filename):
    """
    Given a relative filename, get the full path to that file in the data
    directory.
    """
    return files('language_data') / 'data' / filename
