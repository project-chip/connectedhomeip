# IDL based codegen

## What is a matter IDL

A matter IDL is a text-file that aims to be a concise representation of data
structures, cluster definitions and bindings. It is intended for human
readability (has to be clear and concise, supports comments) as well as machine
parsing (well defined syntax, not free form).

The actual grammar is defined as a
[Lark](https://lark-parser.readthedocs.io/en/latest/index.html) parsing grammar,
however it may be easier to understand with an example:

```
/* C++ comments are supported */
// so are C-style ones


// Every cluster has an identifier that is defined by the matter spec (31 in this case)
cluster AccessControl = 31 {

  // Revision of the cluster being described here
  // If not specified, it is assumed to be 1
  revision 3;

  // Enums and structs can be defined globally or be cluster specific.
  // IDL generation rules will take into account scoping (i.e. pick local defined
  // name first, things defined in one cluster are not visible in another).
  enum AuthMode : ENUM8 {
    kPase = 1;
    kCase = 2;
    kGroup = 3;
  }

  // structures may be fabric scoped by tagging them as 'fabric_scoped'
  // in a fabric scoped structure, fields may be 'fabric_sensitive'
  fabric_scoped struct AccessControlEntry {
    fabric_idx fabricIndex = 0;
    fabric_sensitive Privilege privilege = 1;
    fabric_sensitive AuthMode authMode = 2;
    nullable fabric_sensitive INT64U subjects[] = 3;  // fields in structures may be lists and
    nullable fabric_sensitive Target targets[] = 4;   // they may have attributes: nullable
  }

  // request structures are regular structures that are used
  // as command inputs. Special tagging to make the use clear.
  request struct AnnounceOTAProviderRequest {}
  request struct ConnectNetworkRequest {
    OCTET_STRING networkID = 0;
    INT64U breadcrumb = 1;
  }

  // Response structures are used for command outputs
  // Responses are encoded as a command and use a unique ID for encoding
  response struct ConnectNetworkResponse = 123 {
    CHAR_STRING debugText = 1;
    INT32S errorValue = 2;
  }

  // events can be specified with a type (critical/info) and may contain data
  critical event StartUp = 0 {
       INT32U softwareVersion = 0;
  }

  // no-data events are supported
  info event Leave = 2 {
  }

  // events default to 'view' privilege however this can be modified
  info event access(read: manage) RestrictedEvent = 3 {
  }

  attribute AccessControlEntry acl[] = 0;    // attributes are read-write by default
  attribute ExtensionEntry extension[] = 1;  // and require a (spec defined) number

  // attributes may require timed writes
  timedwrite attribute int16u require_timed_writes = 3;

  // Access control privileges on attributes default to:
  //
  // access(read: view, write: operate)
  //
  // These defaults can be modified to any of view/operate/manage/administer roles.
  attribute access(read: manage, write: administer) int32u customAcl = 3;

  // attributes may be read-only as well
  readonly attribute int16u clusterRevision = 65533;

  // Commands have spec-defined numbers which are used for over-the-wire
  // invocation.
  //
  // Commands have input and output data types, generally encoded as structures.
  command ConnectNetwork(ConnectNetworkRequest): ConnectNetworkResponse = 0;

  // An output is always available even if just for ok/failure, however
  // IDLs specifically do not define a structure for DefaultSuccess.
  //
  // which is considered an internal type.
  command AnnounceOTAProvider(AnnounceOTAProviderRequest): DefaultSuccess = 1;

  // Some commands may take no inputs at all
  command On(): DefaultSuccess = 2;
  command Off(): DefaultSuccess = 3;

  // command invocation default to "operate" privilege, however these
  // can be modified as well
  command access(invoke: administer) Off(): DefaultSuccess = 4;

  // command invocation can require timed invoke usage
  timed command access(invoke: administer) RevokeCommissioning(): DefaultSuccess = 2;

  // commands may be fabric scoped
  fabric command ViewGroup(ViewGroupRequest): ViewGroupResponse = 1;

  // commands may have multiple attributes
  fabric timed command RequiresTimedInvoke(): DefaultSuccess = 7;

  // Items may have a prefix about api stability.
  //   - "provisional" are generally subject to change
  //   - "internal" are for internal SDK development/usage/testing

  provisional critical event StartUp = 0 {
       INT32U softwareVersion = 0;
  }
  internal struct SomeInternalStruct {}

  struct StructThatIsBeingChanged {
    CHAR_STRING debugText = 1;
    provisional INT32S errorValue = 2;
  }

  provisional timedwrite attribute int16u attributeInDevelopment = 10;
  internal command FactoryReset(): DefaultSuccess = 10;
}

// Clusters may be provisional or internal as well
provisional cluster SomeClusterInDevelopment = 1234 {
  /// ... content removed
}

// On every endpoint number (non-dynamic)
// a series of clusters can be exposed
endpoint 0 {
  // A binding cluster is a cluster that can be bound to for the
  // application to make use.
  //
  // As an example, a light switch can be bound to a light bulb or
  // a cluster can be bound to a OTA provider to use for updates.
  binding cluster OtaSoftwareUpdateProvider;

  // A server cluster is a server that gets exposed to the world.
  //
  // As an example, a light bulb may expose a OnOff cluster.
  server  cluster OtaSoftwareUpdateRequestor {

    // Each endpoint server cluster instantiations will have individual
    // attributes chosen for storage/defaults
    //
    // If no storage default is given, the value is initialized with 0/false/empty
    //
    // Defaults are currently only supported for primitive types (i.e. not
    // list/struct/array, but supports strings)

    ram attribute zeroInit;                    // initialized with 0.
    ram attribute stringDefault default="abc"; // Strings can have defaults.
    ram attribute boolDefault   default=true;  // bools can have defaults.
    ram attribute inRam default=123;           // stored in RAM, lost on reboot.
    persist attribute persist;                 // persisted in NVM across reboot.
    callback attribute usesCallback;           // the zap/ember 'EXTERNAL' callback.
  }
}

```

## Parsing of IDLs

IDL parsing is done within the `matter_idl` python package (this is the current
directory of this README). Most of the heavy lifting is done by the lark using
[matter_grammar.lark](./matter_grammar.lark), which is then turned into an AST:

-   [matter_grammar.lark](./matter_grammar.lark) parses and validates textual
    content
-   [matter_idl_parser.py](./matter_idl_parser.py) has a transformer that
    converts the text given by lark into a more type-safe (and type-rich) AST as
    defined ing [matter_idl_types.py](./matter_idl_types.py)

## Code generation

Code generators are defined in `generators` and their purpose is to convert the
parsed AST into one or several output files. In most cases the output will be
split per cluster so that large CPP files are not generated (faster and more
parallel compilation).

### Code generator base functionality

Generators use [Jinja2](https://jinja.palletsprojects.com/en/3.0.x/) as a
templating language. The general `CodeGenerator` class in
[generators/**init**.py](./generators/__init__.py) provides the ability to
output files based on jinja templates.

In order to build working jinja2 templates, some further processing of the AST
data is required. Some facilities for lookup namespacing (e.g. search for named
data types within cluster first then globally) as well interpretation of data
types into more concrete types is provided by `generators/types.py`.

### Implementing generators

Beyond default AST processing, each generator is expected to add
language-specific filters to create templates that work. This includes:

-   add any additional filters and transforms for data
-   add any additional type processing that is language specific

See the java code generator in `generators/java` as an example of codegen.

### Testing generators

Tests of generation are based on checking that a given input matches an expected
output. The [tests/available_tests](./test/available_tests.yaml) describe for
each input and generator the expected output.

Intent for tests is to be focused and still easy to see deltas. Input IDLs are
expected to be small and focusing on a specific functionality. Keep in mind that
the test outputs are expected to be human-reviwed when codegen logic changes.

These generator tests are run by `test_generators.py`.
