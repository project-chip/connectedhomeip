# Access Control Guide

All Interaction Model operations in Matter must be verified by the the Access
Control mechanism.

Whenever a client device and a server device want to interact with one another
by reading (or subscribing) attributes or events, writing attributes, or
invoking commands, the Access Control mechanism must verify that the client has
sufficient privileges to perform the operation on the server device.

If no sufficient privilege is obtained, the operation cannot take place and it
is denied (status `0x7E Access Denied`).

This guide describes how the Access Control mechanism works and how it is
implemented, and provides examples of Access Control Lists (ACLs) for different
use cases.

<hr>

## Overview

The initial `Administer` privilege is obtained by the commissioner implicitly
over the PASE commissioning channel. This implicit `Administer` privilege is
used to invoke commands on the server node during commissioning.

As part of commissioning, the commissioner invokes the `AddNOC` command,
providing a `CaseAdminNode` argument. As well as commissioning the server node
onto the fabric, that command automatically installs an ACL on the server node
for `Administer` privilege using the provided `CaseAdminNode` argument as CASE
subject. The subject may be a single node, or multiple nodes (by providing a
CAT). These nodes become fabric administrators on that server node.

During and after commissioning, administrators manage ACLs for the fabric on the
server node by reading and writing the fabric-scoped `ACL` attribute of the
Access Control Cluster, which is always present on endpoint 0 on the server
node. It is these ACLs that govern which Interaction Model operations are
allowed or denied on that server node, for subjects on the fabric, via CASE and
group messaging.

<hr>

## Access Control Lists

Access Control Lists (ACLs) are fabric-scoped data structures with the following
fields:

-   `Privilege`
-   `AuthMode`
-   `Subjects`
-   `Targets`

### Privilege field

The `Privilege` can be of the following types:

-   View
-   Operate
-   Manage
-   Administer

> **Note:** An additional `ProxyView` privilege is not yet supported in the
> Matter SDK.

By default, the `View` privilege is required to read attributes or events, and
the `Operate` privilege is required to write attributes or invoke commands.

Clusters may also require stricter privileges for certain operations on certain
endpoints. For example, the Access Control Cluster requires the `Administer`
privilege for all its operations.

If applicable, the ACL grants the privilege, and all less strict privileges
subsumed by it. Therefore an ACL for `Manage` privilege will work for operations
which require `Operate` or `View` privilege (but not `Administer` privilege).

### AuthMode field

The `AuthMode`, that is authentication modes, can be as follow:

-   CASE
-   Group

The ACL applies only to subjects using that authentication mode.

### Subjects field

The `Subjects` field is a list containing zero, one, or more subject
identifiers, which are:

-   Node ID for CASE `AuthMode`
-   Group ID for Group `AuthMode`

A CASE subject may be a CAT, which has its own tag and version mechanism.

The ACL applies only to the listed subjects; if no subjects are listed, the ACL
applies to any subjects using the authentication mode.

### Targets field

The `Targets` field is a list containing zero, one, or more structured entries
with fields:

-   Cluster
-   Endpoint
-   DeviceType

All fields are nullable, but at least one must be present, and the endpoint and
device type fields are mutually exclusive (only one of those two may be
present):

-   If cluster is present, the ACL is targeted to just that cluster.
-   If endpoint is present, the ACL is targeted to just that endpoint.
-   If device type is present, the ACL is targeted to just endpoints which
    contain that device type (as reported by the Descriptor Cluster).

> **Note:** Specifying device type in targets is not yet supported in the Matter
> SDK.

The ACL applies only to the listed targets; if no targets are listed, the ACL
applies to any targets on the server node.

<hr>

## Limitations and Restrictions

The Matter Specification states that a Matter implementation must support at
least:

-   3 ACLs per fabric
-   4 subjects per ACL
-   3 targets per ACL

There is no guarantee that an implementation supports any more than those
minimums.

In the Matter SDK, using the example access control implementation, these values
can be configured globally in `CHIPConfig.h` or per-app in
`CHIPProjectAppConfig.h` by setting:

-   CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_MAX_ENTRIES_PER_FABRIC
-   CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_MAX_SUBJECTS_PER_ENTRY
-   CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_MAX_TARGETS_PER_ENTRY

<hr>

## Case Studies

This section provides use case examples for different ACL scenarios.

### Single Administrator

A single controller commissions a server node, providing its own CASE node ID
for the automatically installed ACL:

-   Privilege: `Administer`
-   AuthMode: `CASE`
-   Subjects: `112233`
-   Targets: (all)

The controller will be able to perform all Interaction Model operations on the
server node, since it is the administrator.

### Multiple Administrators

The commissioner provides a CAT for the automatically installed ACL:

-   Privilege: `Administer`
-   AuthMode: `CASE`
-   Subjects: `0xFFFFFFFD00010001`
-   Targets: (all)

All controllers which have the CAT as part of their CASE credentials will be
administrators for the server node.

### Client Controllers

The commissioner installs an ACL for non-administrative controllers:

-   Privilege: `View`
-   AuthMode: `CASE`
-   Subjects: `4444`, `5555`, `6666`
-   Targets: (all)

The non-administrative controllers are granted `View` privilege for the entire
server node. This doesn't mean they can view the entire server node, as some
clusters may require stricter privileges for reading attributes or events.

### Group Messaging

The commissioner installs an ACL for group messaging:

-   Privilege: `Operate`
-   AuthMode: `Group`
-   Subjects: `123`, `456`
-   Targets: `{cluster:onoff}`, `{endpoint:1}`,
    `{cluster:levelcontrol,endpoint:2}`

Members of groups 123 and 456 are granted `Operate` privilege for the on/off
cluster on any endpoint, any cluster on endpoint 1, and the level control
cluster on endpoint 2.

<hr>

## Managing ACLs using CHIP-Tool

### Usage

The following sections describe the requirements for managing ACLs using the
CHIP-Tool.

#### Entire List

The Access Control Cluster's `ACL` attribute is a list.

> **Note:** Currently, list operations for single entries (append, update,
> delete) are not yet supported in the Matter SDK, so the entire list must be
> written to the attribute to change any ACL.

The write operation may employ multiple messages, making it unreliable. In any
case, ACLs are updated as they are processed, and take effect immediately.

The implication of this is that the administrator must ensure the first ACL in
the list it is writing to the `ACL` attribute is one granting itself
`Administer` privilege. Otherwise, the administrator may lose its administrative
access during the write operation.

#### Null Fields

The tool requires all fields to be provided, even if null.

Incorrect: `{"cluster": 6}`

Correct: `{"cluster": 6, "endpoint": null, "deviceType": null}`

#### Fabric Index

The `ACL` attribute is fabric-scoped, so each ACL has a fabric index.

The tool requires this field to be provided, but ignores it when performing the
actual write.

When reading ACLs, the proper fabric index is shown.

#### Enums Identifiers

The tool requires numerical values for enums and identifiers.

-   Privilege values:

    -   View: 1
    -   Operate: 3
    -   Manage: 4
    -   Administer: 5

-   AuthMode values:

    -   CASE: 2
    -   Group: 3

-   Values for some typical clusters:

    -   On/Off: 6
    -   Level Control: 8
    -   Descriptor: 29
    -   Binding: 30
    -   Access Control: 31
    -   Basic: 40

### Examples

This section provides examples of commands and ACL output for different
operations with the CHIP-Tool.

#### Verification of the Automatically Installed ACL

During commissioning with the CHIP-Tool, an ACL that assigns Administer rights
to the commissioner is automatically installed on the commissionee. This can be
verified using the following command:

```
out/debug/standalone/chip-tool accesscontrol read acl 1 0
```

Assuming the `CaseAdminNode` value is `112233`, the ACL command output for this
case is the following:

```
Endpoint: 0 Cluster: 0x0000_001F Attribute 0x0000_0000 DataVersion: 2578401031
  ACL: 1 entries
    [1]: {
      FabricIndex: 1
      Privilege: 5
      AuthMode: 2
      Subjects: 1 entries
        [1]: 112233
      Targets: null
    }
```

#### Installing a CASE ACL

The following command example requests the installation of a CASE ACL through a
write interaction:

```
out/debug/standalone/chip-tool accesscontrol write acl '[{"fabricIndex": 0, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null}, {"fabricIndex": 0, "privilege": 1, "authMode": 2, "subjects": [4444, 5555, 6666], "targets": null}]' 1 0
```

The resulting ACL command output for this case can look like the following one:

```
Endpoint: 0 Cluster: 0x0000_001F Attribute 0x0000_0000 DataVersion: 2578401034
  ACL: 2 entries
    [1]: {
      FabricIndex: 1
      Privilege: 5
      AuthMode: 2
      Subjects: 1 entries
        [1]: 112233
      Targets: null
     }
    [2]: {
      FabricIndex: 1
      Privilege: 1
      AuthMode: 2
      Subjects: 3 entries
        [1]: 4444
        [2]: 5555
        [3]: 6666
      Targets: null
     }
```

#### Installing a Group ACL

The following command example requests the installation of a Group ACL through a
write interaction:

```
out/debug/standalone/chip-tool accesscontrol write acl '[{"fabricIndex": 0, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null}, {"fabricIndex": 0, "privilege": 1, "authMode": 2, "subjects": [4444, 5555, 6666], "targets": null}, {"fabricIndex": 0, "privilege": 3, "authMode": 3, "subjects": [123, 456], "targets": [{"cluster": 6, "endpoint": null, "deviceType": null}, {"cluster": null, "endpoint": 1, "deviceType": null}, {"cluster": 8, "endpoint": 2, "deviceType": null}]}]' 1 0
```

The resulting ACL command output for this case can look like the following one:

```
Endpoint: 0 Cluster: 0x0000_001F Attribute 0x0000_0000DataVersion: 2578401041
  ACL: 3 entries
    [1]: {
      FabricIndex: 1
      Privilege: 5
      AuthMode: 2
      Subjects: 1 entries
        [1]: 112233
      Targets: null
     }
    [2]: {
      FabricIndex: 1
      Privilege: 1
      AuthMode: 2
      Subjects: 3 entries
        [1]: 4444
        [2]: 5555
        [3]: 6666
      Targets: null
     }
    [3]: {
      FabricIndex: 1
      Privilege: 3
      AuthMode: 3
      Subjects: 2 entries
        [1]: 123
        [2]: 456
      Targets: 3 entries
        [1]: {
          Cluster: 6
          Endpoint: null
          DeviceType: null
         }
        [2]: {
          Cluster: null
          Endpoint: 1
          DeviceType: null
         }
        [3]: {
          Cluster: 8
          Endpoint: 2
          DeviceType: null
         }
     }
```

<hr>

## Managing ACLs Using Chip-repl

### Usage

This section provides examples of commands and ACL output for different
operations with the CHIP-repl.

#### Entire List

See the important notes in the
[Managing ACLs using CHIP-Tool](#managing-acls-using-chip-tool) section, as they
also apply to the CHIP-repl.

### Null Fields

Null fields may be omitted.

This means that the following entry is acceptable:
`Target(cluster=6, endpoint=Null, deviceType=Null)`. Just as the following one:
`Target(cluster=6)`.

The above assumes Target and Null are defined at global scope, which is not
normally the case.

#### Fabric Index

The `ACL` attribute is fabric-scoped, so each ACL has a fabric index.

The CHIP-repl ignores it when performing the actual write. Because null fields
can be omitted, simply do not provide it when writing ACLs.

When reading ACLs, the proper fabric index is shown.

#### Enums and Identifiers

The CHIP-repl accepts numerical values for enums and identifiers, but it also
accepts strongly typed values:

The privileges are:

-   `Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView`
-   `Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate`
-   `Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kManage`
-   `Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister`

The authentication modes are:

-   `Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCASE`
-   `Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kGroup`

Some typical clusters:

-   Clusters.OnOff.id
-   Clusters.LevelControl.id
-   Clusters.Descriptor.id
-   Clusters.Binding.id
-   Clusters.AccessControl.id
-   Clusters.BasicInformation.id

### Examples

This section provides examples of commands and ACL output for different
operations with the CHIP-repl.

#### Automatically Installed ACL

After commissioning with chip-repl, assuming `CaseAdminNode` is 1, the
automatically installed ACL is:

```
await devCtrl.ReadAttribute(1, [ (0, Clusters.AccessControl.Attributes.Acl ) ] )
```

```
{
│   0: {
│   │   <class 'chip.clusters.Objects.AccessControl'>: {
│   │   │   <class 'chip.clusters.Attribute.DataVersion'>: 556798280,
│   │   │   <class 'chip.clusters.Objects.AccessControl.Attributes.Acl'>: [
│   │   │   │   AccessControlEntry(
│   │   │   │   │   fabricIndex=1,
│   │   │   │   │   privilege=<Privilege.kAdminister: 5>,
│   │   │   │   │   authMode=<AuthMode.kCase: 2>,
│   │   │   │   │   subjects=[
│   │   │   │   │   │   1
│   │   │   │   │   ],
│   │   │   │   │   targets=Null
│   │   │   │   )
│   │   │   ]
│   │   }
│   }
}
```

#### Installing a CASE ACL

```
await devCtrl.WriteAttribute(1, [ (0, Clusters.AccessControl.Attributes.Acl( [
  Clusters.AccessControl.Structs.AccessControlEntryStruct(
    privilege = Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
    authMode = Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
    subjects = [ 1 ]
  ),
  Clusters.AccessControl.Structs.AccessControlEntryStruct(
    privilege = Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
    authMode = Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
    subjects = [ 4444, 5555, 6666 ],
  ),
] ) ) ] )
```

```
{
│   0: {
│   │   <class 'chip.clusters.Objects.AccessControl'>: {
│   │   │   <class 'chip.clusters.Attribute.DataVersion'>: 556798289,
│   │   │   <class 'chip.clusters.Objects.AccessControl.Attributes.Acl'>: [
│   │   │   │   AccessControlEntry(
│   │   │   │   │   fabricIndex=1,
│   │   │   │   │   privilege=<Privilege.kAdminister: 5>,
│   │   │   │   │   authMode=<AuthMode.kCase: 2>,
│   │   │   │   │   subjects=[
│   │   │   │   │   │   1
│   │   │   │   │   ],
│   │   │   │   │   targets=Null
│   │   │   │   ),
│   │   │   │   AccessControlEntry(
│   │   │   │   │   fabricIndex=1,
│   │   │   │   │   privilege=<Privilege.kView: 1>,
│   │   │   │   │   authMode=<AuthMode.kCase: 2>,
│   │   │   │   │   subjects=[
│   │   │   │   │   │   4444,
│   │   │   │   │   │   5555,
│   │   │   │   │   │   6666
│   │   │   │   │   ],
│   │   │   │   │   targets=Null
│   │   │   │   )
│   │   │   ]
│   │   }
│   }
}
```

#### Installing a Group ACL

```
await devCtrl.WriteAttribute(1, [ (0, Clusters.AccessControl.Attributes.Acl( [
  Clusters.AccessControl.Structs.AccessControlEntryStruct(
    privilege = Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
    authMode = Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
    subjects = [ 1 ]
  ),
  Clusters.AccessControl.Structs.AccessControlEntryStruct(
    privilege = Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
    authMode = Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
    subjects = [ 4444, 5555, 6666 ],
  ),
  Clusters.AccessControl.Structs.AccessControlEntryStruct(
    privilege = Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
    authMode = Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kGroup,
    subjects = [ 123, 456 ],
    targets = [
      Clusters.AccessControl.Structs.AccessControlTargetStruct(
        cluster = Clusters.OnOff.id,
      ),
      Clusters.AccessControl.Structs.AccessControlTargetStruct(
        endpoint = 1,
      ),
      Clusters.AccessControl.Structs.AccessControlTargetStruct(
        cluster = Clusters.LevelControl.id,
        endpoint = 2,
      ),
    ]
  ),
] ) ) ] )
```

```
{
│   0: {
│   │   <class 'chip.clusters.Objects.AccessControl'>: {
│   │   │   <class 'chip.clusters.Attribute.DataVersion'>: 556798301,
│   │   │   <class 'chip.clusters.Objects.AccessControl.Attributes.Acl'>: [
│   │   │   │   AccessControlEntry(
│   │   │   │   │   fabricIndex=1,
│   │   │   │   │   privilege=<Privilege.kAdminister: 5>,
│   │   │   │   │   authMode=<AuthMode.kCase: 2>,
│   │   │   │   │   subjects=[
│   │   │   │   │   │   1
│   │   │   │   │   ],
│   │   │   │   │   targets=Null
│   │   │   │   ),
│   │   │   │   AccessControlEntry(
│   │   │   │   │   fabricIndex=1,
│   │   │   │   │   privilege=<Privilege.kView: 1>,
│   │   │   │   │   authMode=<AuthMode.kCase: 2>,
│   │   │   │   │   subjects=[
│   │   │   │   │   │   4444,
│   │   │   │   │   │   5555,
│   │   │   │   │   │   6666
│   │   │   │   │   ],
│   │   │   │   │   targets=Null
│   │   │   │   ),
│   │   │   │   AccessControlEntry(
│   │   │   │   │   fabricIndex=1,
│   │   │   │   │   privilege=<Privilege.kOperate: 3>,
│   │   │   │   │   authMode=<AuthMode.kGroup: 3>,
│   │   │   │   │   subjects=[
│   │   │   │   │   │   123,
│   │   │   │   │   │   456
│   │   │   │   │   ],
│   │   │   │   │   targets=[
│   │   │   │   │   │   Target(
│   │   │   │   │   │   │   cluster=6,
│   │   │   │   │   │   │   endpoint=Null,
│   │   │   │   │   │   │   deviceType=Null
│   │   │   │   │   │   ),
│   │   │   │   │   │   Target(
│   │   │   │   │   │   │   cluster=Null,
│   │   │   │   │   │   │   endpoint=1,
│   │   │   │   │   │   │   deviceType=Null
│   │   │   │   │   │   ),
│   │   │   │   │   │   Target(
│   │   │   │   │   │   │   cluster=8,
│   │   │   │   │   │   │   endpoint=2,
│   │   │   │   │   │   │   deviceType=Null
│   │   │   │   │   │   )
│   │   │   │   │   ]
│   │   │   │   )
│   │   │   ]
│   │   }
│   }
}
```
