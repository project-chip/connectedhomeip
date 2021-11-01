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
from chip import tlv, ChipUtility
from dacite import from_dict


@dataclass
class ClusterObjectFieldDescriptor:
    Label: str = ''
    Tag: int = None
    Type: Type = None
    IsArray: bool = False

    def _PutSingleElementToTLV(self, tag, val, writer: tlv.TLVWriter, debugPath: str = '?'):
        if issubclass(self.Type, ClusterObject):
            if not isinstance(val, dict):
                raise ValueError(
                    f"Field {debugPath}.{self.Label} expected a struct, but got {type(val)}")
            self.Type.descriptor.DictToTLVWithWriter(
                f'{debugPath}.{self.Label}', tag, val, writer)
            return
        try:
            val = self.Type(val)
        except Exception:
            raise ValueError(
                f"Field {debugPath}.{self.Label} expected {self.Type}, but got {type(val)}")
        writer.put(tag, val)

    def PutFieldToTLV(self, tag, val, writer: tlv.TLVWriter, debugPath: str = '?'):
        if not self.IsArray:
            self._PutSingleElementToTLV(tag, val, writer, debugPath)
            return
        if not isinstance(val, List):
            raise ValueError(
                f"Field {debugPath}.{self.Label} expected List[{self.Type}], but got {type(val)}")
        writer.startArray(tag)
        for i, v in enumerate(val):
            self._PutSingleElementToTLV(
                None, v, writer, debugPath + f'[{i}]')
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

    def _ConvertNonArray(self, debugPath: str, descriptor: ClusterObjectFieldDescriptor, value: Any) -> Any:
        if not issubclass(descriptor.Type, ClusterObject):
            if not isinstance(value, descriptor.Type):
                raise ValueError(
                    f"Failed to decode field {debugPath}, expected type {descriptor.Type}, got {type(value)}")
            return value
        if not isinstance(value, Mapping):
            raise ValueError(
                f"Failed to decode field {debugPath}, struct expected.")
        return descriptor.Type.descriptor.TagDictToLabelDict(debugPath, value)

    def TagDictToLabelDict(self, debugPath: str, tlvData: Dict[int, Any]) -> Dict[str, Any]:
        ret = {}
        for tag, value in tlvData.items():
            descriptor = self.GetFieldByTag(tag)
            if not descriptor:
                # We do not have enough infomation for this field.
                ret[tag] = value
                continue
            if descriptor.IsArray:
                ret[descriptor.Label] = [
                    self._ConvertNonArray(f'{debugPath}[{i}]', descriptor, v)
                    for i, v in enumerate(value)]
                continue
            ret[descriptor.Label] = self._ConvertNonArray(
                f'{debugPath}.{descriptor.Label}', descriptor, value)
        return ret

    def TLVToDict(self, tlvBuf: bytes) -> Dict[str, Any]:
        tlvData = tlv.TLVReader(tlvBuf).get().get('Any', {})
        return self.TagDictToLabelDict([], tlvData)

    def DictToTLVWithWriter(self, debugPath: str, tag, data: Mapping, writer: tlv.TLVWriter):
        writer.startStructure(tag)
        for field in self.Fields:
            val = data.get(field.Label, None)
            if val is None:
                raise ValueError(
                    f"Field {debugPath}.{field.Label} is missing in the given dict")
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
    def _cluster_object(cls) -> ClusterObject:
        return make_dataclass('InternalClass',
                              [
                                  ('Value', List[cls.attribute_type.Type]
                                   if cls.attribute_type.IsArray else cls.attribute_type.Type, field(default=None)),
                                  ('descriptor', ClassVar[ClusterObjectDescriptor],
                                   field(
                                      default=ClusterObjectDescriptor(
                                          Fields=[ClusterObjectFieldDescriptor(
                                              Label='Value', Tag=0, Type=cls.attribute_type.Type, IsArray=cls.attribute_type.IsArray)]
                                      )
                                  )
                                  )
                              ],
                              bases=(ClusterObject,))
