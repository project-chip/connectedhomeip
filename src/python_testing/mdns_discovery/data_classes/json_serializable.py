#
#    Copyright (c) 2025 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

import json
from dataclasses import asdict
from typing import Any, Dict


class OrderedPropsMeta(type):
    """
    Metaclass that tracks the order of @property definitions for this class.
    Stores names in __property_order__ exactly as written in the class body.
    """
    @classmethod
    def __prepare__(mcls, name, bases, **kwargs):
        return {}  # preserve definition order

    def __new__(mcls, name, bases, namespace, **kwargs):
        cls = super().__new__(mcls, name, bases, dict(namespace))

        # Collect the class's property names in declaration order
        cls.__property_order__ = [
            n for n, v in namespace.items()
            if isinstance(v, property) and not n.startswith("_")
        ]

        return cls


class JsonSerializable(metaclass=OrderedPropsMeta):
    """
    Base class that provides JSON-friendly serialization for dataclasses.

    - Collects all dataclass fields (recursively) using `asdict`, then filters
      out anything that can't be encoded by `json.dumps`.
    - Adds all properties in the order they were written in the class
    """

    def json_dict(self) -> dict:
        result: Dict[str, Any] = {}

        # Turn all dataclass fields (including nested ones) into a dict
        # and keep only the values that can be safely JSON-serialized
        raw = asdict(self)
        for key, value in raw.items():
            if self._is_json_serializable(value):
                result[key] = value

        # Keep properties in preserved declaration order
        for name in getattr(self.__class__, "__property_order__", []):
            value = getattr(self, name)
            if self._is_json_serializable(value):
                result[name] = value

        return result

    @staticmethod
    def _is_json_serializable(value: Any) -> bool:
        try:
            json.dumps(value)
            return True
        except (TypeError, OverflowError):
            return False
