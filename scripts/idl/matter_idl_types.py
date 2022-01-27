import enum

from dataclasses import dataclass, field
from typing import List, Set, Union


class FieldAttribute(enum.Enum):
    OPTIONAL = enum.auto()
    NULLABLE = enum.auto()


class CommandAttribute(enum.Enum):
    TIMED_INVOKE = enum.auto()


class AttributeTag(enum.Enum):
    READABLE = enum.auto()
    WRITABLE = enum.auto()
    GLOBAL = enum.auto()
    NOSUBSCRIBE = enum.auto()


class EventPriority(enum.Enum):
    DEBUG = enum.auto()
    INFO = enum.auto()
    CRITICAL = enum.auto()


class ClusterSide(enum.Enum):
    CLIENT = enum.auto()
    SERVER = enum.auto()


class StructTag(enum.Enum):
    REQUEST = enum.auto()
    RESPONSE = enum.auto()


class EndpointContentType(enum.Enum):
    SERVER_CLUSTER = enum.auto()
    CLIENT_BINDING = enum.auto()


@dataclass
class DataType:
    name: str

    # Applies for strings (char or binary)
    max_length: Union[int, None] = None


@dataclass
class Field:
    data_type: str
    code: int
    name: str
    is_list: bool = False
    attributes: Set[FieldAttribute] = field(default_factory=set)


@dataclass
class Attribute:
    definition: Field
    tags: Set[AttributeTag] = field(default_factory=set)

    @property
    def is_readable(self):
        return AttributeTag.READABLE in self.tags

    @property
    def is_writable(self):
        return AttributeTag.WRITABLE in self.tags

    @property
    def is_global(self):
        return AttributeTag.GLOBAL in self.tags


@dataclass
class Struct:
    name: str
    fields: List[Field]
    tag: Union[StructTag, None] = None


@dataclass
class Event:
    priority: EventPriority
    name: str
    code: int
    fields: List[Field]


@dataclass
class ConstantEntry:
    name: str
    code: int


@dataclass
class Enum:
    name: str
    base_type: str
    entries: List[ConstantEntry]


@dataclass
class Bitmap:
    name: str
    base_type: str
    entries: List[ConstantEntry]


@dataclass
class Command:
    name: str
    code: int
    input_param: str
    output_param: str
    attributes: Set[CommandAttribute] = field(default_factory=set)

    @property
    def is_timed_invoke(self):
        return CommandAttribute.TIMED_INVOKE in self.attributes


@dataclass
class Cluster:
    side: ClusterSide
    name: str
    code: int
    enums: List[Enum] = field(default_factory=list)
    bitmaps: List[Bitmap] = field(default_factory=list)
    events: List[Event] = field(default_factory=list)
    attributes: List[Attribute] = field(default_factory=list)
    structs: List[Struct] = field(default_factory=list)
    commands: List[Command] = field(default_factory=list)


@dataclass
class Endpoint:
    number: int
    server_clusters: List[str] = field(default_factory=list)
    client_bindings: List[str] = field(default_factory=list)


@dataclass
class Idl:
    # Enums and structs represent globally used items
    enums: List[Enum] = field(default_factory=list)
    structs: List[Struct] = field(default_factory=list)
    clusters: List[Cluster] = field(default_factory=list)
    endpoints: List[Endpoint] = field(default_factory=list)
