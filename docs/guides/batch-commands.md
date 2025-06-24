# Accepting Batch Commands

## Overview

As of Matter 1.3, Matter servers have been capable of receiving multiple (batch)
invoke requests in a single Invoke Interaction. This document explains how to
enable accepting batch commands.

## Background

The best place to look in the specification concerning batch commands is
sections that mention `MAX_PATHS_PER_INVOKE`. Another helpful area in the
specification for detailed information is the list of `InvokeRequests` within
the Invoke Request Action, and how specification outlines how requests are
processed and responded to.

One very important aspect of a batch of commands within a single Invoke Request
Message is that the paths associated with each of the commands SHALL be a
concrete (non-wildcard) path, where each path is unique.

## Enabling Batch Commands on Matter Server

Enabling should be as simple as setting `CHIP_CONFIG_MAX_PATHS_PER_INVOKE` to
the maximum number of commands you wish to be able to receive in a batch.

Note:

-   While `CHIP_CONFIG_MAX_PATHS_PER_INVOKE` can technically be set up to 65535,
    most Matter devices send traffic using UDP. This means that the real
    constraint will be the MTU of the UDP packet. In practice, a list of
    `CommandDataIB` containing only `CommandPath` and `CommandRef` with no
    `CommandFields` (For example: An On or Off command), you can fit roughly 58
    `CommandDataIB` into a single Invoke Request Message before exceeding the
    MTU of a UDP packet.

## Testing

Using `chip-repl`, you are able to send batch commands using `SendBatchCommands`
in `ChipDeviceController`.

Note: `chip-tool` does NOT support sending batch commands.

## Advanced: Leveraging Batch Commands for More Efficiently Handling Commands

As of July 2025, when this documentation was initially authored, the SDK simply
calls callbacks to process handling commands sequentially/serially. It is up to
the application/cluster code to identify commands that have been sent in a batch
and find more efficient ways of handling them. For example, a bridge might want
to collect all batched commands in a single invoke action and send them as a
group command to all bridged devices.

Theoretically this could be done by adding a Batcher instance and leveraging the
matter event loop. You would insert the instance of Batcher to handle relevant
command(s), it would append the command to a list, schedule a task on the event
loop to process the entire list. Pseudocode below:

```cpp
class Batcher : public CommandHandlerImpl::Callback{
public:
  class Command {
  public:
    // Command's constructor will need to capture all information needed
    // to process the command when DispatchCommandsAsList is scheduled
    // (or potentially in a subsequent scheduled task). It will also be
    // critical to instantiate an instance of CommandHandler::Handle
    // to allow for the command to be handled asynchronously. For more
    // information on how to properly use CommandHandler::Handle to
    // perform async command handling, see the documentation for
    // CommandHandler::Handle.
    Command(...) {...}
    [...]
  private:
    CommandHandler::Handle mHandle;
    [...]  // Other member variables important for processing the command.
            // This might, generally speaking, include a deep-copy of the command payload
            // (since that can point into network buffers that will go away before the async
            // code runs), information about the fabric and session involved (since both might
            // go away before the async code runs), and so on.
  };
  [...]
  static void DispatchCommandsAsList(intptr_t arg) {
    auto this_ = reinterpret_cast<Batcher*>(arg);
    std::vector<std::unique_ptr<Command>> commands;
    commands.swap(this_->mCommands);
    // Call thing that is capable of more efficiently processing
    // commands in parallel with arg of std::move(commands)
  }

  void DispatchCommand(...) override {
    bool has_previous_pending_request = !mCommands.empty();
    mCommands.push_back(std::make_unique<Command>(...));
    if (!has_previous_pending_request && !mCommands.empty()) {
      SystemLayer().ScheduleWork(DispatchCommandsAsList, this);
    }
  }
private:
  std::vector<std::unique_ptr<Command>> mCommands;
};
```
