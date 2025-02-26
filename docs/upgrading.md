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

### Decoupling of Command Handler Interface from Ember

#### EnumerateGeneratedCommands

Changed the old callback based iteration, into a ListBuilder based approach for
the Enumeration of Generated Commands

`CommandHandlerInterface::EnumerateGeneratedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context)`
becomes
`CommandHandlerInterface::EnumerateGeneratedCommands(const ConcreteClusterPath & cluster, ListBuilder<CommandId> & builder)`

Changes for implementation

-   old

```cpp
   for (auto && cmd : { ScanNetworksResponse::Id, NetworkConfigResponse::Id, ConnectNetworkResponse::Id })
   {
       VerifyOrExit(callback(cmd, context) == Loop::Continue, /**/);
   }
```

-   new

```cpp
   ReturnErrorOnFailure(
        builder.AppendElements({ ScanNetworksResponse::Id, NetworkConfigResponse::Id, ConnectNetworkResponse::Id })
   )
```

#### EnumerateAcceptedCommands

Expanded `EnumerateAcceptedCommands` Interface to provide Attributes and
Privileges using a ListBuilder

`CommandHandlerInterface::EnumerateAcceptedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context)`
becomes
`CommandHandlerInterface::EnumerateAcceptedCommands(const ConcreteClusterPath & cluster, ListBuilder<AcceptedCommandEntry> & builder)`

Changes for implementation:

-   Old

```cpp
    for (auto && cmd : {
             Disable::Id,
             EnableCharging::Id,
         })
    {
        VerifyOrExit(callback(cmd, context) == Loop::Continue, /**/);
    }

    if (HasFeature(Feature::kV2x))
    {
        VerifyOrExit(callback(EnableDischarging::Id, context) == Loop::Continue, /**/);
    }
```

-   new

```cpp
    using QF   = DataModel::CommandQualityFlags;
    using Priv = chip::Access::Privilege;

    ReturnErrorOnFailure(builder.AppendElements({
        { Disable::Id,        QF::kTimed, Priv::kOperate },
        { EnableCharging::Id, QF::kTimed, Priv::kOperate },
    }));

    if (HasFeature(Feature::kV2x))
    {
        ReturnErrorOnFailure(builder.EnsureAppendCapacity(1));
        ReturnErrorOnFailure(
            builder.Append({ EnableDischarging::Id, QF::kTimed, Priv::kOperate})
        );
    }

```

Important Notes:

Be sure to use `EnsureAppendCapacity` before single element Append
`ListBuilder::Append` as it will not grow the buffer if not available already
