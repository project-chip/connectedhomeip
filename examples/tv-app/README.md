# Matter TV Example (Media Player)

The `tv-app` is a reference Media Player device. By default it runs as a
**Casting Video Player** (device type `0x0023`), but it can also be exercised as
any of the other three media player device types defined by the Matter Device
Library and the Media Player Architecture.

For build, commissioning, casting, and App Platform instructions see
[linux/README.md](linux/README.md).

## Media player device types

There are four media player device types. They share the same "media playback"
feature set and differ by whether they also do content launching and whether
they act as a **commissioner** (the "casting" role) versus a plain
**commissionable node** (the "basic"/"streaming" role):

| Device type            | ID       | Rev | Role               | Minimum feature set                                     |
| ---------------------- | -------- | --- | ------------------ | ------------------------------------------------------- |
| Basic Video Player     | `0x0028` | 2   | Commissionable     | Media playback + keypad (On/Off, Media Playback, Keypad Input) |
| Casting Video Player   | `0x0023` | 2   | Commissioner       | Basic Video Player + content launch (Content Launcher)  |
| Streaming Audio Player | `0x0020` | 1   | Commissionable     | Media playback + content launch (Media Playback, Content Launcher) |
| Casting Audio Player   | `0x0021` | 1   | Commissioner       | Streaming Audio Player + commissioning                  |

The authoritative cluster requirements are in the spec Device Library
(`device_types/{BasicVideoPlayer,CastingVideoPlayer,StreamingAudioPlayer,CastingAudioPlayer}`)
and the Media Player Architecture chapter. Endpoint 1 of `tv-app.zap` already
hosts a **superset** of clusters (On/Off, Media Playback, Keypad Input, Channel,
Media Input, Low Power, Target Navigator, Audio Output, Content Launcher,
Application Launcher, Content Control, Media File Management, ...), so it
satisfies the mandatory cluster set of all four types.

## Selecting the device type

The device type is expressed in **two independent places**:

1. The **declared** device type — endpoint 1's `deviceType` in
   [tv-common/tv-app.matter](tv-common/tv-app.matter) / `tv-app.zap`, surfaced at
   runtime through the Descriptor cluster `DeviceTypeList`. This is what a
   controller reads to learn what the device is, and what certification checks.
2. The **advertised** device type — the compile-time
   `CHIP_DEVICE_CONFIG_DEVICE_TYPE` in
   [tv-common/include/CHIPProjectAppConfig.h](tv-common/include/CHIPProjectAppConfig.h),
   which feeds only the DNS-SD `_T<id>` commissioning subtype used for discovery
   filtering.

### Runtime: `--device-type` (Linux)

The Linux `tv-app` accepts a flag that presents endpoint 1 as a different media
device type at boot, without a rebuild:

```
./out/debug/chip-tv-app --device-type basic-video
./out/debug/chip-tv-app --device-type casting-audio
./out/debug/chip-tv-app --device-type streaming-audio
./out/debug/chip-tv-app --device-type casting-video   # the default
```

Accepted values: `casting-video` (default), `basic-video`, `casting-audio`,
`streaming-audio`.

**What it changes:**

-   The **Descriptor cluster `DeviceTypeList`** on endpoint 1 (via
    `emberAfSetDeviceTypeList`, applied in `ApplicationInit`). This is what a
    commissioner reads to learn the device type.
-   The **DNS-SD `_T<id>` advertising subtype** (via
    `ConfigurationMgr().SetDeviceTypeId`, applied during argument parsing so it
    is in place before the server starts advertising). Commissioners that filter
    discovery by device-type subtype see the selected type. The override is not
    persisted across reboots.

Together these are enough to have the device *advertise as, declare itself as,*
and be tested as any of the four types, because endpoint 1 already exposes the
superset of clusters described above.

**What it does NOT change (by design):**

-   The **commissioner vs. commissionable role**. The Linux `tv-app` is built as
    a combined server + commissioner (it runs the UDC/CommissionerDiscovery
    machinery and the `controller`/`app` shell commands). Selecting
    `basic-video` or `streaming-audio` declares the commissionable-only type but
    does not disable the commissioner stack; the app still behaves as a
    commissioner.
-   The **cluster set**. No clusters are added or removed; the endpoint keeps its
    superset.

So the runtime flag is intended for exercising controllers, discovery, and the
advertised/declared device type against each type — not for producing a
byte-faithful build of a shipping product of that type (the cluster set and
commissioner role are still those of the compiled Casting Video Player).

### Build time: a faithful variant

For a variant that is also faithful in cluster set and role, change the data
model (and, optionally, the compile-time default device type) and rebuild:

1. Edit endpoint 1's device type and trim the endpoint's clusters to the target
   type's requirements in `tv-common/tv-app.zap` (open it with ZAP), then
   regenerate `tv-app.matter` and the generated code. Note that the two audio
   player types (`0x0020`, `0x0021`) are new spec additions and are not yet
   present in `src/app/zap-templates/zcl/data-model/chip/matter-devices.xml`, so
   ZAP's device-type dropdown will need those device types added there first.
2. For a commissionable-only type (Basic Video Player, Streaming Audio Player),
   the commissioner-specific behavior would additionally need to be gated off
   (e.g. `CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE`).
3. Optionally set `CHIP_DEVICE_CONFIG_DEVICE_TYPE` in
   [tv-common/include/CHIPProjectAppConfig.h](tv-common/include/CHIPProjectAppConfig.h)
   to the target device type ID, so the build advertises that type by default
   without needing the `--device-type` flag.

A cleaner long-term approach would be a build (`gn`) argument that selects among
per-type ZAP/data-model files and the matching compile constant, mirroring how
other examples ship multiple variants. That is not implemented today; the
runtime flag above is the supported way to switch the advertised/declared type.
