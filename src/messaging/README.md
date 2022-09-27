# Message Layer

## Debug Prints

When progress logging is enabled, the message layer emits a number of prints
that provide more details on the messages being sent or received. Most of these
are fairly self explanatory with the exception of the ones listed below.

### Message Transmission

#### Legend

On message transmission, the following print is emitted. The various fields that
will be expanded are denoted with `$` .

Unless specified, numerical values are represented in decimal notation.

```
<<< [E:$exchange_id M: $msg_id (Ack: $ack_msg_id)] ($msg_category) Msg TX to $fabric_index:$destination [$compressed_fabric_id] --- Type $protocol_id:$msg_type ($protocol_name:$msg_type_name)
```

| Field                | Description                                                                                                                            |
| -------------------- | -------------------------------------------------------------------------------------------------------------------------------------- |
| exchange_id          | Exchange ID + `IsInitiator` flag from message header ('i' if initiator, 'r' if responder')                                             |
| msg_id               | Message counter                                                                                                                        |
| ack_msg_id           | If present, the ACK message counter. Otherwise, this entire field is omitted from display                                              |
| msg_category         | U: Un-secure Unicast, S: Secure Unicast, G: Secure Groupcast                                                                           |
| fabric_index         | Fabric index on the sending side                                                                                                       |
| destination          | 64-bit Node Identifier that can represent both group, operational and temporary node identifiers depending on `$msg_category` (in hex) |
| compressed_fabric_id | If present and valid, lower 16-bits of the compressed fabric ID (in hex). Otherwise, it will be set to 0000.                           |
| protocol_id          | 16-bit Protocol ID within the common vendor namespace (in hex)                                                                         |
| msg_type             | 8-bit message type ID (in hex)                                                                                                         |
| protocol_name        | If available, a logical name for the protocol                                                                                          |
| msg_type_name        | If available, a logical name for the message type                                                                                      |

#### Examples:

_Unencrypted Unicast:_

```
<<< [E:26341i M: 264589322] (U) Msg TX to 0:0000000000000000 [0000] --- Type 0000:30 (SecureChannel:CASE_Sigma1)
```

_Secure Unicast:_

```
<<< [E:26347i M: 30642895 (Ack: 9203233)] (S) Msg TX to 1:0000000012344321 [1667] --- Type 0001:06 (IM:WriteRequest)
```

_Secure Groupcast:_

```
<<< [E:26349i M: 2000] (G) Msg TX to 1:FFFFFFFFFFFF0102 [1667] --- Type 0001:06 (IM:WriteRequest)
```

### Message Reception

#### Legend

On message transmission, the following print is emitted. The various fields that
will be expanded are denoted with `$` .

Unless specified, numerical values are represented in decimal notation.

```
>>> [E:$exchange_id M: $msg_id (Ack: $ack_msg_id)] ($msg_category) Msg RX from $fabric_index:$source [$compressed_fabric_id] --- Type $protocol_id:$msg_type ($protocol_name:$msg_type_name)
```

This has a similar legend to that for transmission except `$source` denotes the
source's node identifier and `$fabric_index` is the index on the recipient.

#### Examples

_Unencrypted Unicast:_

```
>>> [E:26341i M: 264589322] (U) Msg RX from 0:0FDE2AE2EAF5D74D [0000] --- Type 0000:30 (SecureChannel:CASE_Sigma1)
```

_Secure Unicast:_

```
>>> [E:26342i M: 30642885] (S) Msg RX from 1:000000000001B669 [1667] --- Type 0001:08 (IM:InvokeCommandRequest)
```

_Secure Groupcast:_

```
>>> [E:26349i M: 2000] (G) Msg RX from 1:000000000001B669 [0000] --- Type 0001:06 (IM:WriteRequest)
```
