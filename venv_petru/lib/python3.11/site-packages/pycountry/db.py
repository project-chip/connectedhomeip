import json
import logging
import threading
from collections.abc import Callable, Iterator
from typing import Any, Generic, TypeVar, cast

logger = logging.getLogger("pycountry.db")


class Data:
    def __init__(self, **fields: str):
        self._fields = fields

    def __getattr__(self, key: str) -> str:
        if key in self._fields:
            return self._fields[key]
        raise AttributeError(key)

    def __setattr__(self, key: str, value: str) -> None:
        if key != "_fields":
            self._fields[key] = value
        super().__setattr__(key, value)

    def __repr__(self) -> str:
        cls_name = self.__class__.__name__
        fields = ", ".join("%s=%r" % i for i in sorted(self._fields.items()))
        return f"{cls_name}({fields})"

    def __dir__(self) -> list[str]:
        return dir(self.__class__) + list(self._fields)

    def __iter__(self):
        # allow casting into a dict
        for field in self._fields:
            yield field, getattr(self, field)


class Country(Data):
    pass


class Subdivision(Data):
    pass


F = TypeVar("F", bound=Callable[..., Any])


def lazy_load(f: F) -> F:
    def load_if_needed(self, *args, **kw):
        if not self._is_loaded:
            with self._load_lock:
                self._load()
        return f(self, *args, **kw)

    return cast(F, load_if_needed)


T = TypeVar("T", bound=Data)


class Database(Generic[T]):
    data_class: type | str
    root_key: str | None = None
    no_index: list[str] = []
    special_index: list[str] = []

    def __init__(self, filename: str) -> None:
        self.filename = filename
        self._is_loaded = False
        self._load_lock = threading.Lock()

        if isinstance(self.data_class, str):
            self.factory = type(self.data_class, (Data,), {})
        else:
            self.factory = self.data_class

    def _clear(self):
        self._is_loaded = False
        self.objects = []
        self.indices = {}

    def _special_index(self, obj, key) -> None:
        raise NotImplementedError("Must be implemented in subclass")

    def _special_deindex(self, obj, key) -> None:
        raise NotImplementedError("Must be implemented in subclass")

    def _index_object(self, obj) -> None:
        for key, value in obj._fields.items():
            if key in self.no_index:
                continue
            if key in self.special_index:
                self._special_index(obj, key)
                continue
            # Lookups and searches are case insensitive. Normalize
            # here.
            index = self.indices.setdefault(key, {})
            value = value.lower()
            if value in index:
                logger.debug(
                    "%s %r already taken in index %r and will be "
                    "ignored. This is an error in the databases."
                    % (self.factory.__name__, value, key)
                )
            index[value] = obj

    def _deindex_object(self, obj) -> None:
        for key, value in obj._fields.items():
            if key in self.no_index:
                continue
            if key in self.special_index:
                self._special_deindex(obj, key)
                continue
            value = value.lower()
            index = self.indices.setdefault(key, {})
            if value in index:
                del index[value]

    def _load(self) -> None:
        if self._is_loaded:
            # Help keeping the _load_if_needed code easier
            # to read.
            return
        self._clear()

        with open(self.filename, encoding="utf-8") as f:
            tree = json.load(f)

        for entry in tree[self.root_key]:
            obj = self.factory(**entry)
            self.objects.append(obj)
            # Inject into indices
            self._index_object(obj)
        self._is_loaded = True

    # Public API

    @lazy_load
    def add_entry(self, **kw):
        # create the object with the correct dynamic type
        obj = self.factory(**kw)

        # append object
        self.objects.append(obj)

        # update indices
        self._index_object(obj)

    @lazy_load
    def remove_entry(self, **kw):
        # make sure that we receive None if no entry found
        kw.pop("default", None)
        obj = self.get(**kw)
        if not obj:
            raise KeyError(
                f"{self.factory.__name__} not found and cannot be removed: {kw}"
            )

        # remove object
        self.objects.remove(obj)

        # update indices
        self._deindex_object(obj)

    @lazy_load
    def __iter__(self) -> Iterator[T]:
        return iter(self.objects)

    @lazy_load
    def __len__(self) -> int:
        return len(self.objects)

    @lazy_load
    def get(self, *, default: T | None = None, **kw: str | None) -> T | None:
        if len(kw) != 1:
            raise TypeError("Only one criteria may be given")
        field, value = kw.popitem()
        if not isinstance(value, str):
            raise LookupError()
        # Normalize for case-insensitivity
        value = value.lower()
        index = self.indices[field]
        try:
            return index[value]
        except KeyError:
            # Pythonic APIs implementing     get() shouldn't raise KeyErrors.
            # Those are a bit unexpected and they should rather support
            # returning `None` by default and allow customization.
            return default

    @lazy_load
    def lookup(self, value: str) -> T:
        if not isinstance(value, str):
            raise LookupError()

        # Normalize for case-insensitivity
        value = value.lower()

        # Use indexes first
        for key in self.indices:
            try:
                return self.indices[key][value]
            except LookupError:
                pass

        # Use non-indexed values now. Avoid going through indexed values.
        for candidate in self:
            for k in self.no_index:
                v = candidate._fields.get(k)
                if v is None:
                    continue
                if v.lower() == value:
                    return candidate

        raise LookupError("Could not find a record for %r" % value)
