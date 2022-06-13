# Matter tracing

Matter tracing provides a tool for applications to trace information about the
execution of the application. It depends on
[pw_trace module](https://pigweed.dev/pw_trace/).

Application can override trace events with custom trace system by setting
MATTER_CUSTOM_TRACE to true and direct trace macros to
trace/MatterCustomTrace.h.

## How to add trace events

1. Include "trace/trace.h" in the source file.
2. Add `${chip_root}/src/trace` as deps in BUILD.gn.
3. Add MATTER*TRACE_EVENT*\* in functions to be traced.

## Example

```
#include "pw_trace/trace.h"

  void SendButton() {
    MATTER_TRACE_EVENT_FUNCTION();
    // do something
  }

  void InputLoop() {
    while(1) {
      auto event = WaitNewInputEvent()
      MATTER_TRACE_EVENT_SCOPE("Handle Event");  // measure until loop finished
      if (event == kNewButton){
        SendButton();
        MATTER_TRACE_EVENT_END("button");  // Trace event was started in ButtonIsr
      } else {
        MATTER_TRACE_EVENT_INSTANT("Unknown event");
      }
    }
  }

  void ButtonIsr() {
    MATTER_TRACE_EVENT_START("button");
    SendNewInputEvent(kNewButton);
  }
```
