#
#    Copyright (c) 2023 Project CHIP Authors
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

import ctypes
from enum import Enum, auto
from typing import Optional

import chip.native
from chip.native import PyChipError


def _GetTracingLibraryHandle() -> ctypes.CDLL:
    """ Get the native library handle with tracing methods initialized.

      Retreives the CHIP native library handle and attaches signatures to
      native methods.
      """

    # Getting a handle without requiring init, as tracing methods
    # do not require chip stack startup
    handle = chip.native.GetLibraryHandle(chip.native.HandleFlags(0))

    # Uses one of the type decorators as an indicator for everything being
    # initialized.
    if not handle.pychip_tracing_start_json_file.argtypes:
        setter = chip.native.NativeLibraryHandleMethodArguments(handle)

        setter.Set('pychip_tracing_start_json_log', None, [])
        setter.Set('pychip_tracing_start_json_file', PyChipError, [ctypes.c_char_p])

        setter.Set('pychip_tracing_start_perfetto_system', None, [])
        setter.Set('pychip_tracing_start_perfetto_file', PyChipError, [ctypes.c_char_p])

        setter.Set('pychip_tracing_stop', None, [])

    return handle


class TraceType(Enum):
    JSON = auto()
    PERFETTO = auto()


def StartTracingTo(trace_type: TraceType, file_name: Optional[str] = None):
    """
    Initiate tracing to the specified destination.

    Note that only one active trace can exist of a given type (i.e. cannot trace both
    to files and logs/system).
    """
    handle = _GetTracingLibraryHandle()

    if trace_type == TraceType.JSON:
        if file_name is None:
            handle.pychip_tracing_start_json_log()
        else:
            handle.pychip_tracing_start_json_file(file_name.encode('utf-8')).raise_on_error()
    elif trace_type == TraceType.PERFETTO:
        if file_name is None:
            handle.pychip_tracing_start_perfetto_system()
        else:
            handle.pychip_tracing_start_perfetto_file(file_name.encode('utf-8')).raise_on_error()
    else:
        raise ValueError("unknown trace type")


def StopTracing():
    """
    Make sure tracing is stopped.

    MUST be called before application exits.
    """
    _GetTracingLibraryHandle().pychip_tracing_stop()


class TracingContext:
    """Allows scoped enter/exit for tracing, like:

    with TracingContext() as tracing:
       tracing.Start(TraceType.JSON)
       # ...

    """

    def Start(self, trace_type: TraceType, file_name: Optional[str] = None):
        StartTracingTo(trace_type, file_name)

    def StartFromString(self, destination: str):
        """
        Convert a human string to a perfetto start.

        Supports json:log, json:path, perfetto, perfetto:path
        """
        if destination == 'perfetto':
            self.Start(TraceType.PERFETTO)
        elif destination == 'json:log':
            self.Start(TraceType.JSON)
        elif destination.startswith("json:"):
            self.Start(TraceType.JSON, destination[5:])
        elif destination.startswith("perfetto:"):
            self.Start(TraceType.PERFETTO, destination[9:])
        else:
            raise ValueError("Invalid trace-to destination: %r", destination)

    def __enter__(self):
        return self

    def __exit__(self, type, value, traceback):
        StopTracing()
