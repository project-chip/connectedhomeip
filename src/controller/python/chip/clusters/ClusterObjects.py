#
#    Copyright (c) 2020 Project CHIP Authors
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

from dataclasses import dataclass, asdict, field, make_dataclass
from typing import ClassVar, List, Dict, Any, Mapping, Type, Union, ClassVar
import enum
import typing
from chip import tlv, ChipUtility
from chip.clusters.Types import Nullable, NullValue
from dacite import from_dict


def GetUnionUnderlyingType(typeToCheck, matchingType=None):
    ''' This retrieves the underlying types behind a unioned type by appropriately
        passing in the required matching type in the matchingType input argument.

        If that is 'None' (not to be confused with NoneType), then it will retrieve
        the 'real' type behind the union, i.e not Nullable && not None
    '''
    if (not(typing.get_origin(typeToCheck) == typing.Union)):
        return None

    for t in typing.get_args(typeToCheck):
        if (matchingType is None):
            if (t != type(None) and t != Nullable):
                return t
        else:
            if (t == matchingType):
                return t

    return None


@dataclass
class ClusterObjectFieldDescriptor:
    Label: str = ''
    Tag: int = None
    Type: Type = None

    def _PutSingleElementToTLV(self, tag, val, elementType, writer: tlv.TLVWriter, debugPath: str = '?'):
        if issubclass(elementType, ClusterObject):
            if not isinstance(val, dict):
                raise ValueError(
                    f"Field {debugPath}.{self.Label} expected a struct, but got {type(val)}")
            elementType.descriptor.DictToTLVWithWriter(
                f'{debugPath}.{self.Label}', tag, val, writer)
            return

        try:
            val = elementType(val)
        except Exception:
            raise ValueError(
                f"Field {debugPath}.{self.Label} expected {elementType}, but got {type(val)}")
        writer.put(tag, val)

    def PutFieldToTLV(self, tag, val, writer: tlv.TLVWriter, debugPath: str = '?'):
        if (val == NullValue):
            if (GetUnionUnderlyingType(self.Type, Nullable) is None):
                raise ValueError(
                    f"Field {debugPath}.{self.Label} was not nullable, but got a null")

            writer.put(tag, None)
        elif (val is None):
            if (GetUnionUnderlyingType(self.Type, type(None)) is None):
                raise ValueError(
                    f"Field {debugPath}.{self.Label} was not optional, but encountered None")
        else:
            #
            # If it is an optional or nullable type, it's going to be a union.
            # So, let's get at the 'real' type within that union before proceeding,
            # since at this point, we're guarenteed to not get None or Null as values.
            #
            elementType = GetUnionUnderlyingType(self.Type)
            if (elementType is None):
                elementType = self.Type

            if not isinstance(val, List):
                self._PutSingleElementToTLV(
                    tag, val, elementType, writer, debugPath)
                return

            writer.startArray(tag)

            # Get the type of the list. This is a generic, which has its sub-type information of the list element
            # inside its type argument.
            (elementType, ) = typing.get_args(elementType)

            for i, v in enumerate(val):
                self._PutSingleElementToTLV(
                    None, v, elementType, writer, debugPath + f'[{i}]')
            writer.endContainer()


@dataclass
class ClusterObjectDescriptor:
    Fields: List[ClusterObjectFieldDescriptor]

    def GetFieldByTag(self, tag: int) -> ClusterObjectFieldDescriptor:
        for field in self.Fields:
            if field.Tag == tag:
                return field
        return None

    def GetFieldByLabel(self, label: str) -> ClusterObjectFieldDescriptor:
        for field in self.Fields:
            if field.Label == label:
                return field
        return None

    def _ConvertNonArray(self, debugPath: str, elementType, value: Any) -> Any:
        if not issubclass(elementType, ClusterObject):
            if (issubclass(elementType, enum.Enum)):
                value = elementType(value)

            if not isinstance(value, elementType):
                raise ValueError(
                    f"Failed to decode field {debugPath}, expected type {elementType}, got {type(value)}")
            return value
        if not isinstance(value, Mapping):
            raise ValueError(
                f"Failed to decode field {debugPath}, struct expected.")
        return elementType.descriptor.TagDictToLabelDict(debugPath, value)

    def TagDictToLabelDict(self, debugPath: str, tlvData: Dict[int, Any]) -> Dict[str, Any]:
        ret = {}
        for tag, value in tlvData.items():
            descriptor = self.GetFieldByTag(tag)
            if not descriptor:
                # We do not have enough information for this field.
                ret[tag] = value
                continue

            if (value is None):
                ret[descriptor.Label] = NullValue
                continue

            if (typing.get_origin(descriptor.Type) == typing.Union):
                realType = GetUnionUnderlyingType(descriptor.Type)
                if (realType is None):
                    raise ValueError(
                        f"Field {debugPath}.{self.Label} has no valid underlying data model type")

                valueType = realType
            else:
                valueType = descriptor.Type

            if (typing.get_origin(valueType) == list):
                listElementType = typing.get_args(valueType)[0]
                ret[descriptor.Label] = [
                    self._ConvertNonArray(
                        f'{debugPath}[{i}]', listElementType, v)
                    for i, v in enumerate(value)]
                continue
            ret[descriptor.Label] = self._ConvertNonArray(
                f'{debugPath}.{descriptor.Label}', valueType, value)
        return ret

    def TLVToDict(self, tlvBuf: bytes) -> Dict[str, Any]:
        tlvData = tlv.TLVReader(tlvBuf).get().get('Any', {})
        return self.TagDictToLabelDict([], tlvData)

    def DictToTLVWithWriter(self, debugPath: str, tag, data: Mapping, writer: tlv.TLVWriter):
        writer.startStructure(tag)
        for field in self.Fields:
            val = data.get(field.Label, None)
            field.PutFieldToTLV(field.Tag, val, writer,
                                debugPath + f'.{field.Label}')
        writer.endContainer()

    def DictToTLV(self, data: dict) -> bytes:
        tlvwriter = tlv.TLVWriter(bytearray())
        self.DictToTLVWithWriter('', None, data, tlvwriter)
        return bytes(tlvwriter.encoding)


class ClusterObject:
    def ToTLV(self):
        return self.descriptor.DictToTLV(asdict(self))

    @classmethod
    def FromDict(cls, data: dict):
        return from_dict(data_class=cls, data=data)

    @classmethod
    def FromTLV(cls, data: bytes):
        return cls.FromDict(data=cls.descriptor.TLVToDict(data))

    @ChipUtility.classproperty
    def descriptor(cls):
        raise NotImplementedError()


class ClusterCommand(ClusterObject):
    @ChipUtility.classproperty
    def cluster_id(self) -> int:
        raise NotImplementedError()

    @ChipUtility.classproperty
    def command_id(self) -> int:
        raise NotImplementedError()

    @ChipUtility.classproperty
    def must_use_timed_invoke(cls) -> bool:
        return False


class Cluster(ClusterObject):
    '''
    When send read requests with returnClusterObject=True, we will set the data_version property of the object.
    Otherwise the [endpoint][cluster][Clusters.DataVersion] will be set to the DataVersion of the cluster.

    For data_version, we do not make it a real property so we can distinguish it with real attributes internally,
    especially the TLV decoding logic. Also ThreadNetworkDiagnostics has an attribute with the same name so we
    picked data_version as its name.
    '''
    @property
    def data_version(self) -> int:
        return self._data_version

    def __rich_repr__(self):
        '''
        Override the default behavior of rich.pretty.pprint for adding the cluster data version.
        '''
        if self._data_version is not None:
            yield "(data version)", self.data_version
        for k in self.__dataclass_fields__.keys():
            if k in self.__dict__:
                yield k, self.__dict__[k]

    def SetDataVersion(self, version: int) -> None:
        self._data_version = version


class ClusterAttributeDescriptor:
    '''
    The ClusterAttributeDescriptor is used for holding an attribute's metadata like its cluster id, attribute id and its type.

    Users should not initialize an object based on this class. Instead, users should pass the subclass objects to tell some methods what they want.

    The implementation of this functions is quite tricky, it will create a cluster object on-the-fly, and use it for actual encode / decode routine to save lines of code.
    '''
    @classmethod
    def ToTLV(cls, tag: Union[int, None], value):
        writer = tlv.TLVWriter()
        wrapped_value = cls._cluster_object(Value=value)
        cls.attribute_type.PutFieldToTLV(tag,
                                         asdict(wrapped_value)['Value'], writer, '')
        return writer.encoding

    @classmethod
    def FromTLV(cls, tlvBuffer: bytes):
        obj_class = cls._cluster_object
        return obj_class.FromDict(obj_class.descriptor.TagDictToLabelDict('', {0: tlv.TLVReader(tlvBuffer).get().get('Any', {})})).Value

    @classmethod
    def FromTagDictOrRawValue(cls, val: Any):
        obj_class = cls._cluster_object
        return obj_class.FromDict(obj_class.descriptor.TagDictToLabelDict('', {0: val})).Value

    @ChipUtility.classproperty
    def cluster_id(self) -> int:
        raise NotImplementedError()

    @ChipUtility.classproperty
    def attribute_id(self) -> int:
        raise NotImplementedError()

    @ChipUtility.classproperty
    def attribute_type(cls) -> ClusterObjectFieldDescriptor:
        raise NotImplementedError()

    @ChipUtility.classproperty
    def must_use_timed_write(cls) -> bool:
        return False

    @ChipUtility.classproperty
    def _cluster_object(cls) -> ClusterObject:
        return make_dataclass('InternalClass',
                              [
                                  ('Value', cls.attribute_type.Type,
                                   field(default=None)),
                                  ('descriptor', ClassVar[ClusterObjectDescriptor],
                                   field(
                                      default=ClusterObjectDescriptor(
                                          Fields=[ClusterObjectFieldDescriptor(
                                              Label='Value', Tag=0, Type=cls.attribute_type.Type)]
                                      )
                                  )
                                  )
                              ],
                              bases=(ClusterObject,))


class ClusterEvent(ClusterObject):
    @ChipUtility.classproperty
    def cluster_id(self) -> int:
        raise NotImplementedError()

    @ChipUtility.classproperty
    def event_id(self) -> int:
        raise NotImplementedError()
