# Access Control Guide

All Interaction Model operations (read attribute, write attribute, invoke
command, read event) are governed by access control, and will be denied (status
0x7E Access Denied) if sufficient privilege for the operation is not obtained.

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

## ACLs

ACLs are fabric-scoped data structures with the following fields:

-   Privilege
-   AuthMode
-   Subjects
-   Targets

### Privilege

Privileges are:

-   View
-   Operate
-   Manage
-   Administer

An additional `ProxyView` privilege is not yet supported in the Matter SDK.

By default, the `View` privilege is required to read attributes or events, and
the `Operate` privilege is required to write attributes or invoke commands.

However, clusters may require stricter privileges for certain operations on
certain endpoints. For example, the Access Control Cluster requires the
`Administer` privilege for all its operations.

If applicable, the ACL grants the privilege, and all less strict privileges
subsumed by it. Therefore an ACL for `Manage` privilege will work for operations
which require `Operate` or `View` privilege (but not `Administer` privilege).

### AuthMode

Authentication modes are:

-   CASE
-   Group

The ACL applies only to subjects using that authentication mode.

### Subjects

Subjects is a list containing zero, one, or more subject identifiers, which are:

-   Node ID for CASE auth mode
-   Group ID for Group auth mode

A CASE subject may be a CAT, which has its own tag and version mechanism.

The ACL applies only to the listed subjects; if no subjects are listed, the ACL
applies to any subjects using the authentication mode.

### Targets

Targets is a list containing zero, one, or more structured entries with fields:

-   Cluster
-   Endpoint
-   DeviceType

All fields are nullable, but at least one must be present, and the endpoint and
device type fields are mutually exclusive (only one of those two may be
present).

If cluster is present, the ACL is targeted to just that cluster.

If endpoint is present, the ACL is targeted to just that endpoint.

If device type is present, the ACL is targeted to just endpoints which contain
that device type (as reported by the Descriptor Cluster).

Specifying device type in targets is not yet supported in the Matter SDK.

The ACL applies only to the listed targets; if no targets are listed, the ACL
applies to any targets on the server node.

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

## Case Studies

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

## Managing ACLs Using Chip-Tool

### Usage

#### Entire List

The Access Control Cluster's `ACL` attribute is a list.

Currently, list operations for single entries (append, update, delete) are not
yet supported in the Matter SDK, so the entire list must be written to the
attribute to change any ACL.

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

The privileges are:

-   View: 1
-   Operate: 3
-   Manage: 4
-   Administer: 5

The authentication modes are:

-   CASE: 2
-   Group: 3

Some typical clusters:

-   On/Off: 6
-   Level Control: 8
-   Descriptor: 29
-   Binding: 30
-   Access Control: 31
-   Basic: 40

### Examples

#### Automatically Installed ACL

After commissioning with chip-tool, assuming `CaseAdminNode` is 112233, the
automatically installed ACL is:

```
out/debug/standalone/chip-tool accesscontrol read acl 1 0
```

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

```
out/debug/standalone/chip-tool accesscontrol write acl '[{"fabricIndex": 0, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null}, {"fabricIndex": 0, "privilege": 1, "authMode": 2, "subjects": [4444, 5555, 6666], "targets": null}]' 1 0
```

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

```
out/debug/standalone/chip-tool accesscontrol write acl '[{"fabricIndex": 0, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null}, {"fabricIndex": 0, "privilege": 1, "authMode": 2, "subjects": [4444, 5555, 6666], "targets": null}, {"fabricIndex": 0, "privilege": 3, "authMode": 3, "subjects": [123, 456], "targets": [{"cluster": 6, "endpoint": null, "deviceType": null}, {"cluster": null, "endpoint": 1, "deviceType": null}, {"cluster": 8, "endpoint": 2, "deviceType": null}]}]' 1 0
```

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

## Managing ACLs Using Chip-Repl

### Usage

#### Entire List

See the important notes in the chip-tool section, as they also apply to
chip-repl.

### Null Fields

Null fields may be omitted.

OK: `Target(cluster=6, endpoint=Null, deviceType=Null)`

Also OK: `Target(cluster=6)`

The above assumes Target and Null are defined at global scope, which is not
normally the case.

#### Fabric Index

The `ACL` attribute is fabric-scoped, so each ACL has a fabric index.

The REPL ignores it when performing the actual write. Because null fields can be
omitted, simply do not provide it when writing ACLs.

When reading ACLs, the proper fabric index is shown.

#### Enums and Identifiers

The REPL accepts numerical values for enums and identifiers, but it also accepts
strongly typed values:

The privileges are:

-   Clusters.AccessControl.Enums.Privilege.kView
-   Clusters.AccessControl.Enums.Privilege.kOperate
-   Clusters.AccessControl.Enums.Privilege.kManage
-   Clusters.AccessControl.Enums.Privilege.kAdminister

The authentication modes are:

-   Clusters.AccessControl.Enums.AuthMode.kCASE
-   Clusters.AccessControl.Enums.AuthMode.kGroup

Some typical clusters:

-   Clusters.OnOff.id
-   Clusters.LevelControl.id
-   Clusters.Descriptor.id
-   Clusters.Binding.id
-   Clusters.AccessControl.id
-   Clusters.Basic.id

### Examples

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
  Clusters.AccessControl.Structs.AccessControlEntry(
    privilege = Clusters.AccessControl.Enums.Privilege.kAdminister,
    authMode = Clusters.AccessControl.Enums.AuthMode.kCase,
    subjects = [ 1 ]
  ),
  Clusters.AccessControl.Structs.AccessControlEntry(
    privilege = Clusters.AccessControl.Enums.Privilege.kView,
    authMode = Clusters.AccessControl.Enums.AuthMode.kCase,
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
  Clusters.AccessControl.Structs.AccessControlEntry(
    privilege = Clusters.AccessControl.Enums.Privilege.kAdminister,
    authMode = Clusters.AccessControl.Enums.AuthMode.kCase,
    subjects = [ 1 ]
  ),
  Clusters.AccessControl.Structs.AccessControlEntry(
    privilege = Clusters.AccessControl.Enums.Privilege.kView,
    authMode = Clusters.AccessControl.Enums.AuthMode.kCase,
    subjects = [ 4444, 5555, 6666 ],
  ),
  Clusters.AccessControl.Structs.AccessControlEntry(
    privilege = Clusters.AccessControl.Enums.Privilege.kOperate,
    authMode = Clusters.AccessControl.Enums.AuthMode.kGroup,
    subjects = [ 123, 456 ],
    targets = [
      Clusters.AccessControl.Structs.Target(
        cluster = Clusters.OnOff.id,
      ),
      Clusters.AccessControl.Structs.Target(
        endpoint = 1,
      ),
      Clusters.AccessControl.Structs.Target(
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
