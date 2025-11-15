from typing import Optional

from .features import FEATURES, conv_space

SI_1000_SPEC = ('', 'k', 'M', 'G', 'T', 'P', 'E', 'Z', 'Y')
SI_1024_SPEC = ('', 'K', 'M', 'G', 'T', 'P', 'E', 'Z', 'Y')
IEC_1024_SPEC = ('', 'Ki', 'Mi', 'Gi', 'Ti', 'Pi', 'Ei', 'Zi', 'Yi')
DECIMALS = [1, 1, 1, 2, 2, 2, 2, 2, 2]


def __human_count(val: float, unit: str, prec: Optional[int], space: str, divisor: int,
                  spec: tuple) -> str:
    for scale, dec in zip(spec, DECIMALS):
        r = round(val, dec)
        if r >= divisor:
            val /= divisor
            continue
        break
    else:
        r, scale = val, '+'

    if prec is not None:
        r = round(val, prec)
    elif r % 1. == 0.:
        prec = 0
    elif (r * 10.) % 1. == 0.:
        prec = 1
    else:
        prec = 2
    return '{:.{}f}{}{}{}'.format(r, prec, space, scale, unit)


def fn_human_count(space: bool, d1024: bool, iec: bool):
    def run(val: float, unit: str, prec: Optional[int]):
        return __human_count(val, unit, prec, space, divisor, spec)

    space = conv_space(space)
    divisor, spec = {
        (False, False): (1000, SI_1000_SPEC),
        (True, False): (1024, SI_1024_SPEC),
        (True, True): (1024, IEC_1024_SPEC),
        (False, True): (1024, IEC_1024_SPEC),  # invalid combination, which just returns the above.
    }[(d1024, iec)]
    return run


class HumanCount(object):
    def __init__(self, value, unit):
        assert value >= 0.
        self._value = value
        self._unit = unit

    @property
    def value(self):
        return self._value

    def unit(self, value: str) -> 'HumanCount':
        self._unit = value
        return self

    def as_human(self, prec: Optional[int] = None) -> str:
        """Return a beautiful representation of this count.
        It dynamically calculates the best scale to use.

        Args:
            prec: an optional custom precision

        Returns:
            the human friendly representation.

        """
        return fn_human_count(FEATURES.feature_space, FEATURES.feature_1024,
                              FEATURES.feature_iec)(self._value, self._unit, prec)

    def __str__(self):
        return self.as_human()

    def __repr__(self):  # pragma: no cover
        return 'HumanCount{{ value={} }} -> {}'.format(self._value, self)

    def __eq__(self, other):
        return self.__str__() == other
