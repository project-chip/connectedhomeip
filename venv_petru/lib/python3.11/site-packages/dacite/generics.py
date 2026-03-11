import sys
from dataclasses import Field, is_dataclass
from typing import Any, Dict, Generic, List, Tuple, Type, TypeVar, Union, get_type_hints
from dacite.exceptions import DaciteError

try:
    from typing import get_args, get_origin, Literal  # type: ignore
except ImportError:
    from typing_extensions import get_args, get_origin, Literal  # type: ignore

from .dataclasses import get_fields as dataclasses_get_fields


def __add_generics(type_origin: Any, type_args: Tuple, generics: Dict[TypeVar, Type]) -> None:
    """Adds (type var, concrete type) entries derived from a type's origin and args to the provided generics dict."""
    if type_origin and type_args and hasattr(type_origin, "__parameters__"):
        for param, arg in zip(type_origin.__parameters__, type_args):
            if isinstance(param, TypeVar):
                if param in generics and generics[param] != arg:
                    raise DaciteError(f"Ambiguous TypeVar: {generics[param]} != {arg}")
                generics[param] = arg


def __dereference(type_name: str, data_class: Type) -> Type:
    """
    Try to find the class belonging to the reference in the provided module and,
    if not found, iteratively look in parent modules.
    """
    if data_class.__class__.__name__ == type_name:
        return data_class

    module_name = data_class.__module__
    parts = module_name.split(".")
    for i in range(len(parts)):
        try:
            module = sys.modules[".".join(parts[:-i]) if i else module_name]
            return getattr(module, type_name)
        except AttributeError:
            pass
    raise AttributeError("Could not find reference.")


def __concretize(
    hint: Union[Type, TypeVar, str], generics: Dict[TypeVar, Type], data_class: Type
) -> Union[Type, TypeVar]:
    """Recursively replace type vars and forward references by concrete types."""

    if isinstance(hint, str):
        return __dereference(hint, data_class)

    if isinstance(hint, TypeVar):
        # Fall back on the original TypeVar if the generics dict does not contain it.
        # Setting config.check_types=False will in some cases still make from_dict work, albeit not type checked ofc.
        return generics.get(hint, hint)

    hint_origin = get_origin(hint)
    hint_args = get_args(hint)
    if hint_origin and hint_args and hint_origin is not Literal:
        concrete_hint_args = tuple(__concretize(a, generics, data_class) for a in hint_args)
        if concrete_hint_args != hint_args:
            if sys.version_info >= (3, 9):
                return hint_origin[concrete_hint_args]
            # It's generally not a good practice to overwrite __args__,
            # and it even has become impossible starting from python 3.13 (read-only),
            # but changing the output of get_type_hints is harmless (see unit test)
            # and at least this way, we get it working for python 3.8.
            hint.__args__ = concrete_hint_args

    return hint


def orig(data_class: Type) -> Any:
    if is_dataclass(data_class):
        return data_class
    return get_origin(data_class)


def get_concrete_type_hints(data_class: Type, *args, **kwargs) -> Dict[str, Any]:
    """
    An overwrite of typing.get_type_hints supporting generics and forward references,
    i.e. substituting concrete types in type vars and references.
    """
    generics: Dict[TypeVar, Type] = {}

    dc_origin = get_origin(data_class)
    dc_args = get_args(data_class)
    __add_generics(dc_origin, dc_args, generics)

    if hasattr(data_class, "__orig_bases__"):
        for base in data_class.__orig_bases__:
            base_origin = get_origin(base)
            base_args = get_args(base)
            if base_origin is not Generic:
                __add_generics(base_origin, base_args, generics)

    data_class = orig(data_class)
    hints = get_type_hints(data_class, *args, **kwargs)

    for key, hint in hints.copy().items():
        hints[key] = __concretize(hint, generics, data_class)

    return hints


def get_fields(data_class: Type) -> List[Field]:
    """An overwrite of dacite.dataclasses.get_fields supporting generics."""
    return dataclasses_get_fields(orig(data_class))
