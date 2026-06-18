# Multi-Image OTA — Design Specification

---

## 1. Overview

The Matter OTA Requestor today downloads a single binary per OTA session and
hands every byte to one OTA image processor implementation. This is
sufficient for devices where only the host firmware changes.

Modern IoT products routinely contain multiple programmable components: a
co-processor, a radio module, a display controller, a peripheral hub. Each
component ships its own firmware and may need updating independently. Running
separate OTA sessions per component — one per BDX transfer — requires
multi-session orchestration that the standard OTA Requestor does not support
and that providers are not required to handle.

This design solves the problem in a single BDX session by bundling all
component firmwares into one `.ota` file with a structured header. The host
device downloads the bundle once, routes each component's bytes to the right
handler, and skips components that are not ready or do not need updating — all
without any changes to the OTA Requestor, Provider protocol, or BDX layer.

Two principles shape everything below:

1. **The primary application firmware OTA path is unchanged.** Every bundle
   contains a new application image, and that image flows through the exact
   `esp_ota_begin` → `esp_ota_write` → `esp_ota_end` →
   `esp_ota_set_boot_partition` sequence used today. Multi-image OTA *adds*
   routing for co-processor binaries around that path; it does not modify it.
2. **The framework routes bytes; the application owns component state.**
   The framework keeps only ephemeral, session-scoped routing state. Anything
   that must survive a reboot — expected component versions, update
   confirmation policy, recovery from mixed-version states — belongs to the
   application, anchored on compile-time constants and runtime queries rather
   than persisted records.

The entire subsystem is gated behind a new build flag (§11). When the flag is
disabled, the existing single-image `OTAImageProcessorImpl` is built and
nothing in this document applies — the current implementation is untouched.

---

## 2. Glossary

| Term                    | Definition                                                                                                                                          |
| ----------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Bundle**              | A single `.ota` file containing a `MultiImageHeader` (which embeds one `SubImageHeader` per component in its `subImages[]` field) followed by all component binaries concatenated. Always contains the application image as the last entry (§3.5). |
| **Main Image Processor**| The OTA image processor implementation the Matter OTA Requestor calls. Owns the full download lifecycle and contains the Dispatcher.                |
| **Dispatcher**          | The routing engine inside the Main Image Processor. Parses the bundle header, maintains the `imageId → sub-processor` map, and routes or skips each component's bytes. |
| **Sub Image Processor** | An application-provided handler for one component. Implements `Init(entry)`, `IsReadyForOTA()`, `Write(block)`, and `Abort(context)`.               |
| **Image ID**            | A 4-byte unsigned integer (uint32, `0x00000000`–`0xFFFFFFFF`) that identifies which sub-processor handles a given component. Stable ABI between firmware and packaging tool. |
| **`SkipData(n)`**       | BDX `BlockQueryWithSkip` — tells the Provider to advance its cursor by `n` bytes without delivering them.                                          |
| **Outcome hook**        | `OnDownloadOutcome()` — an overridable operation on the Main Image Processor, invoked once after a successful download with the per-entry results. The application's chance to observe skips and veto `Apply()` (§5). |
| **Confirmation**        | The application-owned step that accepts the new firmware after first boot: Matter-level (`NotifyUpdateApplied`) plus bootloader-level (`esp_ota_mark_app_valid_cancel_rollback`). See §6.            |
| **`softwareVersion`**   | The Matter bundle version in the outer OTA header. Represents the state where every component in the bundle is at its expected version.             |

---

## 3. OTA File Format

The Matter OTA file header (vendor id, product id, software version, payload
size, …) is unchanged. The **payload** now begins with a `MultiImageHeader` that
describes every binary inside the bundle — including an embedded `subImages[]`
array of `SubImageHeader` entries — followed by the binary data blobs at the
offsets the header specifies. The dispatcher parses the full `MultiImageHeader`
(fixed fields then `subImages[]`) first, then routes each binary's bytes to the
correct processor — using `SkipData()` for any binary whose processor is absent
(assumed up to date) or not ready.

### 3.1 Outer layout

```
+---------------------------------------------------------+
| Matter OTA header (chip-ota-image-tool format)          |
|   vendor_id, product_id, software_version, ...          |
+---------------------------------------------------------+  ← offset 0 of payload
| MultiImageHeader                                        |
|   magic(4)  num_images(1)  reserved(3)                  |
|   subImages[0]   : SubImageHeader  (48 bytes)           |
|   subImages[1]   : SubImageHeader  (48 bytes)           |
|   ...                                                   |
|   subImages[N-1] : SubImageHeader  (48 bytes)           |
+---------------------------------------------------------+
| Binary 0 data   (at subImages[0].offset bytes)          |
+---------------------------------------------------------+
| Binary 1 data   (at subImages[1].offset bytes)          |
+---------------------------------------------------------+
| ...                                                     |
+---------------------------------------------------------+
| Binary N-1 data — ALWAYS the application image (§3.5)   |
+---------------------------------------------------------+
```

All `offset` values are relative to the start of the OTA payload (byte 0
immediately after the outer Matter OTA header ends). All multi-byte fields are
**little-endian**.

### 3.2 MultiImageHeader

`MultiImageHeader` is the single header structure at the start of the payload.
It contains a fixed-size preamble followed by a variable-length array of
`SubImageHeader` entries (`subImages[]`). All fields are little-endian:

**Fixed preamble (8 bytes):**

| Offset | Size | Field       | Semantics                                            |
| ------ | ---- | ----------- | ---------------------------------------------------- |
| 0      | 4    | `magic`     | Must equal `0x4D494F54` ("MIOT") — bundle identifier |
| 4      | 1    | `numImages` | Number of entries in `subImages[]` (1–255; 0 is rejected — every bundle carries at least the application image) |
| 5      | 3    | `reserved`  | Must be zero; reserved for future extensions         |

**Variable-length field:**

| Offset | Size              | Field        | Semantics                                    |
| ------ | ----------------- | ------------ | -------------------------------------------- |
| 8      | `numImages × 48`  | `subImages[]`| Array of `SubImageHeader` entries; see §3.3  |

Total `MultiImageHeader` size on the wire:

```
sizeof(MultiImageHeader) = 8 + numImages * sizeof(SubImageHeader)
                         = 8 + numImages * 48
```

The dispatcher accumulates exactly `sizeof(MultiImageHeader)` bytes before it
switches from header-parsing mode to data-routing mode. Because `numImages` is
not known until the first 8 bytes are read, parsing proceeds in two steps:
1. Accumulate the 8-byte preamble; read `magic` and `numImages`.
2. Accumulate the remaining `numImages × 48` bytes to populate `subImages[]`.

### 3.3 SubImageHeader (element of `MultiImageHeader.subImages[]`)

Each entry in `MultiImageHeader.subImages[]` is a `SubImageHeader` — a fixed
48-byte structure regardless of the binary it describes. All fields are
little-endian:

| Offset | Size | Field     | Semantics                                                      |
| ------ | ---- | --------- | -------------------------------------------------------------- |
| 0      | 4    | `imageId` | Identifies which sub-processor handles this binary (§3.4)      |
| 4      | 4    | `version` | Expected installed version of this binary                      |
| 8      | 4    | `offset`  | Byte offset of binary data from payload start                  |
| 12     | 4    | `length`  | Exact byte count of the binary                                 |
| 16     | 32   | `sha256`  | SHA-256 digest of the binary — mandatory                      |

Field semantics:

| Field      | Notes                                                                                                                                                                                   |
| ---------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `imageId`  | Matched against registered processors at runtime. A bundle may carry `imageId`s not registered on a given device — those entries are skipped via `SkipData()` and assumed to be already up to date. |
| `version`  | Per-binary version number. Used by the sub-processor's readiness decision (compare against the component's cached installed version) and reported in the outcome hook. The OTA Requestor's update decision is driven by the outer Matter `softwareVersion` alone. |
| `offset`   | Absolute byte offset from payload start. Enables the dispatcher to jump directly to each binary with a single `SkipData()` call rather than streaming all preceding bytes. |
| `length`   | Exact byte count. Used both to know when to stop feeding the processor and to compute the skip distance for unneeded binaries.                                              |
| `sha256`  | Mandatory SHA-256 of the binary at `[offset, offset+length)`. The sub-processor must verify this digest before accepting the update as complete. |

### 3.4 Image ID space

`imageId` is a 4-byte unsigned integer (uint32, values `0x00000000`–`0xFFFFFFFF`).
Note that `imageId` is an *identifier*, not a count — how many binaries one
bundle can carry is bounded separately by the 1-byte `numImages` field
(1–255, §3.2). The identifier namespace is partitioned to avoid collisions:

| Range                        | Owner        | Use                                                                                                                                    |
| ---------------------------- | ------------ | -------------------------------------------------------------------------------------------------------------------------------------- |
| `0x00000000`                 | —            | Invalid. Rejected at parse time.                                                                                                       |
| `0x00000001 .. 0x000000FF`   | Platform     | Reserved for platform-defined well-known images. The application firmware image ID is in this range; its exact value is documented with the processor registration code. |
| `0x00000100 .. 0xFFFFFFFF`   | Manufacturer | **Manufacturer-defined.** Pick any value in this range; no central registry.                                                           |

Manufacturer-defined IDs in range `0x00000100`–`0xFFFFFFFF` carry no inherent
meaning beyond what the device firmware and packaging tool agree on. There is no
list to add to and no central authority.

The only runtime validation is **duplicate registration** —
`RegisterProcessor(imageId, ...)` fails if `imageId` is already registered. The
`imageId`-to-meaning mapping is a stable ABI contract between the device
firmware and the packaging tool. Repurposing an ID across firmware versions is
an ABI break for that product.

### 3.5 Bundle rules — application image mandatory and last

These rules are validated by the Dispatcher at parse time; a bundle violating
them is rejected before any byte is routed:

1. **Every bundle contains exactly one application firmware entry** (the
   platform-defined app image ID). This is non-negotiable for two reasons:
   the reported Matter `softwareVersion` is compiled into the application
   image, so without a new app image the device could never report the target
   version and `ConfirmCurrentImage()` would fail forever; and the app image
   carries the compile-time expected-version constants for every co-processor
   (§6.3), which is how the device knows what "consistent" means after the
   update.
2. **The application image is the last entry in the bundle.** Co-processor
   binaries come first. Rationale:
   - Irreversible external writes (flashing a co-processor over a bus) happen
     *before* the host commit point (`Apply()`), never after. By the time the
     boot partition is switched, all co-processor work has already succeeded
     or the session has already aborted.
   - The app image is always written (a legitimate bundle has a bumped
     `softwareVersion`, so the app sub-processor never skips). Because the
     last entry is never skipped, the "skip lands exactly at end-of-file"
     BDX edge case cannot occur — the final bytes of every session are always
     delivered, and EOF is always reached by a normal `Block`/`BlockEOF`, not
     a skip.
3. **Co-processor entries are ordered by write dependency.** The order of
   binaries in the file is the order sub-processors receive bytes; the
   packaging manifest controls it (§3.6, §10.5).

`numImages = 0` is rejected at parse time (rule 1 implies at least one entry).

### 3.6 Packaging tool

A packaging tool takes a manifest that describes each binary — its image ID,
version, and file path — and produces a single `.ota` file. The tool:

1. Reads each binary and computes its SHA-256 digest.
2. Computes each binary's `offset` (header section size + sum of preceding
   binary sizes).
3. Builds the `MultiImageHeader` (preamble + `subImages[]`).
4. Concatenates the header blob and binaries, then wraps the result with the
   standard outer Matter OTA header (vendor ID, product ID, software version,
   digest).

The tool enforces the bundle rules of §3.5 (app image present, app image last).
Output: a single `.ota` file ready to serve from a Matter OTA Provider.
The manifest format, tool invocation, and outer header parameters are
implementation details documented in IMPLEMENTATION.md.

---

## 4. Architecture

### 4.1 Components

The system is built from three roles:

**Main Image Processor** — the single object the Matter OTA Requestor knows
about. It implements the OTA image processor interface and owns the entire OTA
download and apply cycle: it receives every byte block from the BDX session,
drives the overall prepare → process → finalize → apply → abort lifecycle, and
contains the Dispatcher. Its `ConfirmCurrentImage()` is identical to the
single-image implementation — a version comparison only; all richer
confirmation policy lives in the application (§6).

**Dispatcher** — the routing engine inside the Main Image Processor. It parses
the `MultiImageHeader` (preamble then `subImages[]`) from the incoming byte
stream, maintains the `imageId → sub-processor` map populated by application
registration, and redirects each byte chunk to the sub-processor that matches
the current `subImages[i].imageId`. When no sub-processor is registered for an
entry, the Dispatcher records `kSkippedUpToDate` and calls `SkipData()` to
advance past it. When the matched sub-processor returns not-ready, `SkipData()`
is also called and the entry is recorded as `kSkippedNotReady`. All recorded
outcomes are reported through the outcome hook (§5) at the end of the download.

**Sub Image Processor** — an interface the application registers per image ID.
Four methods form the contract:

-   `Init(entry)` — called once with the full `SubImageHeader` for each entry,
    before `IsReadyForOTA()`. Must be light-weight: store the fields needed for
    the readiness decision (`entry.version`) and for subsequent writes
    (`entry.length`, `entry.sha256`). Do not start heavy I/O here — defer that
    to the first `Write()` call so resources are only allocated when `kReady` is
    confirmed.
-   `IsReadyForOTA()` → `DeviceState` — called immediately after
    `Init()`, with no parameters. The sub-processor uses the version stored
    during `Init()` to decide: `kReady` (proceed), `kAlreadyUpToDate` (already
    at target version, skip), `kNotReady` (skip; reported to the application
    via the outcome hook). Must return quickly — no blocking I/O. Decide from
    **cached state** (component versions queried at boot, §6.3), never from a
    live bus round-trip.
-   `Write(block)` → `WriteResult` — called per chunk, only when
    `IsReadyForOTA()` returned `kReady`. On the first chunk the sub-processor
    performs heavy initialisation. Two independent byte counters exist:
    the Dispatcher's counter (decides when to stop feeding this entry and move
    to the next) and the sub-processor's own running total (detects the last
    chunk for SHA-256 finalisation and commit logic). The sub-processor must
    maintain its own running total and compare it against `entry.length`
    (stored during `Init()`) to detect the last chunk. Returns one of two values:

    | Return value | Meaning | Dispatcher action |
    | ------------ | ------- | ----------------- |
    | `kDone`      | Write completed synchronously — the Dispatcher may immediately proceed | Call `FetchNextData()` or `SkipData()` for the next entry |
    | `kPending`   | Write is in progress asynchronously — the Dispatcher must wait | Suspend; do not call `FetchNextData()` or `SkipData()` until `OnWriteComplete()` is called back |

    A sub-processor that writes directly to a local partition (which internally
    uses platform scheduling but resolves within the same dispatch cycle) returns
    `kDone`. A sub-processor that delegates to a worker task (bus transfer, DMA)
    returns `kPending`.

-   `Abort(context)` — called on every sub-processor that had `Init()` called
    whenever the OTA session ends without completing normally — whether due to an
    error or a deliberate cancellation. `AbortContext` carries two fields:

    | Field    | Values                | Meaning                                                        |
    | -------- | --------------------- | -------------------------------------------------------------- |
    | `reason` | `kError`, `kCancelled`| `kError`: session ended due to a failure; `kCancelled`: deliberate abort by the application |
    | `error`  | platform error code   | The specific error that caused the abort; unset when `reason` is `kCancelled` |

    The Main Image Processor determines the reason using an internal `mLastError`
    field. Whenever `Write()` or any internal Dispatcher operation returns an
    error, the Main Image Processor saves it into `mLastError` before propagating
    the error to the SDK. When the SDK then calls `Abort()` (which carries no
    reason), the Main Image Processor checks `mLastError`: if set, the reason is
    `kError` with that code; if not set (the SDK cancelled without a prior
    internal failure), the reason is `kCancelled`. `mLastError` is cleared at
    the start of each new download session.

    The sub-processor may use `context.reason` to log or handle the two cases
    differently. It must discard any partially written data, call off any pending
    async operation, and release all resources acquired since `Init()`. If the
    write target was already irreversibly modified before `Abort()` arrives,
    that is recorded for the application's boot-time consistency check (§6.3) —
    component rollback is the application's responsibility (§7.1). Must not
    block.

**`OnWriteComplete(result)`** — this is **not** a sub-processor interface
method. It is an operation **on the Main Image Processor** that an
asynchronous sub-processor calls (from its worker task) when a write that
returned `kPending` finishes. It is the only mechanism that unblocks the
Dispatcher after a `kPending` return:

-   Must be called **exactly once per `kPending` `Write()`**, from a task
    context (never from within the `Write()` call itself).
-   The Main Image Processor's handler **must post its continuation back to
    the Matter thread** (via `PlatformMgr().ScheduleWork()`) before returning —
    it must not call `FetchNextData()` or any BDX operation directly from the
    worker task context. `FetchNextData()` and `SkipData()` drive the BDX
    state machine and are only safe to call from the Matter event loop thread.
-   Once resumed on the Matter thread: on success, the Dispatcher calls
    `FetchNextData()` (or advances to the next entry as applicable); on error,
    it sets `mLastError` and lets the SDK call `Abort()`.
-   The Dispatcher holds a reference to the copied block buffer until the
    Matter thread resumes after `OnWriteComplete()`; only then is the buffer
    released and the next BDX block requested.
-   Sub-processors whose `Write()` always returns `kDone` never call it.

`DeviceState` is a three-value enum — not a boolean — because the
outcome report (§5) distinguishes "already up to date" from "unavailable":

| Value              | Dispatcher action                      | Recorded outcome      |
| ------------------ | -------------------------------------- | --------------------- |
| `kReady`           | Call `Write()` per chunk               | `kWritten`            |
| `kAlreadyUpToDate` | `SkipData(skip distance)`              | `kSkippedUpToDate`    |
| `kNotReady`        | `SkipData(skip distance)`              | `kSkippedNotReady`    |

**Skipped means permanently skipped for this OTA cycle.** The Provider will not
re-send those bytes. A component whose processor returns `kNotReady` is not
updated this cycle; what happens next is the application's policy decision,
made in the outcome hook (§5) and the boot-time consistency check (§6.3).
Think of `kNotReady` as "opt out of this update," not "defer until later in
this session."

**`IsReadyForOTA()` must return immediately.** In ReceiverDrive mode the
Provider waits for the next `BlockQuery` or `BlockQueryWithSkip`. If the
Requestor goes silent for too long the Provider will terminate the session with
a status-report timeout. The readiness check must complete in milliseconds —
which is why it must consult cached component state, never the bus. Any
initialization that takes longer (bus negotiation, boot-mode handshake) must be
done before the OTA session begins or deferred to the first `Write()`.

**Mixed-version state after a `kNotReady` skip.** If one binary is updated and
another is skipped with `kNotReady`, the device ends the OTA cycle with
mismatched versions across its components. The application chooses its policy
in the outcome hook (§5) and is responsible for detecting and recovering the
mismatch at boot (§6.3, §10.6). Do not let an update proceed past a skipped
component unless the device can tolerate running the old version of that
component alongside new versions of the others.

The sub-processor owns all state tracking for its component. The framework
defines the interface; applications provide the implementations.

### 4.2 Component relationships

```
   Matter OTA Requestor
          |
          | OTA Image Processor Interface
          v
  +-------+------------------------------------------+
  |              Main Image Processor                 |
  |   mLastError — set on any internal failure        |
  |   currentBytePosition — tracks stream position    |
  |   OnWriteComplete(result) ◄── called by async     |
  |                               sub-processors      |
  |   +-------------------------------------------+  |
  |   |              Dispatcher                   |  |
  |   |   MultiImageHeader (incl. subImages[])    |  |
  |   |   imageId → sub-processor map             |  |
  |   |   per-entry outcome table (RAM only)      |  |
  |   |   routes chunks / calls SkipData()        |  |
  |   +--------+----------------------------------+  |
  +------------+-------------------------------------+
     |         |
     |         |  framework → application (once, post-Finalize)
     |         |  OnDownloadOutcome(outcomes) → kProceed | kAbort
     |         v
     |    [ Application apply policy — §5 ]
     |
     |  framework → sub-processor
     | Init(SubImageHeader) — always, light
     | IsReadyForOTA() → DeviceState
     | Write(block) → kDone | kPending
     | Abort(AbortContext) — on error or cancel
     v
  +--------------------------------------------------+
  |      Sub Image Processor (interface)             |
  |   Init(SubImageHeader)              (app-impl)  |
  |   IsReadyForOTA()                   (app-impl)  |
  |   Write(block)  → kDone|kPending    (app-impl)  |
  |   Abort(AbortContext)               (app-impl)  |
  +--------------------------------------------------+
               ^                   ^
               |                   |
       [App firmware]     [Co-processor / any]
       (registered by     (registered by
        platform layer)    application)
```

**Main Image Processor** lifecycle on the SDK interface:

- `PrepareDownload()` — called first by the OTA Requestor. Must return
  immediately (defers via `ScheduleWork`). Resets all session state: clears
  `mLastError`, resets `currentBytePosition` to zero, clears the per-entry
  outcome table, resets the header-parse state machine. Calls
  `esp_ota_get_next_update_partition()` and `esp_ota_begin()` exactly as the
  single-image implementation does today — this is safe because every bundle
  carries an application image that will be written (§3.5).
- `ProcessBlock()` — receives every BDX block. Immediately deep-copies the
  block data into an owned buffer, schedules `HandleProcessBlock()` on the
  Matter thread, and returns. Tracks `mLastError` and `currentBytePosition`.
  The normative routing algorithm is in §9.1 R2.
- `Finalize()` / `Apply()` / `Abort()` — all must return immediately and defer
  real work via `ScheduleWork`. `HandleFinalize` runs `esp_ota_end()` (which
  validates the staged application image), then invokes the outcome hook (§5).
  `HandleApply` runs `esp_ota_set_boot_partition()` unconditionally — safe,
  because the app image is always the last entry and always written. See §7
  for the ordering guarantee between `Finalize()` and the last
  `ProcessBlock()`.
- `IsFirstImageRun()` / `ConfirmCurrentImage()` — identical to the
  single-image implementation: state check and version comparison
  respectively. Confirmation *policy* is application-owned and lives in the
  requestor driver, not here (§6).

**Dispatcher** — validates each entry's offset, looks up its sub-processor,
calls `Init(entry)` and `IsReadyForOTA()`. For `kNotReady`/`kAlreadyUpToDate`
calls `SkipData()` — which is the next BDX block request; never follow it with
`FetchNextData()`. For `kReady` calls `Write()`, then either `FetchNextData()`
(`kDone`) or suspends until the sub-processor calls `OnWriteComplete()` (`kPending`).
A block may span two entries; skip distances are derived from `currentBytePosition`.
Full routing rules: §9.1 R2.

**Sub Image Processor** — receives `Init()`, `IsReadyForOTA()`, then `Write()`
per chunk if `kReady`. Full interface contract: §4.1.

**Registration** — applications call a register function associating an
`imageId` with a sub-processor instance. Registration must complete **before
`InitOTARequestor()`** (i.e., before the Matter OTA Requestor is initialised),
not merely before the first OTA session — the requestor driver's first-boot
path runs during stack initialisation. Each `imageId` may have at most one
registered processor; duplicate registration is an error. The default
application-image sub-processor is registered automatically by the platform
layer.

---

## 5. Download Outcome and Apply Policy

The framework keeps a small in-RAM table during the download: one outcome per
`subImages[]` entry.

| Outcome              | Meaning                                                                 |
| -------------------- | ----------------------------------------------------------------------- |
| `kWritten`           | Sub-processor returned `kReady`; all bytes delivered and SHA-256 verified. |
| `kSkippedUpToDate`   | Sub-processor returned `kAlreadyUpToDate`, or no processor was registered for the `imageId`. |
| `kSkippedNotReady`   | Sub-processor returned `kNotReady`.                                     |

There is no `kFailed` outcome — a failed write aborts the entire session
(`Abort()` fans out to every `Init()`-ed sub-processor) and the hook is never
invoked.

### 5.1 The outcome hook

After `HandleFinalize` succeeds (`esp_ota_end()` validated the application
image) and **before** the Requestor proceeds to `ApplyUpdateRequest`, the Main
Image Processor invokes one overridable operation:

```
ApplyDecision OnDownloadOutcome(Span<const EntryOutcome> outcomes)
// EntryOutcome = { imageId, version, outcome }
// ApplyDecision = kProceed | kAbort
```

- Runs on the Matter thread; must not block.
- Default implementation returns `kProceed`.
- Returning `kAbort` ends the cycle: `Abort()` fans out to all `Init()`-ed
  sub-processors with `reason = kCancelled`, the staged application image is
  discarded (`esp_ota_abort`), the device stays on the old firmware, and the
  next periodic `QueryImage` (still reporting the **old** `softwareVersion`,
  so the Provider will offer the bundle again) retries the whole bundle.
  No retry state is persisted — the next cycle is indistinguishable from a
  first attempt.

### 5.2 The two apply policies

The hook exists so the application can choose, per product, what a
`kSkippedNotReady` entry means:

**Policy A — best-effort (default).** Return `kProceed` regardless of skips.
The application firmware updates; a skipped co-processor stays on its old
version. Consequence to understand clearly: once the new firmware is confirmed,
the device reports the new `softwareVersion`, so `QueryImage` returns
*NotAvailable* — **the skipped component will not be updated by OTA until the
next bundle release**. Recovery is therefore the application's boot-time
consistency check (§6.3): re-flash the component from a staged partition or an
image embedded in the application firmware. Choose Policy A only if every
co-processor has such a recovery source, or if the product tolerates the
component staying stale until the next release.

**Policy B — all-or-nothing.** Return `kAbort` if any outcome is
`kSkippedNotReady`. The whole bundle is dropped and retried later from the old
firmware. The app update is delayed until every component is ready, but
components never drift, and no out-of-band recovery path is needed.

Both policies require zero persistent state. A product can also mix them —
e.g., proceed despite a skipped non-critical display controller but abort if
the radio co-processor was skipped.

> **Why there is no framework retry machinery.** A tempting alternative is to
> apply anyway, persist the list of skipped entries in NVS, and re-trigger
> `QueryImage` after the reboot to download only the missing sub-image. That
> flow cannot work with a standard Provider: after the reboot the device
> already reports the *new* `softwareVersion`, so `QueryImage` returns
> NotAvailable and the retry never starts. Policy A (recover out-of-band) and
> Policy B (don't apply) are the two flows that are actually reachable with
> spec-compliant Providers, and neither needs persistence.

---

## 6. Confirmation — Application-Owned

### 6.1 What confirmation means

Two independent commits happen after the first boot of new firmware, in this
order:

1. **Bootloader-level** — `esp_ota_mark_app_valid_cancel_rollback()`. Until
   this is called, the image is in *pending-verify* state and **any** reboot
   (power loss, watchdog, crash, manual reset) rolls back to the previous
   firmware. Requires `CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE=y` (§11.2) —
   without it there is no pending-verify state and no passive rollback.
2. **Matter-level** — `ConfirmCurrentImage()` succeeding, followed by
   `mRequestor->NotifyUpdateApplied()`, which clears the persisted `kApplying`
   state and emits the `VersionApplied` event.

In the stock SDK, `DefaultOTARequestorDriver::Init` performs the Matter-level
confirmation automatically and immediately at boot
(`IsFirstImageRun()` → `ConfirmCurrentImage()` → `NotifyUpdateApplied()`).
This design moves the *when* into the application's hands.

### 6.2 The deferral mechanism

The driver is application-owned code on ESP32 (`CustomOTARequestorDriver`).
Its `Init` is changed to **not** auto-confirm: when `IsFirstImageRun()` is
true it records confirmation-pending and returns. The application calls an
explicit gate when — and only when — it is satisfied the device is healthy:

```cpp
CHIP_ERROR CustomOTARequestorDriver::ConfirmUpdate()
{
    VerifyOrReturnError(mConfirmationPending, CHIP_ERROR_INCORRECT_STATE);
    CHIP_ERROR err = mImageProcessor->ConfirmCurrentImage();   // version check
    if (err == CHIP_NO_ERROR)
    {
        esp_ota_mark_app_valid_cancel_rollback();   // bootloader-level commit
        mRequestor->NotifyUpdateApplied();          // Matter-level commit
    }
    else
    {
        mRequestor->Reset();
        // application choice: esp_ota_mark_app_invalid_rollback_and_reboot()
    }
    mConfirmationPending = false;
    return err;
}
```

Typical acceptance gate before calling `ConfirmUpdate()`: network up,
co-processor versions match the expected constants (§6.3), application
self-test passed.

Three rules:

1. **Deferral happens at the driver level, never inside
   `ConfirmCurrentImage()`.** Returning an error from `ConfirmCurrentImage()`
   makes the SDK call `mRequestor->Reset()`, which clears `kApplying` and the
   target version — after that the Matter-level confirmation cannot be
   re-entered for this boot. `ConfirmCurrentImage()` stays a fast, synchronous
   version check.
2. **Crash-before-confirm is the safety net, not a bug.** While confirmation
   is pending, `kApplying` persists and the image is pending-verify; an
   unexpected reboot rolls back cleanly and the old firmware resumes.
3. **Do not dawdle.** The application must bound the pending window: confirm
   within a deadline or trigger a deliberate rollback
   (`esp_ota_mark_app_invalid_rollback_and_reboot()`). A device left
   indefinitely unconfirmed loses the update on any reboot, at an arbitrary
   later time, which is worse than a prompt deliberate decision.

### 6.3 Boot-time consistency check — no persisted state

The framework persists **nothing** across the reboot. Both sides of the
post-update consistency comparison are available without NVS:

- **Expected versions** are compile-time constants in the new application
  firmware. The bundle is built as a set, so the app image carries
  `EXPECTED_<COMPONENT>_VERSION` constants for every co-processor it was
  packaged with. (This is the same pattern as the existing RCP auto-update,
  where the border-router firmware embeds the RCP image it expects.)
- **Actual versions** are ground truth, queried from each component over its
  bus at boot.

On every boot — before Matter initialisation — the application:

1. Queries each external component's installed version (and caches the result;
   `IsReadyForOTA()` decides from this cache, §4.1).
2. Compares against the expected constants.
3. On mismatch: re-flashes the component from a known-good source (a staging
   partition written during the download, or an image embedded in the app
   firmware), or marks the device unhealthy and withholds confirmation /
   Matter advertising until resolved.

This check is the single recovery path for *every* mixed-version cause:
a `kNotReady` skip under Policy A, a write that claimed success but did not
stick, a component that rolled itself back, or a host rollback after a
co-processor was already flashed (§7.3). It does not need to know which cause
occurred — the recovery action is the same. Recomputing from ground truth each
boot is strictly more robust than persisted records, which can go stale or lie.

If the application wants retry limits or backoff for its recovery attempts,
that counter is the application's own NVS entry under its own policy — the
framework neither provides nor requires it.

---

## 7. Threading Model

```
BDX layer (any thread)
   │
   ├─ ProcessBlock() called
   │      │
   │      ├─ DEEP COPY block data into owned buffer   ← buffer safe after return
   │      └─ schedule HandleProcessBlock on Matter thread
   │                          ──────────────────────────►
                                                         Matter thread
                                                           │
                                                           ├─ strip outer Matter OTA header (once)
                                                           ├─ Phase 1: accumulate MultiImageHeader
                                                           ├─ Phase 2: Dispatcher routes blocks
                                                           │
                                                           │  [kReady, Write() returns kDone]
                                                           ├─ SubImageProcessor.Write() ──► kDone
                                                           │     └─ FetchNextData() ─────────────► BDX: next BlockQuery
                                                           │
                                                           │  [kReady, Write() returns kPending]
                                                           ├─ SubImageProcessor.Write() ──► kPending
                                                           │     └─ Dispatcher suspends (no BDX call)
                                                           │
                                                           │               Worker task (any thread)
                                                           │                      │
                                                           │                      ├─ async I/O completes
                                                           │                      └─ OnWriteComplete(result)
                                                           │                                │
                                                           │           ◄───────────────────┘
                                                           ├─ Dispatcher resumes
                                                           │     └─ FetchNextData() ─────────────► BDX: next BlockQuery
                                                           │
                                                           │  [kNotReady or kAlreadyUpToDate]
                                                           ├─ SkipData(distance) ───────────────► BDX: BlockQueryWithSkip
                                                           │     └─ (no FetchNextData — skip IS the query)
                                                           │
                                                           └─ Finalize / OnDownloadOutcome / Apply / Abort run here too
```

**Block buffer lifetime:** The `ByteSpan` passed to `ProcessBlock()` points into
the BDX layer's receive buffer. The BDX layer may reuse that buffer as soon as
`ProcessBlock()` returns. The Main Image Processor must deep-copy the block data
into its own owned buffer before returning. The owned buffer must remain valid
until either `FetchNextData()` is called (sync path) or `OnWriteComplete()` is
received (async path). Only then may the buffer be released.

**`SkipData()` vs `FetchNextData()` are mutually exclusive per block.** Calling
`SkipData()` sends a `BlockQueryWithSkip` BDX message immediately — it is the
next block request. The Provider responds with a new block at the new offset,
which arrives as the next `ProcessBlock()` call. Calling `FetchNextData()` after
`SkipData()` would send a second request while one is already in-flight,
corrupting the BDX stop-and-wait state machine. The Dispatcher calls exactly one
of these per logical step.

All Dispatcher logic and sub-processor `Init()` / `IsReadyForOTA()` / `Write()`
calls execute on the **Matter thread**. Sub-processors must not perform long
blocking operations on the Matter thread — any slow I/O (bus writes, DMA waits)
must be dispatched to a worker task, with `OnWriteComplete()` called back when
done. See §10.3.

**`Finalize()` and `ProcessBlock()` ordering.** The BDX layer calls
`ProcessBlock()` and then immediately calls `Finalize()` (synchronously, in the
same event handler) when the last block carries the EOF flag. Both methods must
return immediately and defer their actual work via `PlatformMgr().ScheduleWork()`
onto the same FIFO Matter thread queue. This is the only mechanism that
guarantees `HandleFinalize` always executes after `HandleProcessBlock` for the
last block. If either method does real work inline instead of deferring, the
ordering guarantee is lost and `Finalize()` may run before the last block's
bytes have been processed.

---

## 8. Atomicity & Failure Modes

### 8.1 Commit boundary

The only commit point the framework controls is the platform-level step inside
`Apply()` that makes the newly downloaded primary firmware active on the next
boot. Until that step executes, a reboot (power loss, crash) leaves the device
on its old firmware — no partial state is visible to the bootloader. Because
the application image is the last entry in the bundle (§3.5), all co-processor
writes have already completed (or the session has already aborted) by the time
this commit point is reached.

Sub-processors receive bytes during the download via `Write()`. Whatever
platform-specific action they take with those bytes (e.g., staging to a
secondary partition, streaming over a bus) is the application's
responsibility.

> **Component firmware rollback is out of scope for this framework.** The
> Multi-Image Processor and the platform layer are responsible only for
> rolling back the **primary ESP32 application firmware** (via the ESP-IDF
> OTA rollback mechanism). Rolling back a co-processor, radio module, or any
> other component after it has been flashed is entirely the **application's
> responsibility**. If a sub-processor's write operation must be reversible,
> the application must implement that rollback logic and invoke it from its
> own `Abort()` handling — the framework will not do it. The boot-time
> consistency check (§6.3) is the application's recovery anchor.

### 8.2 Binary ordering in the OTA file

The order of binaries in the OTA file determines the order in which
sub-processors receive bytes. Co-processor entries come first, ordered by the
application's write dependencies; the application image is always last (§3.5).
The packaging tool produces the file in manifest order and enforces the
app-last rule.

### 8.3 Failure-mode matrix

| Where it fails                                                 | What survives                                                  | Recovery                                                                                    |
| -------------------------------------------------------------- | -------------------------------------------------------------- | ------------------------------------------------------------------------------------------- |
| Mid-download (network, power)                                  | Old firmware in active slot; sub-processor writes partially done | Next OTA cycle overwrites cleanly from byte 0. Boot-time check (§6.3) recovers any component left half-written. |
| `Finalize()` returns error                                     | Old firmware; sub-processor writes may be partially done       | `Abort()` fans out; application cleans up sub-processor state.                              |
| Outcome hook returns `kAbort`                                  | Old firmware; co-processor writes already done stay done       | Deliberate Policy-B drop. Next periodic `QueryImage` (old version still reported) retries the whole bundle. |
| `Apply()` fails                                                | Old firmware on primary; co-processor writes already done      | Split-version state. Boot-time consistency check (§6.3) detects the mismatch and recovers.  |
| Power loss after `Apply()` commits, before reboot              | New firmware will be booted next                               | Normal supported state — bootloader sees the committed slot.                                |
| New firmware crashes on first boot                             | Old firmware (bootloader rollback; requires §11.2 config)      | Boot-time check on the **old** firmware detects any co-processor already flashed to the new version and restores it. |
| New firmware boots but application never calls `ConfirmUpdate()` | Old firmware on next reboot (passive rollback)                | Application must confirm within its deadline or roll back deliberately (§6.2).              |

### 8.4 What "atomic" means here

Truly atomic across the primary firmware and any external component
(co-processor, peripheral) is impossible without a shared transaction log. The
framework provides **best-effort atomicity**:

-   Within the primary firmware slot: real atomicity at the platform commit
    step.
-   Across primary + external components: the app-last ordering rule (§3.5)
    plus the application's boot-time reconciliation (§6.3).

---

## 9. Implementation Requirements

This section describes the requirements a platform implementation must satisfy.
No existing SDK code needs to change — the multi-image OTA processor is a new
`OTAImageProcessorInterface` implementation that uses the existing BDX
downloader and session layer as-is (`OTADownloader::SkipData()` /
`BDXDownloader::SkipData()` already exist).

### 9.1 Platform Layer Requirements

#### R1 — Sub Image Processor interface: four operations

The full contract for each operation is defined in §4.1. The four operations are:

| # | Operation | One-line rule |
|---|-----------|---------------|
| 1 | `Init(entry)` | Light-weight; store entry fields; no heavy I/O. |
| 2 | `IsReadyForOTA()` | Returns `DeviceState` from cached state; must complete in milliseconds. |
| 3 | `Write(block)` | Returns `kDone` (sync) or `kPending` (async); verifies SHA-256 on last chunk. |
| 4 | `Abort(context)` | Discard partial state, cancel async ops, release resources, do not block. |

Asynchronous sub-processors additionally call `OnWriteComplete(result)` on the
Main Image Processor — exactly once per `kPending` return, from a worker task
(§4.1).

The default readiness is `kReady`. Implementations whose component is always
present and always ready need only implement `Write` and `Abort`; the others
have safe defaults.

#### R2 — Dispatcher routing behaviour

The Dispatcher maintains `currentBytePosition`, initialised to
`sizeof(MultiImageHeader)` (the first byte of binary data after the header).

After header parse and before routing, the Dispatcher validates the bundle
rules (§3.5): `numImages ≥ 1`, exactly one entry carries the platform app
image ID, and that entry is last. A violating bundle aborts the session with a
format error.

For each entry in `MultiImageHeader.subImages[]` the Dispatcher must:

1. **Validate** `currentBytePosition == entry.offset`. If not, the bundle is
   malformed — abort the session with an error.
2. **Validate** `entry.length > 0` and `entry.offset + entry.length ≤ payloadSize`
   (where `payloadSize` is from the outer Matter OTA header). Note that
   `entry.length` is a 4-byte field and is passed to `SkipData()`
   (`BDXDownloader::SkipData(uint32_t)`), so per-entry skips are bounded by
   `UINT32_MAX` by construction.
3. **Look up** the registered sub-processor by `imageId`. Treat a missing
   registration as `kAlreadyUpToDate` — the component is assumed to be already
   at its expected version on this device.
4. If a processor is found: call `Init(entry)` (light), then call
   `IsReadyForOTA()` (no params) and record the outcome. If no processor is
   found: record `kSkippedUpToDate`.
5. **If `kNotReady` or `kAlreadyUpToDate`:** compute the skip distance as
   `(entry.offset + entry.length) - currentBytePosition`. This accounts for
   any bytes of this entry already consumed by the current block (split-block
   case). Call `SkipData(skipDistance)`. `SkipData()` sends a
   `BlockQueryWithSkip` BDX message immediately — this is the next block
   request. **Do not call `FetchNextData()` after `SkipData()`**; doing so
   sends a second BDX request while one is already in-flight, corrupting the
   stop-and-wait state machine.
6. **If `kReady`:** feed chunks via `Write()` until `entry.length` bytes have
   been delivered. The Dispatcher owns its routing counter (decides when to
   stop feeding this entry and move to the next); the sub-processor maintains
   its own running total independently (for SHA-256 finalisation and commit
   logic — see §4.1). After each `Write()`:
   - If `Write()` returns `kDone`: call `FetchNextData()` to request the next
     BDX block — **unless this was the last chunk of the last entry**
     (`currentBytePosition + remaining == payloadSize`). After the last
     entry there is no next block to request; the BDX layer has already
     scheduled `Finalize()` and calling `FetchNextData()` at this point would
     send a spurious `BlockQuery` into an EOF session, corrupting the BDX
     state machine. (Because the last entry is the always-written application
     image, the symmetric "skip into EOF" case cannot occur — §3.5.)
   - If `Write()` returns `kPending`: suspend the Dispatcher. Wait for the
     sub-processor to call `OnWriteComplete()`. On success, call
     `FetchNextData()` (same last-entry exception applies). On error, set
     `mLastError` and let the SDK call `Abort()`.
7. **Advance** `currentBytePosition` by `entry.length` after each entry is
   fully delivered or skipped, and **advance `mParams.downloadedBytes` by
   skipped bytes as well as written bytes** — progress reporting
   (`UpdateStateProgress`) is computed against `totalFileBytes`, which includes
   the bytes of skipped entries.

**Split-block rule:** A single BDX block may contain bytes belonging to more
than one entry. When the Dispatcher finishes entry[i]'s bytes mid-block, it
immediately transitions to entry[i+1] using the remaining bytes without waiting
for a new `ProcessBlock()` call. It runs steps 1–6 above for entry[i+1] using
the remaining bytes. If entry[i+1] is `kNotReady` or `kAlreadyUpToDate`, the
skip distance in step 5 correctly resolves to less than `entry[i+1].length`
because `currentBytePosition` already accounts for the bytes consumed from the
current block.

#### R3 — Outcome hook: `OnDownloadOutcome`

The outcome hook is fully specified in §5. The Main Image Processor must expose
`OnDownloadOutcome` as an overridable operation receiving the per-entry outcome
list and returning `kProceed` or `kAbort`. It is invoked exactly once per
successful download, on the Matter thread, after `esp_ota_end()` succeeds and
before the apply phase. The default implementation returns `kProceed`.

#### R4 — Application-owned confirmation in the requestor driver

The platform's requestor driver (`CustomOTARequestorDriver` on ESP32) must not
auto-confirm on first boot. It records confirmation-pending and exposes a
`ConfirmUpdate()` gate per §6.2. `ConfirmCurrentImage()` on the Main Image
Processor remains the synchronous version check it is today.

### 9.2 Compatibility Notes

#### N1 — Single-image processor storage erasure

An existing single-image processor that erases its storage destination at the
start of a download session is not affected by the multi-image design. Each sub-
processor manages its own storage independently; no sub-processor is aware of
what other sub-processors do.

#### N2 — Single-image processor header expectations

An existing single-image processor that expects the outer Matter OTA header at
the start of the byte stream is not compatible with the sub-processor `Write`
contract. The Main Image Processor strips the outer header before any bytes
reach a sub-processor. Sub-processors must not be written to re-parse it.

#### N3 — Encrypted OTA (`CONFIG_ENABLE_ENCRYPTED_OTA`) is incompatible

The pre-encrypted OTA scheme (`esp_encrypted_img`) decrypts the payload as one
sequential stream. `SkipData()` removes ciphertext from the middle of that
stream, so everything after the first skip decrypts to garbage. Multi-image
OTA and whole-payload encrypted OTA are therefore **mutually exclusive at
build time** (§11.1). Per-sub-image encryption (each sub-processor owning its
decryption context) is future work (§16).

#### N4 — Delta OTA (`CONFIG_ENABLE_DELTA_OTA`) is incompatible

The delta patch stream is likewise sequential over the whole payload and is
inherently tied to the application partition. Mutually exclusive at build time
(§11.1). Per-sub-image delta encoding is future work (§16).

#### N5 — The RCP OTA delegate is superseded

The existing `mOtaRcpDelegate` path (`CONFIG_OPENTHREAD_BORDER_ROUTER` +
`CONFIG_AUTO_UPDATE_RCP` in the single-image processor) is an ad-hoc two-image
scheme: an RCP image prepended to the app image and peeled off in
`HandleProcessBlock`. Under multi-image OTA the RCP becomes an ordinary
registered sub-processor with its own `imageId`, staging into the same host
flash partition the delegate uses today. The delegate mechanism is not
compiled when multi-image OTA is enabled; a product migrating to multi-image
OTA repackages its RCP image as a bundle entry.

---

## 10. Adding a Sub Image Processor

This section describes the steps for adding support for an additional binary
(e.g. a co-processor, a peripheral, a secondary partition) to the multi-image
OTA flow.

### 10.1 Pick an image ID

Pick **any value in the range `0x00000100`–`0xFFFFFFFF`** for a manufacturer-defined
binary. There is no list to add to and no central authority.

Document the chosen value in the product's release notes — it is a stable ABI
contract between the device firmware and the packaging tool. Do not reuse a
value across unrelated binaries in the same product, and do not repurpose a
value across firmware releases.

### 10.2 Implement the Sub Image Processor interface

The full interface contract is in §4.1. Implementation notes for each method:

**`Init(entry)`** — store `entry.version`, `entry.length`, and `entry.sha256`.
No heavy I/O; defer to first `Write()`.

**`IsReadyForOTA()`** — decide based on the component's **cached** state
(populated by the boot-time check, §6.3 — never a live bus round-trip here):

| Component state (cached)                               | Return value       | Effect                                                              |
| ------------------------------------------------------ | ------------------ | ------------------------------------------------------------------- |
| Reachable, installed version ≠ `entry.version`         | `kReady`           | Dispatcher delivers chunks via `Write()`.                           |
| Installed version == `entry.version`                   | `kAlreadyUpToDate` | Skipped; outcome `kSkippedUpToDate`.                                |
| Busy (initialising, running a critical task)           | `kNotReady`        | Skipped; outcome `kSkippedNotReady` — application policy decides (§5.2). |
| Unreachable or not responding                          | `kNotReady`        | Skipped; log the reason.                                            |
| Error state requiring manual recovery                  | `kNotReady`        | Skipped; update would be unsafe.                                    |

Must return in milliseconds. The return value is permanent for this OTA cycle.
Log the specific reason so skips are observable and diagnosable.

**`Write(block)`** — the Dispatcher guarantees chunks arrive sequentially from
byte 0, with exactly `entry.length` bytes total, containing raw binary only
(no headers). The buffer remains valid until `FetchNextData()` or
`OnWriteComplete()` fires. Detect the last chunk by comparing the running byte
total against `entry.length`. SHA-256 verification, commit, and bus-transfer
completion must happen on the last chunk. See §10.3 for the async write
pattern. Choose the write strategy deliberately:

- **Staged** — write to a host flash partition now; flash the component at the
  next boot (the RCP pattern). Reversible until the staged image is consumed;
  pairs naturally with the boot-time check.
- **Direct** — stream over the bus to the component during the download.
  Irreversible once started; the component must be recoverable via §6.3 if the
  session aborts mid-stream.

**`Abort(context)`** — discard partial writes, cancel any pending async
operation, release all resources acquired since `Init()`. If the write target
was already irreversibly modified before `Abort()` arrives, the boot-time
consistency check (§6.3) is the recovery path. Must not block.

### 10.3 Non-blocking writes

The Dispatcher and Matter thread cannot stall waiting for slow bus
acknowledgments. If the write operation (UART, SPI, I²C, etc.) takes more than a
few milliseconds per chunk:

1. The chunk arrives in an owned buffer managed by the Main Image Processor.
   This buffer is guaranteed to remain valid for the lifetime of the async
   operation — the BDX layer will not request the next block until
   `OnWriteComplete()` is called.
2. Copy the relevant bytes to a local DMA descriptor or bus transfer buffer.
3. Return `kPending` from `Write()` immediately.
4. Schedule the actual bus transfer to a worker task.
5. When the transfer completes, call `OnWriteComplete(result)` on the Main Image
   Processor handle. This is the only way to resume BDX block fetching after a
   `kPending` return. Do not call `FetchNextData()` directly from the
   sub-processor.

Sub-processors whose writes complete synchronously (e.g. writing to a local
partition, which may internally use platform scheduling but resolves before
returning) return `kDone` from `Write()`. The Dispatcher calls `FetchNextData()`
immediately after. These sub-processors do not use `OnWriteComplete()`.

This pattern applies to any slow transport and keeps the Matter thread
responsive. Mind the BDX idle timeout: per-chunk transfer time (e.g. ~90 ms
for 1 KB at 115200 baud) must stay well under the Provider's status-report
timeout.

### 10.4 Register the processor

Register the sub-processor instance with the Dispatcher **before
`InitOTARequestor()`** — i.e., during application initialisation, before the
Matter OTA Requestor is initialised — associating it with the chosen image ID.

Registration only builds the `imageId → sub-processor` map. It does **not**
determine processing order. Processing order is determined entirely by the order
of `MultiImageHeader.subImages[]` entries in the OTA file, which is set at packaging time
(§10.5). A processor registered first can have an image ID that appears last in
the file.

Each image ID may have at most one registered processor. Registering a duplicate
image ID is an error.

### 10.5 Wire up packaging

Add the binary and its image ID to the product's release manifest and ensure the
CI build passes both to the packaging tool. The order of entries in the manifest
determines the order of binaries in the OTA file, which determines the delivery
order. Place co-processor binaries in the order that matches your application's
write dependencies; the application image is always last (§3.5 — the tool
enforces it).

Also add the component's new version to the application firmware's
expected-version constants (§6.3) in the same release — the bundle and the
constants must always move together.

### 10.6 Boot-time consistency check

For any binary that targets an external component (co-processor, peripheral),
implement the boot-time check of §6.3 before the application initializes
Matter:

1. Query the component's currently installed version (cache the result for
   `IsReadyForOTA()`).
2. Compare it against the expected-version constant compiled into the running
   firmware.
3. If they do not match, either re-trigger the update from a known-good source
   (a staging partition or a blob bundled in the primary firmware) or mark the
   device as unhealthy and withhold Matter advertising / confirmation until
   the inconsistency is resolved.

This is the recovery path for every mixed-version cause: a Policy-A skip, a
silently failed write, a component-side rollback, or a host rollback after the
component was already flashed. **This check is a hard requirement, not a
suggestion, for any product that registers a co-processor sub-processor.**

---

## 11. Build Configuration

### 11.1 Build flag

Multi-image OTA is disabled by default. A single flag enables the entire
subsystem, following the same pattern as the existing optional OTA features in
`config/esp32/components/chip/Kconfig`:

```
config ENABLE_MULTI_IMAGE_OTA
    bool "Enable multi-image OTA"
    depends on ENABLE_OTA_REQUESTOR
    depends on !ENABLE_ENCRYPTED_OTA && !ENABLE_DELTA_OTA
    default n
```

When disabled, the build compiles the existing single-image
`OTAImageProcessorImpl` and **nothing in the current implementation changes**.
When enabled, the multi-image processor sources under
`src/platform/ESP32/multi-ota/` are compiled instead, and the application's
OTA initialisation registers the multi-image processor with the downloader.
The two implementations are mutually exclusive — exactly one
`OTAImageProcessorInterface` implementation is linked.

The `depends on !ENABLE_ENCRYPTED_OTA && !ENABLE_DELTA_OTA` exclusions are
required, not stylistic — see §9.2 N3/N4. The Kconfig flag maps to a GN
argument for the chip component build, gating the source list in
`src/platform/ESP32/BUILD.gn` the same way the existing flags do.

### 11.2 Platform requirements

The target platform must have:

-   At least one alternate firmware slot so the new firmware can be written
    without overwriting the currently running image.
-   **`CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE=y`.** The application-owned
    confirmation model (§6) depends on the pending-verify state: an
    unconfirmed image must roll back on the next reboot. Without this config
    there is no passive rollback and the safety story in §6.2/§8.3 does not
    hold.

Specific platform configuration (partition tables, bootloader flags, etc.) is
documented alongside the platform integration code.

---

## 12. Versioning

Three distinct version numbers exist in this system:

| Number                              | Where it lives                                | Who reads it                                   | Semantics                                                                                                         |
| ----------------------------------- | --------------------------------------------- | ---------------------------------------------- | ----------------------------------------------------------------------------------------------------------------- |
| Matter `softwareVersion`            | Outer OTA header & `BasicInformation` cluster | OTA Requestor / Provider                       | "Is there a newer bundle?" — represents the state where every component in the bundle is at its expected version. |
| Per-binary `SubImageHeader.version` | `MultiImageHeader.subImages[]` entries        | Sub-processors (`IsReadyForOTA()`), outcome hook | Expected installed version of each individual component for this bundle.                                          |
| Component-reported version          | Read from the component at boot (cached)      | Boot-time consistency check, `IsReadyForOTA()` | Ground truth of what is actually installed on that component right now.                                           |

### 12.1 Discipline

-   The outer `softwareVersion` strictly increases on every release and is
    bumped whenever any component in the bundle changes.
-   **Every bundle contains a rebuilt application image** compiled with the
    new `softwareVersion` and with expected-version constants matching every
    co-processor binary in the same bundle (§3.5, §6.3) — even when the
    application code itself is functionally unchanged.
-   The per-component `SubImageHeader.version` is the authoritative expected
    version for that component in this bundle and must equal the corresponding
    constant compiled into the bundle's application image.
-   The component-reported runtime version is the ground truth. The boot-time
    check compares runtime versions against the constants of the running
    firmware, not against any previous bundle.
-   Never release a bundle where any component's `SubImageHeader.version` is
    lower than what is already deployed — this would cause sub-processors
    reporting "already at target version" to incorrectly pass verification
    against a downgraded expectation.

### 12.2 Security note

`SubImageHeader.sha256` provides **integrity, not authenticity**. The
application image is additionally verified by the bootloader (secure-boot
signature, when enabled) at `esp_ota_end()` / first boot — co-processor
binaries receive no such platform check. Products whose co-processors are
security-relevant should embed a vendor signature inside the component binary
itself and verify it in the sub-processor (on the last chunk, alongside the
SHA-256) or in the component's own bootloader.

---

## 13. Testing

### 13.1 Unit tests

Feed synthesized byte streams into the Dispatcher and verify routing behaviour
using stub sub-processors. Tests should cover:

**Header and bundle validation:**

-   `MultiImageHeader` parsing: valid magic, invalid magic rejected,
    `numImages = 0` rejected.
-   Bundle rules (§3.5): missing app-image entry rejected; app-image entry not
    last rejected; duplicate app-image entries rejected.
-   `entry.length = 0`: rejected at parse time.
-   Bundle with `entry.offset` that does not match `currentBytePosition`:
    session aborted immediately with a format error.

**Routing:**

-   Boundary cases: block ends exactly at a binary boundary.
-   Block split across two binaries: last bytes of binary A and first bytes of
    binary B arrive in the same block.
-   `IsReadyForOTA()` returning `kNotReady`: verify `Init()` is called once,
    `SkipData()` is called with the correct relative skip distance (not blindly
    `entry.length` — must account for `currentBytePosition`), `Write()` is never
    called, and `FetchNextData()` is **not** called after `SkipData()`.
-   `IsReadyForOTA()` returning `kAlreadyUpToDate`: same assertions; outcome
    recorded as `kSkippedUpToDate`.
-   No registered processor for an image ID: verify `SkipData()` is called with
    the correct skip distance and the entry is recorded as `kSkippedUpToDate`
    (not `kSkippedNotReady`).
-   `Init()` receives the correct `SubImageHeader` fields before the first
    `Write()` call.
-   `currentBytePosition` tracking across a split block: verify that when a
    block spans two entries, the skip distance for entry[i+1] equals
    `entry[i+1].length` minus the bytes of entry[i+1] already consumed from
    the current block.
-   Progress accounting: `downloadedBytes` advances across skipped entries so
    that it equals `totalFileBytes` at `Finalize()` regardless of skips.
-   SHA-256 digest mismatch on the last chunk: verify the sub-processor rejects
    the update and the session aborts.

**Sync/async write paths:**

-   `Write()` returns `kDone`: verify `FetchNextData()` is called immediately
    after, before processing any further entries.
-   `Write()` returns `kDone` on the last chunk of the last entry: verify
    `FetchNextData()` is **not** called — sending a `BlockQuery` into an EOF
    session would corrupt the BDX state machine.
-   `Write()` returns `kPending`: verify `FetchNextData()` is **not** called
    until `OnWriteComplete(success)` fires from the worker task; verify the
    owned block buffer remains valid throughout.
-   `Write()` returns `kPending`, then `OnWriteComplete(error)`: verify
    `mLastError` is set, `FetchNextData()` is never called, and `Abort()` is
    invoked on all `Init()`-ed sub-processors with `reason = kError`.

**Outcome hook and apply policy (§5):**

-   All entries written → hook invoked once with all `kWritten`; default
    returns `kProceed`; `Apply()` proceeds.
-   One entry `kSkippedNotReady` → hook receives it; with the default
    (`kProceed`), `Apply()` proceeds (Policy A).
-   Hook overridden to return `kAbort` on `kSkippedNotReady` → verify
    `Abort()` fans out with `reason = kCancelled`, `esp_ota_abort()` is called,
    no `Apply()`, and the requestor returns to idle (Policy B).
-   Unregistered image ID appears in the hook as `kSkippedUpToDate`.
-   Hook is **not** invoked when the session aborts on error.
-   Session failure after the hook would have fired is impossible by
    construction (hook fires only after `esp_ota_end()` succeeds) — assert
    ordering.

**Confirmation (§6):**

-   Driver does not auto-confirm: after a simulated first boot
    (`IsFirstImageRun() == true`), verify neither
    `esp_ota_mark_app_valid_cancel_rollback()` nor `NotifyUpdateApplied()` is
    called until the application calls `ConfirmUpdate()`.
-   `ConfirmUpdate()` with matching version: verify the order — version check,
    bootloader mark-valid, `NotifyUpdateApplied()`.
-   `ConfirmUpdate()` with mismatching version: verify `Reset()` is called and
    mark-valid is **not**.
-   `Apply()` failure: verify the device does not reboot and stays on the old
    firmware.

### 13.2 Integration tests

A hardware- or emulator-based test that:

1. Builds a primary firmware image and a stub secondary image.
2. Packages them into a two-component `.ota` bundle (secondary first, app
   last).
3. Serves the bundle via a Matter OTA Provider.
4. Runs the device under test, confirms it downloads, applies, reboots, runs
   the boot-time consistency check, confirms via the application gate, and
   reports the updated version.
5. Repeats with the secondary's stub sub-processor returning `kNotReady`,
   under both Policy A (verify app updates, hook reports the skip) and
   Policy B (verify nothing is applied and the next query retries the
   bundle).
6. Crash-before-confirm: reboot the device after `Apply()` but before
   `ConfirmUpdate()`; verify bootloader rollback restores the old firmware.

### 13.3 Failure injection

Inject failures at: mid-write (return error from `Write()` /
`OnWriteComplete(error)`), `Finalize()`, `Apply()`. For each case, verify the
device remains on the previously running firmware after the failure and that
`Abort()` reached every `Init()`-ed sub-processor.

---

## 14. End-to-End Example — RCP + Light co-processors over UART

A host ESP32 (the Matter node) with two UART-attached co-processors: an
802.15.4 RCP and a "Light" MCU with its own UART bootloader.

```
┌─────────────────────────────┐
│  ESP32 host (Matter node)   │
│  app fw in OTA_0/OTA_1      │
│  rcp_fw staging partition   │
└───────┬──────────┬──────────┘
   UART0│     UART1│
┌───────┴───┐ ┌────┴──────────┐
│ RCP       │ │ Light MCU     │
└───────────┘ └───────────────┘
```

| imageId  | Component | Write strategy                                  | Write mode |
|----------|-----------|--------------------------------------------------|------------|
| `0x0100` | RCP       | staged: host `rcp_fw` partition, flashed at boot | sync → `kDone` |
| `0x0101` | Light     | direct: streamed over UART1 to its bootloader    | async → `kPending` |
| app ID   | App fw    | `esp_ota_write` to inactive OTA partition        | sync → `kDone` |

**Packaging.** Manifest order: RCP, Light, App (app last — §3.5). The app
binary is compiled with `softwareVersion = 10300` and constants
`EXPECTED_RCP_VERSION = 103`, `EXPECTED_LIGHT_VERSION = 27`.

**Boot (old firmware 10200).** App registers both sub-processors, runs the
boot-time check — RCP reports 102, Light reports 26, both match the *old*
firmware's constants — and caches the versions. Matter starts.

**Download.** `QueryImage` (reporting 10200) → update to 10300 offered →
`PrepareDownload` (`esp_ota_begin`, session state reset) → BDX blocks arrive.
The outer Matter header is stripped, the `MultiImageHeader` accumulated and
validated, routing begins.

- *RCP entry:* cached 102 ≠ 103 → `kReady`. Each chunk written to the
  `rcp_fw` staging partition, `kDone`, `FetchNextData()`. Last chunk: SHA-256
  verified, staged-image trailer written. The RCP itself is untouched.
- *Light entry:* cached 26 ≠ 27 → `kReady`. First `Write()` commands the MCU
  into bootloader mode; each chunk is handed to a worker task and `kPending`
  returned; the worker streams it over UART1, waits for the ACK, and calls
  `OnWriteComplete(success)`, which resumes the Dispatcher on the Matter
  thread. Last chunk: SHA-256 verified, bootloader commit command sent.
  (Had the Light been unreachable at boot, `IsReadyForOTA()` would return
  `kNotReady` and the Dispatcher would issue one `BlockQueryWithSkip` past the
  remainder of the entry.)
- *App entry:* standard `esp_ota_write` per chunk; on the final chunk no
  `FetchNextData()` — the EOF block has already queued `Finalize()`.

**Finalize → outcome → apply.** `esp_ota_end()` validates the app image.
`OnDownloadOutcome({rcp: kWritten, light: kWritten, app: kWritten})` →
`kProceed`. Requestor apply flow runs; `esp_ota_set_boot_partition()`; reboot.

**First boot of 10300.** Image is pending-verify. The boot-time check sees the
staged RCP image (103) newer than the running RCP (102) → flashes the RCP over
UART0 from the staging partition → RCP reports 103 ✓. Light reports 27 ✓.
Matter starts; the driver notes `IsFirstImageRun()` and waits. The application
gate passes (versions match, network up, self-test OK) and calls
`ConfirmUpdate()`: version check ✓ → `esp_ota_mark_app_valid_cancel_rollback()`
→ `NotifyUpdateApplied()`. Update complete — no state was persisted at any
point.

**If it had gone wrong:**

- Light skipped (`kNotReady`) under Policy A: device ends up on 10300 with
  Light at 26; the boot-time check flags it; the app recovers from its
  embedded Light image or accepts staleness until the next bundle.
- Under Policy B: the hook returns `kAbort`; nothing applied; the bundle is
  retried whole on the next query.
- New firmware crashes before `ConfirmUpdate()`: bootloader rolls back to
  10200. If the RCP was already flashed to 103, the old firmware's boot-time
  check detects 103 ≠ 102 and restores the RCP from its old staged image —
  this is why §10.6 is a hard requirement.

---

## 15. Responsibilities

### 15.1 This framework provides

-   The **Main Image Processor** implementing the standard OTA image processor
    interface — the single entry point the Matter OTA Requestor calls.
-   The **Dispatcher** — header parsing (`MultiImageHeader` including its
    `subImages[]`), bundle-rule validation (§3.5), the `imageId →
    sub-processor` map, byte-level routing, `SkipData()` for absent and
    not-ready entries, and the in-RAM outcome table.
-   The **Sub Image Processor interface** — the four-method contract (§4.1)
    plus the `OnWriteComplete()` resume path for async writes.
-   The **outcome hook** `OnDownloadOutcome()` (§5).
-   The **`MultiImageHeader` binary format** (preamble + embedded `SubImageHeader`
    array) and the packaging tool that produces `.ota` files conforming to it.
-   A default sub-processor for the primary application firmware slot,
    registered automatically at startup for the platform-defined application
    image ID, wrapping today's `esp_ota_*` flow unchanged.
-   This document.

### 15.2 Platform layer provides

-   The mechanism to write the primary application firmware to its designated
    storage location and make it the active boot target on `Apply()`.
-   The requestor driver with deferred, application-gated confirmation
    (§6.2, R4).
-   The platform-level `Abort()` implementation that cleans up any partially
    written primary firmware (`esp_ota_abort`).

### 15.3 Application provides

For a primary-firmware-only setup: **nothing beyond enabling multi-image OTA
and calling `ConfirmUpdate()`** (or wiring it to a trivial always-confirm
policy). The default sub-processor and packaging manifest cover the rest.

For a multi-component product, the application provides:

1. **One Sub Image Processor implementation per additional binary.** See §10.
2. **Registration** — registers each sub-processor under its chosen image ID
   before `InitOTARequestor()`.
3. **An apply policy** — Policy A, Policy B, or a per-component mix, expressed
   in the `OnDownloadOutcome()` override (§5.2). Default is Policy A.
4. **The confirmation gate** — when to call `ConfirmUpdate()`, what health
   checks gate it, and the deadline/rollback decision (§6.2).
5. **The boot-time consistency check** with expected-version constants and a
   recovery source per external component (§6.3, §10.6). Hard requirement.
6. **Packaging integration** — CI calls the packaging tool with a manifest that
   lists all components and their image IDs, app image last.
7. **A versioning policy** per §12.1.

### 15.4 OTA Provider / build tooling provides

-   A reachable Matter OTA Provider that serves the `.ota` file and supports
    `BlockQueryWithSkip` (the SDK reference Provider does; see §16.4).
-   The packaging tool invoked as part of release CI.
-   Operational practice: never publish a bundle with a lower `softwareVersion`
    than the currently deployed one.

---

## 16. Open Questions / Future Work

1. **Resume on disconnect.** BDX retries restart from byte 0; there is no
   per-binary progress checkpoint. If the device reboots mid-download it
   re-downloads from the beginning. Acceptable for most products.

2. **`numImages` upper bound.** No maximum below 255 is defined. The in-memory
   buffer required to accumulate the `MultiImageHeader` during parsing is
   `8 + numImages × 48` bytes (up to ~12 KB for 255 entries). Devices with
   constrained RAM should define a platform-specific limit and reject bundles
   that exceed it.

3. **Per-sub-image encryption and delta.** Whole-payload encrypted OTA and
   delta OTA are build-time incompatible with skipping (§9.2 N3/N4). A future
   revision can move both inside the sub-processor boundary: each entry
   carries its own encrypted or delta-encoded blob and the owning
   sub-processor decrypts/patches in `Write()`. Nothing in the wire format
   prevents this.

4. **Provider compliance for `BlockQueryWithSkip`.** A minimal or
   non-compliant Provider may reject skip messages (the SDK reference Provider
   handles them). There is currently no negotiation mechanism. When no entry
   is skipped — the common case of a fully-applicable bundle — no skip message
   is ever sent, so the feature degrades gracefully. A future extension could
   add a read-and-discard fallback for Providers that reject skips.

---

## Appendix A. Design Rationale — SkipData Approach

#### Pros

| #   | Benefit                                                                                                                                                           |
| --- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| P1  | **Single BDX session.** No multi-session orchestration, no changes to the `DefaultOTARequestor` state machine.                                                    |
| P2  | **Spec-compliant.** `BlockQueryWithSkip` (0x15) is a standard BDX message. Any compliant Provider handles it.                                                     |
| P3  | **RAM stays constant.** All sub-processors share one block buffer. Registered sub-processors are just pointers in a map; no extra RAM per component.              |
| P4  | **Stateless cycles.** Nothing is persisted between OTA cycles; a `kNotReady` skip simply means the next cycle (or the application's boot-time recovery) gets another attempt, with no stale state to manage. |
| P5  | **No session layer changes.** The existing BDX downloader and session layer are used as-is (`SkipData()` already exists). The multi-image processor is a new `OTAImageProcessorInterface` implementation only. |
| P6  | **Existing single-image path untouched.** The subsystem is gated behind `ENABLE_MULTI_IMAGE_OTA`; when disabled, the current implementation builds unchanged.     |

#### Cons

| #   | Limitation                                                                                                                                                                                                                                                |
| --- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| C1  | **Skip = no update this cycle.** `IsReadyForOTA()` is a yes/no decision before bytes start flowing. There is no "wait and retry." If a component needs 10 seconds to enter update mode it misses this OTA cycle. Pre-session synchronization is required. |
| C2  | **Processing order locked to file layout.** Components are processed in the order they appear in the OTA file. The packaging tool must produce the file in the order that matches the application's write dependencies (app image always last).           |
| C3  | **No mid-session pause.** The Dispatcher cannot pause the BDX session and resume later. Provider idle timeout terminates the session if `BlockQuery` or `BlockQueryWithSkip` is not sent promptly.                                                        |
| C4  | **Mixed-version state on skip (Policy A).** A device where one component was skipped runs a new version of some components alongside old versions of others, and OTA cannot fix it until the next bundle (the device already reports the new `softwareVersion`). The application's boot-time check (§6.3) is the mandatory recovery path. Policy B avoids this at the cost of delaying the whole bundle. |
| C5  | **Provider must support `BlockQueryWithSkip`** whenever entries can be skipped. Most compliant Matter OTA Providers handle it; verify before relying on skip (§16.4).                                                                                      |
| C6  | **Incompatible with whole-payload encrypted/delta OTA.** Sequential-stream transforms cannot survive skips; build-time mutual exclusion (§11.1), per-sub-image variants are future work.                                                                  |
