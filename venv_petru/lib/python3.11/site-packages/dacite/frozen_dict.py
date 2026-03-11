from collections.abc import Mapping


class FrozenDict(Mapping):
    dict_cls = dict

    def __init__(self, *args, **kwargs):
        self._dict = self.dict_cls(*args, **kwargs)
        self._hash = None

    def __getitem__(self, key):
        return self._dict[key]

    def __contains__(self, key):
        return key in self._dict

    def copy(self, **add_or_replace):
        return self.__class__(self, **add_or_replace)

    def __iter__(self):
        return iter(self._dict)

    def __len__(self):
        return len(self._dict)

    def __repr__(self):
        return f"<{self.__class__.__name__} {repr(self._dict)}>"

    def __hash__(self):
        if self._hash is None:
            self._hash = 0
            for key, value in self._dict.items():
                self._hash ^= hash((key, value))
        return self._hash
