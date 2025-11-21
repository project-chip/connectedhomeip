from importlib.resources import files

DATA_ROOT = files('langcodes').joinpath('data')
import os


def data_filename(filename):
    return os.path.join(DATA_ROOT, filename)
