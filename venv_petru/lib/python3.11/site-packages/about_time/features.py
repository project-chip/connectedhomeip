# Features modeled after my Rust lib https://crates.io/crates/human-repr

class Features:
    def __init__(self):
        self._feature_space = False
        self._feature_1024 = False
        self._feature_iec = False

    @property
    def feature_space(self) -> bool:
        return self._feature_space

    @property
    def feature_1024(self) -> bool:
        return self._feature_1024

    @property
    def feature_iec(self) -> bool:
        return self._feature_iec

    @feature_space.setter
    def feature_space(self, value: bool):  # pragma: no cover
        self._feature_space = bool(value)

    @feature_1024.setter
    def feature_1024(self, value: bool):  # pragma: no cover
        self._feature_1024 = bool(value)

    @feature_iec.setter
    def feature_iec(self, value: bool):  # pragma: no cover
        self._feature_iec = bool(value)
        self.feature_1024 = value


def conv_space(space: bool) -> str:
    return {False: '', True: ' '}[space]


FEATURES = Features()
