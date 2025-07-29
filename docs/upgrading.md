# Upgrading notes

## API changes and code migration

### `AttributePersistenceProvider`

`AttributePersistenceProvider` was moved to `src/app/persistence` and its
interface was updated:

-   Read/write operate over pure buffers, without type information

This update was done so that the interface is decoupled from ember and metadata
types. The reasons for this approach:

-   simpler/more modular code (easier to maintain)
-   Have more generic storage support (including variable size data)
-   Ability to preserve backward compatibility with existing products without
    increasing flash size by adding additional abstraction layers

Callers will validate data validity on read instead of relying on data
validation by the persistence provider.

See <https://github.com/project-chip/connectedhomeip/pull/39693> for changes.

The only change is that the `EmberAfAttributeMetadata` argument is not passed in
anymore into `Read` and implementations are expected to just return the opaque
data stored. API updates changed:

-   FROM OLD

    ```cpp
    ReturnErrorOnFailure(ReadValue(aPath, aMetadata, aByteSpan));
    ```

-   TO NEW

    ```cpp
    ReturnErrorOnFailure(ReadValue(aPath, aByteSpan));
    ReturnErrorOnFailure(ValidateData(aByteSpan, aMetadata));
    ```

    Where callers would implement `ValidateData`. Implementations do not have
    the use of aMetadata anymore and cluster data is opaque now.

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

### `ServerInitParams::dataModelProvider` in `Server::Init` and `FactoryInitParams`

Server and controller initialization require a set data model provider to work
rather than auto-initializing ember-compatible code-generated data models.

To preserve `codegen/zap` generated logic, use
`CodegenDataModelProviderInstance` (see changes in
[36558](https://github.com/project-chip/connectedhomeip/pull/36558) and
[36613](https://github.com/project-chip/connectedhomeip/pull/36613) ).

To use default attribute persistence, you need to pass in a
`PersistentStorageDelegate` to `CodegenDataModelProviderInstance`. See example
changes in [36658](https://github.com/project-chip/connectedhomeip/pull/36658)
).
