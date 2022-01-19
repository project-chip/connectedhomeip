import enum

from dataclasses import dataclass, field
from typing import List, Set


class FieldAttribute(enum.Enum):
    OPTIONAL = enum.auto()
    NULLABLE = enum.auto()


class AttributeAccess(enum.Enum):
    READONLY = enum.auto()
    READWRITE = enum.auto()


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
class Field:
    data_type: str
    code: int
    name: str
    is_list: bool = False
    attributes: Set[FieldAttribute] = field(default_factory=set)


@dataclass
class Attribute:
    access: AttributeAccess
    definition: Field


@dataclass
class Struct:
    name: str
    fields: List[Field]
    tag: StructTag = None


@dataclass
class Event:
    priority: EventPriority
    name: str
    code: int
    fields: List[Field]


@dataclass
class EnumEntry:
    name: str
    code: int


@dataclass
class Enum:
    name: str
    base_type: str
    entries: List[EnumEntry]


@dataclass
class Command:
    name: str
    code: int
    input_param: str
    output_param: str


@dataclass
class Cluster:
    side: ClusterSide
    name: str
    code: int
    enums: List[Enum] = field(default_factory=list)
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
