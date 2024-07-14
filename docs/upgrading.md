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
