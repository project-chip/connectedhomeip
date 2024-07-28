import enum
from dataclasses import dataclass, field
from typing import List, Optional, Set, Union

from lark.tree import Meta


class ApiMaturity(enum.Enum):
    STABLE = enum.auto()  # default
    PROVISIONAL = enum.auto()
    INTERNAL = enum.auto()
    DEPRECATED = enum.auto()

# Information about parsing location for specific items
# Helpful when referencing data items in logs when processing


@dataclass
class ParseMetaData:
    line: Optional[int]
    column: Optional[int]
    start_pos: Optional[int]

    def __init__(self, meta: Optional[Meta] = None, line: Optional[int] = None, column: Optional[int] = None, start_pos: Optional[int] = None):
        if meta:
            self.line = getattr(meta, 'line', None)
            self.column = getattr(meta, 'column', None)
            self.start_pos = getattr(meta, 'start_pos', None)
        else:
            self.line = line
            self.column = column
            self.start_pos = start_pos


class StructQuality(enum.Flag):
    NONE = 0
    FABRIC_SCOPED = enum.auto()


class FieldQuality(enum.Flag):
    NONE = 0
    OPTIONAL = enum.auto()
    NULLABLE = enum.auto()
    FABRIC_SENSITIVE = enum.auto()


class CommandQuality(enum.Flag):
    NONE = 0
    TIMED_INVOKE = enum.auto()
    FABRIC_SCOPED = enum.auto()


class AttributeQuality(enum.Flag):
    NONE = 0
    READABLE = enum.auto()
    WRITABLE = enum.auto()
    NOSUBSCRIBE = enum.auto()
    TIMED_WRITE = enum.auto()


class AttributeStorage(enum.Enum):
    RAM = enum.auto()
    PERSIST = enum.auto()
    CALLBACK = enum.auto()


class EventPriority(enum.Enum):
    DEBUG = enum.auto()
    INFO = enum.auto()
    CRITICAL = enum.auto()


class EventQuality(enum.Flag):
    NONE = 0
    FABRIC_SENSITIVE = enum.auto()


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
    min_length: Optional[int] = None
    max_length: Optional[int] = None

    # Applies for numbers
    min_value: Optional[int] = None
    max_value: Optional[int] = None


@dataclass
class Field:
    data_type: DataType
    code: int
    name: str
    is_list: bool = False
    qualities: FieldQuality = FieldQuality.NONE
    api_maturity: ApiMaturity = ApiMaturity.STABLE

    @property
    def is_optional(self):
        return FieldQuality.OPTIONAL & self.qualities

    @property
    def is_nullable(self):
        return FieldQuality.NULLABLE & self.qualities


@dataclass
class Attribute:
    definition: Field
    qualities: AttributeQuality = AttributeQuality.NONE
    readacl: AccessPrivilege = AccessPrivilege.VIEW
    writeacl: AccessPrivilege = AccessPrivilege.OPERATE
    default: Optional[Union[str, int]] = None
    api_maturity: ApiMaturity = ApiMaturity.STABLE

    @property
    def is_readable(self):
        return AttributeQuality.READABLE & self.qualities

    @property
    def is_writable(self):
        return AttributeQuality.WRITABLE & self.qualities

    @property
    def is_subscribable(self):
        return not (AttributeQuality.NOSUBSCRIBE & self.qualities)

    @property
    def requires_timed_write(self):
        return AttributeQuality.TIMED_WRITE & self.qualities


@dataclass
class Struct:
    name: str
    fields: List[Field]
    tag: Optional[StructTag] = None
    code: Optional[int] = None  # for responses only
    qualities: StructQuality = StructQuality.NONE
    api_maturity: ApiMaturity = ApiMaturity.STABLE
    is_global: bool = False


@dataclass
class Event:
    priority: EventPriority
    name: str
    code: int
    fields: List[Field]
    readacl: AccessPrivilege = AccessPrivilege.VIEW
    qualities: EventQuality = EventQuality.NONE
    description: Optional[str] = None
    api_maturity: ApiMaturity = ApiMaturity.STABLE

    @property
    def is_fabric_sensitive(self):
        return EventQuality.FABRIC_SENSITIVE & self.qualities


@dataclass
class ConstantEntry:
    name: str
    code: int
    api_maturity: ApiMaturity = ApiMaturity.STABLE


@dataclass
class Enum:
    name: str
    base_type: str
    entries: List[ConstantEntry]
    api_maturity: ApiMaturity = ApiMaturity.STABLE
    is_global: bool = False


@dataclass
class Bitmap:
    name: str
    base_type: str
    entries: List[ConstantEntry]
    api_maturity: ApiMaturity = ApiMaturity.STABLE
    is_global: bool = False


@dataclass
class Command:
    name: str
    code: int
    input_param: Optional[str]
    output_param: str
    qualities: CommandQuality = CommandQuality.NONE
    invokeacl: AccessPrivilege = AccessPrivilege.OPERATE
    description: Optional[str] = None
    api_maturity: ApiMaturity = ApiMaturity.STABLE

    # Parsing meta data missing only when skip meta data is requested
    parse_meta: Optional[ParseMetaData] = field(default=None, compare=False)

    @property
    def is_timed_invoke(self):
        return CommandQuality.TIMED_INVOKE & self.qualities


@dataclass
class Cluster:
    name: str
    code: int
    revision: int = 1
    enums: List[Enum] = field(default_factory=list)
    bitmaps: List[Bitmap] = field(default_factory=list)
    events: List[Event] = field(default_factory=list)
    attributes: List[Attribute] = field(default_factory=list)
    structs: List[Struct] = field(default_factory=list)
    commands: List[Command] = field(default_factory=list)
    description: Optional[str] = None
    api_maturity: ApiMaturity = ApiMaturity.STABLE

    # Parsing meta data missing only when skip meta data is requested
    parse_meta: Optional[ParseMetaData] = field(default=None, compare=False)


@dataclass
class AttributeInstantiation:
    name: str
    storage: AttributeStorage
    default: Optional[Union[str, int, bool]] = None

    # Parsing meta data missing only when skip meta data is requested
    parse_meta: Optional[ParseMetaData] = field(default=None, compare=False)


@dataclass
class CommandInstantiation:
    name: str

    # Parsing meta data missing only when skip meta data is requested
    parse_meta: Optional[ParseMetaData] = field(default=None, compare=False)


@dataclass
class ServerClusterInstantiation:
    name: str
    commands: List[CommandInstantiation] = field(default_factory=list)
    attributes: List[AttributeInstantiation] = field(default_factory=list)
    events_emitted: Set[str] = field(default_factory=set)

    # Parsing meta data missing only when skip meta data is requested
    parse_meta: Optional[ParseMetaData] = field(default=None, compare=False)


@dataclass
class DeviceType:
    name: str
    code: int
    version: int


@dataclass
class Endpoint:
    number: int
    device_types: List[DeviceType] = field(default_factory=list)
    server_clusters: List[ServerClusterInstantiation] = field(
        default_factory=list)
    client_bindings: List[str] = field(default_factory=list)


@dataclass
class Idl:
    clusters: List[Cluster] = field(default_factory=list)
    endpoints: List[Endpoint] = field(default_factory=list)

    # Global types
    global_bitmaps: List[Bitmap] = field(default_factory=list)
    global_enums: List[Enum] = field(default_factory=list)
    global_structs: List[Struct] = field(default_factory=list)

    # IDL file name is available only if parsing provides a file name
    parse_file_name: Optional[str] = field(default=None)
