from .core import about_time
from .features import FEATURES
from .human_count import HumanCount
from .human_duration import HumanDuration
from .human_throughput import HumanThroughput

VERSION = (4, 2, 1)

__author__ = 'Rogério Sampaio de Almeida'
__email__ = 'rsalmei@gmail.com'
__version__ = '.'.join(map(str, VERSION))

__all__ = ('__author__', '__version__', 'about_time', 'HumanCount', 'HumanDuration',
           'HumanThroughput', 'FEATURES')
