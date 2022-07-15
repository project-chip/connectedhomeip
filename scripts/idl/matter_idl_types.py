import enum
from lark.tree import Meta

from dataclasses import dataclass, field
from typing import List, Set, Optional, Union


# Information about parsing location for specific items
# Helpful when referencing data items in logs when processing
@dataclass
class ParseMetaData:
    line: int
    column: int

    def __init__(self, meta: Meta = None, line: int = None, column: int = None):
        if meta:
            self.line = meta.line
            self.column = meta.column
        else:
            self.line = line
            self.column = column


class FieldAttribute(enum.Enum):
    OPTIONAL = enum.auto()
    NULLABLE = enum.auto()


class CommandAttribute(enum.Enum):
    TIMED_INVOKE = enum.auto()
    FABRIC_SCOPED = enum.auto()


class AttributeTag(enum.Enum):
    READABLE = enum.auto()
    WRITABLE = enum.auto()
    NOSUBSCRIBE = enum.auto()
    FABRIC_SCOPED = enum.auto()


class AttributeStorage(enum.Enum):
    RAM = enum.auto()
    PERSIST = enum.auto()
    CALLBACK = enum.auto()


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


class AccessPrivilege(enum.Enum):
    VIEW = enum.auto()
    OPERATE = enum.auto()
    MANAGE = enum.auto()
    ADMINISTER = enum.auto()


class AttributeOperation(enum.Enum):
    READ = enum.auto()
    WRITE = enum.auto()


@dataclass
class DataType:
    name: str

    # Applies for strings (char or binary)
    max_length: Optional[int] = None


@dataclass
class Field:
    data_type: DataType
    code: int
    name: str
    is_list: bool = False
    attributes: Set[FieldAttribute] = field(default_factory=set)

    @property
    def is_optional(self):
        return FieldAttribute.OPTIONAL in self.attributes

    @property
    def is_nullable(self):
        return FieldAttribute.NULLABLE in self.attributes


@dataclass
class Attribute:
    definition: Field
    tags: Set[AttributeTag] = field(default_factory=set)
    readacl: AccessPrivilege = AccessPrivilege.VIEW
    writeacl: AccessPrivilege = AccessPrivilege.OPERATE
    default: Optional[Union[str, int]] = None

    @property
    def is_readable(self):
        return AttributeTag.READABLE in self.tags

    @property
    def is_writable(self):
        return AttributeTag.WRITABLE in self.tags

    @property
    def is_subscribable(self):
        return AttributeTag.NOSUBSCRIBE not in self.tags


@dataclass
class Struct:
    name: str
    fields: List[Field]
    tag: Optional[StructTag] = None
    code: Optional[int] = None  # for responses only


@dataclass
class Event:
    priority: EventPriority
    name: str
    code: int
    fields: List[Field]
    readacl: AccessPrivilege = AccessPrivilege.VIEW


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
    input_param: Optional[str]
    output_param: str
    attributes: Set[CommandAttribute] = field(default_factory=set)
    invokeacl: AccessPrivilege = AccessPrivilege.OPERATE

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

    # Parsing meta data missing only when skip meta data is requested
    parse_meta: Optional[ParseMetaData] = field(default=None)


@dataclass
class AttributeInstantiation:
    name: str
    storage: AttributeStorage
    default: Optional[Union[str, int, bool]] = None

    # Parsing meta data missing only when skip meta data is requested
    parse_meta: Optional[ParseMetaData] = field(default=None)


@dataclass
class ServerClusterInstantiation:
    name: str
    attributes: List[AttributeInstantiation] = field(default_factory=list)

    # Parsing meta data missing only when skip meta data is requested
    parse_meta: Optional[ParseMetaData] = field(default=None)


@dataclass
class DeviceType:
    name: str
    code: int


@dataclass
class Endpoint:
    number: int
    device_types: List[DeviceType] = field(default_factory=list)
    server_clusters: List[ServerClusterInstantiation] = field(default_factory=list)
    client_bindings: List[str] = field(default_factory=list)


@dataclass
class Idl:
    # Enums and structs represent globally used items
    enums: List[Enum] = field(default_factory=list)
    structs: List[Struct] = field(default_factory=list)
    clusters: List[Cluster] = field(default_factory=list)
    endpoints: List[Endpoint] = field(default_factory=list)

    # IDL file name is available only if parsing provides a file name
    parse_file_name: Optional[str] = field(default=None)
