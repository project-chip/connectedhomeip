# Upgrading notes

## API changes and code migration

### `CommandHandler`

`CommandHandler` ability to directly invoke `Prepare/TLV-Write/Finish` cycles
has been changed to only expose `AddResponse/AddStatus/AddClusterSpecific*`.

Original versions of `CommandHandler` exposed the following low-level
implementation-specific methods: `PrepareCommand`,
`PrepareInvokeResponseCommand`, `GetCommandDataIBTLVWriter` and `FinishCommand`.
These are not exposed anymore and instead one should use `AddResponse` or
`AddResponseData`. When using an `EncodableToTLV` argument, the same
functionality should be achievable.

Example

Before:

```cpp

const CommandHandler::InvokeResponseParameters prepareParams(requestPath);
ReturnOnFailure(commandHandler->PrepareInvokeResponseCommand(path, prepareParams));

TLV::TLVWriter *writer = commandHandler->GetCommandDataIBTLVWriter();
ReturnOnFailure(writer->Put(chip::TLV::ContextTag(1), 123));
ReturnOnFailure(writer->Put(chip::TLV::ContextTag(2), 234));
return commandHandler->FinishCommand();
```

After:

```cpp

class ReplyEncoder : public DataModel::EncodableToTLV
{
public:
    CHIP_ERROR EncodeTo(TLV::TLVWriter & writer, TLV::Tag tag) const override
    {
        TLV::TLVType outerType;
        ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outerType));

        ReturnOnFailure(writer.Put(chip::TLV::ContextTag(1), 123));
        ReturnOnFailure(writer.Put(chip::TLV::ContextTag(2), 234));

        return writer.EndContainer(outerType);
    }
};

// ...
ReplyEncoder replyEncoder;
commandHandler->AddResponse(path, kReplyCommandId, replyEncoder);

// or if error handling is implemented:
//
// ReturnErrorOnFailure(commandHandler->AddResponseData(path, kReplyCommandId, replyEncoder));
//
// In many cases error recovery from not being able to send a reply is not easy or expected,
// so code does AddResponse rather than AddResponseData.

```

### `CommandHandlerInterface` in `chip::app::InteractionModelEngine`

Command handler lists were placed in a separate registry class that is
independent of the InteractionModelEngine class.

The following replacements exist:

-   `chip::app::InteractionModelEngine::RegisterCommandHandler` replaced by
    `chip::app::CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler`
-   `chip::app::InteractionModelEngine::UnregisterCommandHandler` replaced by
    `chip::app::CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler`
-   `chip::app::InteractionModelEngine::FindCommandHandler` replaced by
    `chip::app::CommandHandlerInterfaceRegistry::Instance().GetCommandHandler`
-   `chip::app::InteractionModelEngine::UnregisterCommandHandlers` replaced by
    `chip::app::CommandHandlerInterfaceRegistry::Instance().UnregisterAllCommandHandlersForEndpoint`

### AttributeAccessInterface registration and removal

A new object exists for the attribute access interface registry, accessible as
`chip::app::AttributeHandlerInterfaceRegistry::Instance()`

Replacements for methods are:

-   `registerAttributeAccessOverride` replaced by
    `chip::app::AttributeAccessInterfaceRegistry::Instance().Register`
-   `unregisterAttributeAccessOverride` replaced by
    `chip::app::AttributeAccessInterfaceRegistry::Instance().Unregister`
-   `unregisterAllAttributeAccessOverridesForEndpoint` replaced by
    `chip::app::AttributeAccessInterfaceRegistry::Instance().UnregisterAllForEndpoint`
-   `chip::app::GetAttributeAccessOverride` replaced by
    `chip::app::AttributeAccessInterfaceRegistry::Instance().Get`
