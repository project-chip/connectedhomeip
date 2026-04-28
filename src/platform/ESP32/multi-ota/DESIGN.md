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

---

## 2. Glossary

| Term                    | Definition                                                                                                                                          |
| ----------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Bundle**              | A single `.ota` file containing a `MultiImageHeader` (which embeds one `SubImageHeader` per component in its `subImages[]` field) followed by all component binaries concatenated. |
| **Main Image Processor**| The OTA image processor implementation the Matter OTA Requestor calls. Owns the full download lifecycle and contains the Dispatcher.                |
| **Dispatcher**          | The routing engine inside the Main Image Processor. Parses the bundle header, maintains the `imageId → sub-processor` map, and routes or skips each component's bytes. |
| **Sub Image Processor** | An application-provided handler for one component. Implements `Init(entry)`, `IsReadyForOTA()`, `Write()`, `OnWriteComplete(result)`, `Confirm(expectedVersion)`, and `Abort(context)`. |
| **Image ID**            | A 4-byte unsigned integer (uint32, `0x00000000`–`0xFFFFFFFF`) that identifies which sub-processor handles a given component. Stable ABI between firmware and packaging tool. |
| **`SkipData(n)`**       | BDX `BlockQueryWithSkip` — tells the Provider to advance its cursor by `n` bytes without delivering them.                                          |
| **Confirmation**        | The platform action that marks newly booted firmware as valid and cancels any pending rollback. Triggered by `ConfirmCurrentImage()`.               |
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
| 4      | 1    | `numImages` | Number of entries in `subImages[]` (0–255)           |
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
| `imageId`  | Matched against registered processors at runtime. A bundle may carry `imageId`s not registered on a given device — those entries are skipped via `SkipData()` and assumed to be already up to date (do not block confirmation). |
| `version`  | Per-binary version number. Informational and for sanity checks only; the OTA Requestor's update decision is driven by the outer Matter `softwareVersion` alone.             |
| `offset`   | Absolute byte offset from payload start. Enables the dispatcher to jump directly to each binary with a single `SkipData()` call rather than streaming all preceding bytes. |
| `length`   | Exact byte count. Used both to know when to stop feeding the processor and to compute the skip distance for unneeded binaries.                                              |
| `sha256`  | Mandatory SHA-256 of the binary at `[offset, offset+length)`. The sub-processor must verify this digest before accepting the update as complete. |

### 3.4 Image ID space

`imageId` is a 4-byte unsigned integer (uint32, values `0x00000000`–`0xFFFFFFFF`).
The namespace is partitioned to avoid collisions:

| Range                        | Owner        | Use                                                                                                                                    |
| ---------------------------- | ------------ | -------------------------------------------------------------------------------------------------------------------------------------- |
| `0x00000000`                 | —            | Invalid. Rejected at parse time.                                                                                                       |
| `0x00000001 .. 0x000000FF`   | Platform     | Reserved for platform-defined well-known images. Specific assignments are an implementation detail and not fixed by this spec.         |
| `0x00000100 .. 0xFFFFFFFF`   | Manufacturer | **Manufacturer-defined.** Pick any value in this range; no central registry.                                                           |

The specific well-known IDs within range `0x01`–`0xFF` (e.g. which value means
"application firmware", etc.) are determined by the implementation and documented
alongside the processor registration code. This spec does not assign them.

Manufacturer-defined IDs in range `0x00000100`–`0xFFFFFFFF` carry no inherent
meaning beyond what the device firmware and packaging tool agree on. There is no
list to add to and no central authority.

The only runtime validation is **duplicate registration** —
`RegisterProcessor(imageId, ...)` fails if `imageId` is already registered. The
`imageId`-to-meaning mapping is a stable ABI contract between the device
firmware and the packaging tool. Repurposing an ID across firmware versions is
an ABI break for that product.

### 3.5 Packaging tool

A packaging tool takes a manifest that describes each binary — its image ID,
version, and file path — and produces a single `.ota` file. The tool:

1. Reads each binary and computes its SHA-256 digest.
2. Computes each binary's `offset` (header section size + sum of preceding
   binary sizes).
3. Builds the `MultiImageHeader` (preamble + `subImages[]`).
4. Concatenates the header blob and binaries, then wraps the result with the
   standard outer Matter OTA header (vendor ID, product ID, software version,
   digest).

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
contains the Dispatcher.

**Dispatcher** — the routing engine inside the Main Image Processor. It parses
the `MultiImageHeader` (preamble then `subImages[]`) from the incoming byte
stream, maintains the `imageId → sub-processor` map populated by application
registration, and redirects each byte chunk to the sub-processor that matches
the current `subImages[i].imageId`. When no sub-processor is registered for an
entry, the Dispatcher treats it as `kAlreadyUpToDate` (assumed up to date) and
calls `SkipData()` to advance past it. When the matched sub-processor returns
not-ready, `SkipData()` is also called but the entry blocks confirmation.

**Sub Image Processor** — an interface the application registers per image ID.
Six methods form the contract:

-   `Init(entry)` — called once with the full `SubImageHeader` for each entry,
    before `IsReadyForOTA()`. Must be light-weight: store the fields needed for
    the readiness decision (`entry.version`) and for subsequent writes
    (`entry.length`, `entry.sha256`). Do not start heavy I/O here — defer that
    to the first `Write()` call so resources are only allocated when `kReady` is
    confirmed.
-   `IsReadyForOTA()` → `DeviceReadinessState` — called immediately after
    `Init()`, with no parameters. The sub-processor uses the version stored
    during `Init()` to decide: `kReady` (proceed), `kAlreadyUpToDate` (already
    at target version, skip but counts as verified), `kNotReady` (skip, blocks
    confirmation). Must return quickly — no blocking I/O.
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

-   `OnWriteComplete(result)` — called by the sub-processor (not by the
    framework) when an asynchronous write operation finishes. This is the
    mechanism by which a sub-processor that returned `kPending` from `Write()`
    unblocks the Dispatcher. The Dispatcher's `OnWriteComplete()` handler
    **must post its continuation back to the Matter thread** (via
    `PlatformMgr().ScheduleWork()`) before returning — it must not call
    `FetchNextData()` or any BDX operation directly from the worker task
    context. `FetchNextData()` and `SkipData()` drive the BDX state machine
    and are only safe to call from the Matter event loop thread. Once resumed
    on the Matter thread: on success, call `FetchNextData()` (or skip as
    applicable); on error, set `mLastError` and let the SDK call `Abort()`.
    Sub-processors whose `Write()` always returns `kDone` never call
    `OnWriteComplete()`.

    **`OnWriteComplete()` must be called exactly once per `kPending` `Write()`
    call, from a task context (not from within the `Write()` call itself).** The
    Dispatcher holds a reference to the copied block buffer until the Matter
    thread resumes after `OnWriteComplete()`; only then is the buffer released
    and the next BDX block requested.

-   `Confirm(expectedVersion)` — called on every sub-processor that had `Init()`
    called during the download, at `ConfirmCurrentImage()` time (§5.1). The
    framework passes the persisted expected version. The sub-processor compares
    the component's current installed version against `expectedVersion` and
    returns success or error. Does not require `Init()` to have been called
    first — all information needed is in `expectedVersion`.
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
    async operation, and release all resources acquired since `Init()`. Must not
    block.

`DeviceReadinessState` is a three-value enum — not a boolean — because the
confirmation policy (§12.1) requires distinguishing "already up to date" from
"unavailable":

| Value              | Dispatcher action                      | Counts as verified?                             |
| ------------------ | -------------------------------------- | ----------------------------------------------- |
| `kReady`           | Call `Write()` per chunk               | Yes                                             |
| `kAlreadyUpToDate` | `SkipData(entry.length)`               | Yes — already at target version                 |
| `kNotReady`        | `SkipData(entry.length)`               | **No** — blocks `softwareVersion` confirmation  |

**Skipped means permanently skipped for this OTA cycle.** The Provider will not
re-send those bytes. A component whose processor returns `kNotReady` is not
updated this cycle and blocks `softwareVersion` confirmation. A component
returning `kAlreadyUpToDate` is also skipped but counts as verified — it does
not block confirmation. Think of `kNotReady` as "opt out of this update," not
"defer until later in this session."

**`IsReadyForOTA()` must return immediately.** In ReceiverDrive mode the
Provider waits for the next `BlockQuery` or `BlockQueryWithSkip`. If the
Requestor goes silent for too long the Provider will terminate the session with
a status-report timeout. The readiness check must complete in milliseconds. Any
initialization that takes longer (bus negotiation, boot-mode handshake) must be
done before the OTA session begins.

**Mixed-version state after a `kNotReady` skip.** If one binary is updated and
another is skipped with `kNotReady`, the device ends the OTA cycle with
mismatched versions across its components. The application is responsible for
detecting and handling this (§7.4, §10.6). Do not skip a component unless the
device can tolerate running the old version of that component alongside new
versions of the others.

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
  |   +-------------------------------------------+  |
  |   |              Dispatcher                   |  |
  |   |   MultiImageHeader (incl. subImages[])    |  |
  |   |   imageId → sub-processor map             |  |
  |   |   routes chunks / calls SkipData()        |  |
  |   +--------+----------------------------------+  |
  +------------+-------------------------------------+
               |  framework → sub-processor
               | Init(SubImageHeader) — always, light
               | IsReadyForOTA() → DeviceReadinessState
               | Write(block) → kDone | kPending
               | Confirm(expectedVersion) — first-boot per-entry check
               | Abort(AbortContext) — on error or cancel
               |
               |  sub-processor → framework (async only)
               | OnWriteComplete(result) ────────────────►
               v
  +------------+-------------------------------------+
  |      Sub Image Processor (interface)             |
  |   Init(SubImageHeader)              (app-impl)  |
  |   IsReadyForOTA()                   (app-impl)  |
  |   Write(block)  → kDone|kPending    (app-impl)  |
  |   OnWriteComplete(result) — calls framework      |
  |   Confirm(expectedVersion) → result  (app-impl)  |
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
  `mLastError`, resets `currentBytePosition` to zero, resets the header-parse
  state machine, and prepares for a fresh download. Any NVS entries from a
  previous `softwareVersion` are erased here if the incoming version differs.
- `ProcessBlock()` — receives every BDX block. Immediately deep-copies the
  block data into an owned buffer, schedules `HandleProcessBlock()` on the
  Matter thread, and returns. Tracks `mLastError` and `currentBytePosition`.
  The normative routing algorithm is in §9.1 R2.
- `Finalize()` / `Apply()` / `Abort()` — all must return immediately and defer
  real work via `ScheduleWork`. See §6 for the ordering guarantee between
  `Finalize()` and the last `ProcessBlock()`.

**Dispatcher** — validates each entry's offset, looks up its sub-processor,
calls `Init(entry)` and `IsReadyForOTA()`. For `kNotReady`/`kAlreadyUpToDate`
calls `SkipData()` — which is the next BDX block request; never follow it with
`FetchNextData()`. For `kReady` calls `Write()`, then either `FetchNextData()`
(`kDone`) or suspends until the sub-processor calls `OnWriteComplete()` (`kPending`).
A block may span two entries; skip distances are derived from `currentBytePosition`.
Full routing rules: §9.1 R2.

**Sub Image Processor** — receives `Init()`, `IsReadyForOTA()`, then `Write()`
per chunk if `kReady`. Full interface contract: §4.1.

**Registration** — applications call a register function before the OTA session
starts, associating an `imageId` with a sub-processor instance. Each `imageId`
may have at most one registered processor; duplicate registration is an error.

---

## 5. Confirmation and Retry

### 5.1 Boot validation and `softwareVersion` confirmation

1. On the next boot the platform starts the newly committed firmware. If the
   firmware fails to start (crash loop, hard fault before confirming), the
   platform's rollback mechanism restores the previously running firmware.
2. On the first successful boot of the new firmware, the OTA Requestor calls
   `IsFirstImageRun()` — returns true. The Requestor then calls
   `ConfirmCurrentImage()`.
3. The multi-image `ConfirmCurrentImage()` performs **two checks** before
   signalling the platform to mark the firmware as confirmed:
   a. The running `softwareVersion` matches the version the Requestor
      downloaded.
   b. For each entry that had a registered processor with `Init()` called, the
      framework looks up the sub-processor by the persisted `imageId` and calls
      `Confirm(expectedVersion)` with the persisted version:
      - Component is at `expectedVersion` → `Confirm()` returns success.
      - Component is unavailable → `Confirm()` does a one-time opportunistic
        check; if it still cannot be reached, returns an error.
      - Component is at a different version (update failed or was skipped)
        → `Confirm()` returns an error.
      - No processor registered for an entry (recorded as `kAlreadyUpToDate`
        by the Dispatcher) → no `Confirm()` call; the entry is counted as
        verified automatically.
      If any sub-processor's `Confirm()` returns an error, confirmation is
      withheld.
4. If either check fails, `ConfirmCurrentImage()` returns an error. The SDK
   calls `mRequestor->Reset()`, which clears the `kApplying` state from
   persistent storage. **The SDK does not trigger a reboot.** The device
   continues running the new firmware, but that firmware remains unconfirmed by
   the platform bootloader — meaning a rollback will occur on the next reboot
   for any reason (power loss, watchdog, manual reset). Before returning the
   error, the Main Image Processor invokes the retry policy hook (§5.2) so the
   platform can schedule a new `QueryImage`. This new OTA session runs on the
   still-booted new firmware: sub-images that are already at their target
   version return `kAlreadyUpToDate` and are skipped; only the missing
   sub-image is downloaded and written. If that session succeeds,
   `ConfirmCurrentImage()` passes, the bootloader confirms the firmware, and
   the mixed-version state is resolved without a rollback.

   If retries are exhausted (see §5.2), the platform must trigger a deliberate
   reboot so the bootloader rolls back to the previously confirmed firmware.
   Leaving the device running indefinitely on unconfirmed firmware is not safe.

**`ConfirmCurrentImage()` is called exactly once per firmware boot.** The
`kApplying` state that causes `IsFirstImageRun()` to return true is cleared by
`mRequestor->Reset()` when confirmation fails — subsequent boots of the same
firmware do not re-enter the confirmation flow. Recovery after a failed
confirmation is always through a new OTA cycle, not a repeated confirmation
attempt.

The Main Image Processor persists one thing in NVS across the reboot (keyed by
the `softwareVersion` being applied):

- For every entry whose registered processor had `Init()` called: the `imageId`
  and the expected `version` (8 bytes per entry). On the next boot,
  `ConfirmCurrentImage()` looks up each sub-processor by `imageId` and calls
  `Confirm(expectedVersion)` directly — no `Init()` or `IsReadyForOTA()` call
  is needed post-reboot. The other `SubImageHeader` fields (`offset`, `length`,
  `sha256`) are download-phase-only and are not persisted.

Once `ConfirmCurrentImage()` succeeds — meaning all components are verified
and the new `softwareVersion` is committed — the Main Image Processor **must
erase the persisted `{imageId, version}` records and `attemptCount`**. This ensures
the next OTA cycle (for `softwareVersion + 1` or any future bundle) starts with
a clean slate and does not inherit stale entries from the previous cycle.

### 5.2 Retry policy for incomplete cycles

When a download session finishes with one or more `kNotReady` entries, the
affected nodes were not updated this cycle. Without intervention the device will
next attempt an update only when the periodic `QueryImage` timer fires — which
can be many minutes away. If `softwareVersion` is confirmed prematurely it may
never fire at all (see §12.1).

To recover faster, the Main Image Processor can actively re-trigger `QueryImage`
after a platform-chosen delay, without waiting for the periodic timer. The retry
policy — when to retry, how many times, and with what backoff — is entirely the
platform's responsibility and is expressed via an overridable operation on the
Main Image Processor.

#### Retry hook

After any download session ends with at least one `kNotReady` entry, or after
`ConfirmCurrentImage()` withholds confirmation, the Main Image Processor invokes
the `OnPendingNodeUpdates` operation, passing two arguments:

- **Pending image IDs** — the list of image IDs whose sub-processors returned
  `kNotReady` this cycle. `kAlreadyUpToDate` entries are not included; they are
  already verified and do not drive retries.
- **Attempt count** — the number of times this operation has been invoked for
  the current `softwareVersion`. The Main Image Processor increments this counter
  and persists it in NVS, keyed by `softwareVersion`, so that post-rollback
  retries do not reset it. When the Main Image Processor begins a new OTA session
  for a different `softwareVersion`, it erases all NVS entries from the previous
  version (`{imageId, version}` records and attempt counter) before proceeding,
  ensuring no stale state carries over between bundles.

The platform implementation decides what to do:

| Decision                       | Behaviour                                                                                              |
| ------------------------------ | ------------------------------------------------------------------------------------------------------ |
| Re-trigger immediately         | Schedule a `QueryImage` with zero delay (via `ScheduleWork` or a zero-interval timer) and return. Must not call `QueryImage` synchronously from within the hook — the OTA state machine has not yet unwound to `kIdle` at hook-call time. |
| Re-trigger after a fixed delay | Schedule a `QueryImage` to fire after a platform-chosen interval.                                      |
| Exponential backoff            | Increase the delay geometrically with attempt count; e.g. `30s × 2^attemptCount`, capped at a maximum.|
| Retry up to N times then stop  | Compare attempt count against a platform-defined limit; stop scheduling and raise an alert when exceeded.|
| Do nothing                     | Return without action — normal periodic `QueryImage` takes over.                                       |

The default behaviour is to do nothing. Platforms that require fast convergence
for connected nodes must override this operation.

**All implementations must schedule the `QueryImage` asynchronously** — never
call it synchronously from within `OnPendingNodeUpdates`. When the hook is
called, the OTA state machine has not yet returned to `kIdle`: on the
`ConfirmCurrentImage()` path the state is still `kApplying`; on the download
path the BDX session teardown has not fully completed. A synchronous
`QueryImage` call at this point would hit the wrong state and either be
silently rejected or corrupt the state machine. Scheduling via `ScheduleWork`
or a zero-interval timer ensures the call fires only after the current event
has fully unwound and the requestor has transitioned back to `kIdle`.

#### When the hook is called

| Event | Hook called? | Device state at hook time | Notes |
| ----- | ------------ | ------------------------- | ----- |
| Download completes; all entries `kReady` or `kAlreadyUpToDate` | No | — | Cycle was complete — no retry needed. |
| Download completes; one or more entries `kNotReady` | **Yes** — before Apply | Old firmware still running | Platform may schedule a retry after the upcoming reboot. |
| `ConfirmCurrentImage()` withholds confirmation | **Yes** — before returning error | **New firmware running, unconfirmed** | Platform should schedule a new `QueryImage` immediately. The retry session runs on the new firmware; only the missing sub-image is downloaded. If the retry succeeds, the bootloader confirms the firmware and no rollback occurs. |

When `ConfirmCurrentImage()` withholds confirmation, the new firmware is still
running but is unconfirmed by the bootloader. A rollback will occur on the next
reboot for any reason. The retry hook is the opportunity to fix this by
scheduling an immediate `QueryImage` and completing the missing sub-image update
before any reboot happens.

#### Limits and failure cases

- **Permanent unavailability.** If a node is decommissioned or has a hardware
  fault, `IsReadyForOTA()` will always return `kNotReady`. The platform must
  implement a max-retry limit. When the limit is reached, the platform **must
  trigger a deliberate reboot** so the bootloader rolls back to the previously
  confirmed firmware. Leaving the device running indefinitely on unconfirmed
  firmware is unsafe. The attempt count is the natural signal for detecting this.
- **Node recovers between cycles.** If `IsReadyForOTA()` returned `kNotReady`
  but the node recovers before the retry fires, the next OTA cycle will pick it
  up cleanly — no special handling needed.
- **No impact on the BDX session in progress.** The retry hook runs after the
  current BDX session has fully ended. It never interrupts or restarts a live
  download.

---

## 6. Threading Model

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
                                                           └─ Finalize / Apply / Abort run here too
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

## 7. Atomicity & Failure Modes

### 7.1 Commit boundary

The only commit point the framework controls is the platform-level step inside
`Apply()` that makes the newly downloaded primary firmware active on the next
boot. Until that step executes, a reboot (power loss, crash) leaves the device
on its old firmware — no partial state is visible to the bootloader.

Sub-processors receive bytes during the download via `Write()`. Whatever
platform-specific action they take with those bytes (e.g., staging to a
secondary partition, buffering for a bus transfer) is the application's
responsibility.

> **Component firmware rollback is out of scope for this framework.** The
> Multi-Image Processor and the platform layer are responsible only for
> rolling back the **primary ESP32 application firmware** (via the ESP-IDF
> OTA rollback mechanism). Rolling back a co-processor, radio module, or any
> other component after it has been flashed is entirely the **application's
> responsibility**. If a sub-processor's write operation must be reversible,
> the application must implement that rollback logic and invoke it from its
> own `Abort()` handling — the framework will not do it.

### 7.2 Binary ordering in the OTA file

The order of binaries in the OTA file determines the order in which
sub-processors receive bytes. If the application's write logic for one binary
must complete before another begins (e.g., a co-processor must be flashed before
the host firmware is staged), the packaging tool must produce the file in that
order. See §3.5.

### 7.3 Failure-mode matrix

| Where it fails                                                 | What survives                                                  | Recovery                                                                                    |
| -------------------------------------------------------------- | -------------------------------------------------------------- | ------------------------------------------------------------------------------------------- |
| Mid-download (network, power)                                  | Old firmware in active slot; new data partially written        | Next OTA cycle overwrites cleanly from byte 0.                                              |
| `Finalize()` returns error                                     | Old firmware; sub-processor writes may be partially done       | `Abort()` is called on the Main Image Processor; application cleans up sub-processor state. |
| `Apply()` fails                                                | Old firmware on primary; sub-processor writes may be partially done | Component rollback is the application's responsibility (out of scope for this framework). Next OTA cycle re-flashes as needed. |
| `Apply()` fails; sub-processor write was irreversible (e.g. co-processor already flashed over bus) | Old primary firmware; external component already on new firmware | Split-version state. Boot-time consistency check (§10.6) detects mismatch and re-triggers update or marks device unhealthy. |
| Power loss after `Apply()` commits, before reboot              | New firmware will be booted next                               | Normal supported state — bootloader sees the committed slot.                                |
| New firmware crashes on first boot                             | Old firmware                                                   | Platform rollback restores previous slot before `ConfirmCurrentImage()` is called.          |
| New firmware boots but `ConfirmCurrentImage()` is never called | Old firmware on next reboot                                    | Platform rolls back. Application must confirm.                                              |

### 7.4 What "atomic" means here

Truly atomic across the primary firmware and any external component
(co-processor, peripheral) is impossible without a shared transaction log. The
framework provides **best-effort atomicity**:

-   Within the primary firmware slot: real atomicity at the platform commit
    step.
-   Across primary + external components: depends on application ordering logic
    and boot-time reconciliation (see §10.6).

---

## 8. Responsibilities

### 8.1 This framework provides

-   The **Main Image Processor** implementing the standard OTA image processor
    interface — the single entry point the Matter OTA Requestor calls.
-   The **Dispatcher** — header parsing (`MultiImageHeader` including its
    `subImages[]`), the `imageId → sub-processor` map, byte-level routing,
    and `SkipData()` for absent entries (assumed up to date) and not-ready entries.
-   The **Sub Image Processor interface** — the contract (`IsReadyForOTA()` +
    write method) that applications implement per image ID.
-   The **`MultiImageHeader` binary format** (preamble + embedded `SubImageHeader`
    array) and the packaging tool that produces `.ota` files conforming to it.
-   A default sub-processor for the primary application firmware slot,
    registered automatically at startup for the platform-defined application
    image ID.
-   This document.

### 8.2 Platform layer provides

-   The mechanism to write the primary application firmware to its designated
    storage location and make it the active boot target on `Apply()`.
-   Boot validation: detecting a first run of new firmware, confirming it, and
    rolling back if it does not confirm.
-   The platform-level `Abort()` implementation that cleans up any partially
    written primary firmware.

### 8.3 Application provides

For a primary-firmware-only setup: **nothing beyond enabling multi-image OTA**.
The default sub-processor and packaging manifest cover it.

For a multi-component product, the application provides:

1. **One Sub Image Processor implementation per additional binary.** Implements
   `IsReadyForOTA()` and the write method for that binary's destination
   (partition, bus, NVS, etc.). See §10.
2. **Registration** — registers each sub-processor under its chosen image ID
   before the OTA session begins.
3. **Packaging integration** — CI calls the packaging tool with a manifest that
   lists all components and their image IDs.
4. **A boot-time consistency check** for any external component (co-processor,
   peripheral). On boot, query the component's installed version and re-trigger
   update if it does not match the expected version for the running firmware.
   See §10.6.
5. **A versioning policy** — define when the outer `softwareVersion` is bumped
   (recommended: bump on any component change).

### 8.4 OTA Provider / build tooling provides

-   A reachable Matter OTA Provider that serves the `.ota` file.
-   The packaging tool invoked as part of release CI.
-   Operational practice: never publish a bundle with a lower `softwareVersion`
    than the currently deployed one.

---

## 9. Implementation Requirements

This section describes the requirements a platform implementation must satisfy.
No existing SDK code needs to change — the multi-image OTA processor is a new
`OTAImageProcessorInterface` implementation that uses the existing BDX
downloader and session layer as-is.

### 9.1 Platform Layer Requirements

#### R1 — Sub Image Processor interface: six operations

The full contract for each operation is defined in §4.1. The six operations are:

| # | Operation | One-line rule |
|---|-----------|---------------|
| 1 | `Init(entry)` | Light-weight; store entry fields; no heavy I/O. |
| 2 | `IsReadyForOTA()` | Returns `DeviceReadinessState`; must complete in milliseconds. |
| 3 | `Write(block)` | Returns `kDone` (sync) or `kPending` (async); verifies SHA-256 on last chunk. |
| 4 | `OnWriteComplete(result)` | Called by sub-processor from worker task; exactly once per `kPending` return. |
| 5 | `Confirm(expectedVersion)` | First-boot check that component is at `expectedVersion`; no prior `Init()` required. |
| 6 | `Abort(context)` | Discard partial state, cancel async ops, release resources, do not block. |

The default readiness is `kReady`. Implementations whose component is always
present and always ready need only implement `Write`, `Confirm`, and `Abort`;
the others have safe defaults.

#### R2 — Dispatcher routing behaviour

The Dispatcher maintains `currentBytePosition`, initialised to
`sizeof(MultiImageHeader)` (the first byte of binary data after the header).

For each entry in `MultiImageHeader.subImages[]` the Dispatcher must:

1. **Validate** `currentBytePosition == entry.offset`. If not, the bundle is
   malformed — abort the session with an error.
2. **Validate** `entry.length > 0` and `entry.offset + entry.length ≤ payloadSize`
   (where `payloadSize` is from the outer Matter OTA header). Note that
   `entry.length` is a 4-byte field and is passed to `SkipData()` which accepts
   a maximum of `UINT32_MAX` bytes per call. Entries that would require a skip
   larger than this are rejected.
3. **Look up** the registered sub-processor by `imageId`. Treat a missing
   registration as `kAlreadyUpToDate` — the component is assumed to be already
   at its expected version on this device.
4. If a processor is found: call `Init(entry)` (light), then call
   `IsReadyForOTA()` (no params) and record the result. If no processor is
   found: record `kAlreadyUpToDate`.
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
   logic — see §4.1). After
   each `Write()`:
   - If `Write()` returns `kDone`: call `FetchNextData()` to request the next
     BDX block — **unless this was the last chunk of the last entry**
     (`currentBytePosition + entry.length == payloadSize`). After the last
     entry there is no next block to request; the BDX layer has already
     scheduled `Finalize()` and calling `FetchNextData()` at this point would
     send a spurious `BlockQuery` into an EOF session, corrupting the BDX
     state machine.
   - If `Write()` returns `kPending`: suspend the Dispatcher. Wait for the
     sub-processor to call `OnWriteComplete()`. On success, call
     `FetchNextData()` (same last-entry exception applies). On error, set
     `mLastError` and let the SDK call `Abort()`.
7. **Advance** `currentBytePosition` by `entry.length` after each entry is
   fully delivered or skipped.

**Split-block rule:** A single BDX block may contain bytes belonging to more
than one entry. When the Dispatcher finishes entry[i]'s bytes mid-block, it
immediately transitions to entry[i+1] using the remaining bytes without waiting
for a new `ProcessBlock()` call. It runs steps 1–6 above for entry[i+1] using
the remaining bytes. If entry[i+1] is `kNotReady` or `kAlreadyUpToDate`, the
skip distance in step 5 correctly resolves to less than `entry[i+1].length`
because `currentBytePosition` already accounts for the bytes consumed from the
current block.

#### R3 — Retry hook: `OnPendingNodeUpdates`

The retry hook is fully specified in §5.2. The Main Image Processor must expose
`OnPendingNodeUpdates` as an overridable operation receiving the list of pending
image IDs and the persisted `attemptCount`. The default behaviour is to do
nothing; platforms override to implement scheduling, backoff, and retry limits.

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

**`IsReadyForOTA()`** — decide based on the component's current state:

| Component state                                        | Return value       | Effect                                                              |
| ------------------------------------------------------ | ------------------ | ------------------------------------------------------------------- |
| Reachable, installed version ≠ `entry.version`         | `kReady`           | Dispatcher delivers chunks via `Write()`.                           |
| Installed version == `entry.version`                   | `kAlreadyUpToDate` | Skipped. Counts as verified — no update needed.                     |
| Busy (initialising, running a critical task)           | `kNotReady`        | Skipped. Blocks `softwareVersion` confirmation this cycle.          |
| Unreachable or not responding                          | `kNotReady`        | Skipped. Blocks confirmation. Log the reason.                       |
| Error state requiring manual recovery                  | `kNotReady`        | Skipped. Blocks confirmation. Update would be unsafe.               |

Must return in milliseconds. The return value is permanent for this OTA cycle.
Log the specific reason so skips are observable and diagnosable.

**Write method** — the Dispatcher guarantees chunks arrive sequentially from
byte 0, with exactly `entry.length` bytes total, containing raw binary only
(no headers). The buffer remains valid until `FetchNextData()` or
`OnWriteComplete()` fires. Detect the last chunk by comparing running byte total
against `entry.length`. SHA-256 verification, commit, and bus-transfer
completion must happen on the last chunk. See §10.3 for async write pattern.

**`Confirm(expectedVersion)`** — called by the framework post-reboot with the
persisted expected version. No prior `Init()` call is made. The sub-processor
queries the component's current installed version and compares it against
`expectedVersion`:

| Component state at `Confirm()` time | Action |
| ------------------------------------ | ------ |
| Installed version == `expectedVersion` | Return success. |
| Component unreachable / unavailable | One-time opportunistic check; return error if still unreachable. |
| Installed version ≠ `expectedVersion` | Return error (update failed or was skipped). |

**`Abort(context)`** — discard partial writes, cancel any pending async
operation, release all resources acquired since `Init()`. If the write target
was already flashed before `Abort()` arrives, record the inconsistency for the
boot-time consistency check (§10.6). Must not block.

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
responsive.

### 10.4 Register the processor

Register the sub-processor instance with the Dispatcher before the OTA session
starts, associating it with the chosen image ID. Registration is typically done
at application initialization.

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
order. Place binaries in the order that matches your application's write
dependencies.

### 10.6 Boot-time consistency check

For any binary that targets an external component (co-processor, peripheral),
implement a boot-time check before the application initializes Matter:

1. Query the component's currently installed version.
2. Compare it against the version expected by the running firmware.
3. If they do not match, either re-trigger the update from a known-good source
   (e.g., a blob bundled in the primary firmware) or mark the device as
   unhealthy and withhold Matter advertising until the inconsistency is
   resolved.

This is the recovery path for the case where the primary firmware was committed
but the external component's bytes were skipped or its write failed silently.

---

## 11. Build Configuration

### 11.1 Build flag

Multi-image OTA is disabled by default. A single build flag enables the entire
subsystem:

```
chip_enable_multi_ota_requestor = true
```

When disabled, the build uses the standard single-image processor. The two paths
are mutually exclusive — at most one OTA image processor implementation can be
linked.

### 11.2 Platform requirements

The target platform must have:

-   At least one alternate firmware slot so the new firmware can be written
    without overwriting the currently running image.
-   Boot validation support: the ability to mark a new firmware as confirmed
    after a successful first boot, and to roll back to the previous firmware if
    confirmation does not arrive.

Specific platform configuration (partition tables, bootloader flags, etc.) is
documented alongside the platform integration code.

---

## 12. Versioning

Three distinct version numbers exist in this system:

| Number                              | Where it lives                                | Who reads it                                   | Semantics                                                                                                         |
| ----------------------------------- | --------------------------------------------- | ---------------------------------------------- | ----------------------------------------------------------------------------------------------------------------- |
| Matter `softwareVersion`            | Outer OTA header & `BasicInformation` cluster | OTA Requestor / Provider                       | "Is there a newer bundle?" — represents the state where every component in the bundle is at its expected version. |
| Per-binary `SubImageHeader.version` | `MultiImageHeader.subImages[]` entries        | Sub-processors, boot-time check                | Expected installed version of each individual component for this bundle.                                          |
| Component-reported version          | Read from the component at runtime            | Boot-time consistency check, `IsReadyForOTA()` | Ground truth of what is actually installed on that component right now.                                           |

### 12.1 Discipline

-   The outer `softwareVersion` strictly increases on every release and is
    bumped whenever any component in the bundle changes.
-   The per-component `SubImageHeader.version` is the authoritative expected
    version for that component in this bundle. Use it as the target in the
    boot-time verification check.
-   The component-reported runtime version is the ground truth. The boot-time
    check compares runtime versions against the expected table for the running
    bundle, not against the previous bundle.
-   Never release a bundle where any component's `SubImageHeader.version` is
    lower than what is already deployed — this would cause sub-processors
    reporting "already at target version" to incorrectly pass verification
    against a downgraded expectation.

---

## 13. Testing

### 13.1 Unit tests

Feed synthesized byte streams into the Dispatcher and verify routing behaviour
using stub sub-processors. Tests should cover:

-   `MultiImageHeader` parsing: valid magic/version, invalid magic rejected,
    `numImages` = 0 handled gracefully.
-   Boundary cases: block ends exactly at a binary boundary.
-   Block split across two binaries: last bytes of binary A and first bytes of
    binary B arrive in the same block.
-   `IsReadyForOTA()` returning `kNotReady`: verify `Init()` is called once,
    `SkipData()` is called with the correct relative skip distance (not blindly
    `entry.length` — must account for `currentBytePosition`), `Write()` is never
    called, and `FetchNextData()` is **not** called after `SkipData()`.
-   `IsReadyForOTA()` returning `kAlreadyUpToDate`: verify `Init()` is called
    once, `SkipData()` is called with the correct relative skip distance,
    `Write()` is never called, and `FetchNextData()` is **not** called after
    `SkipData()`.
-   No registered processor for an image ID: verify `SkipData()` is called with
    the correct skip distance, the entry is recorded as `kAlreadyUpToDate` (not
    `kNotReady`), and confirmation is **not** blocked.
-   `Init()` receives the correct `SubImageHeader` fields before the first
    `Write()` call.
-   SHA-256 digest mismatch on the last chunk: verify the sub-processor rejects
    the update and `Finalize()` returns an error.
-   `Apply()` failure: verify the device does not reboot and stays on the old
    firmware.
-   `Write()` returns `kDone`: verify `FetchNextData()` is called immediately
    after, before processing any further entries.
-   `Write()` returns `kDone` on the last chunk of the last entry
    (`currentBytePosition + entry.length == payloadSize`): verify
    `FetchNextData()` is **not** called — sending a `BlockQuery` into an EOF
    session would corrupt the BDX state machine.
-   `Write()` returns `kPending`: verify `FetchNextData()` is **not** called
    until `OnWriteComplete(success)` fires from the worker task; verify the
    owned block buffer remains valid throughout.
-   `Write()` returns `kPending`, then `OnWriteComplete(error)`: verify
    `mLastError` is set, `FetchNextData()` is never called, and `Abort()` is
    invoked on all sub-processors with `reason = kError`.
-   Bundle with `entry.offset` that does not match `currentBytePosition`:
    verify the session is aborted immediately with a format error.
-   `entry.length = 0`: verify the entry is rejected at parse time.
-   `currentBytePosition` tracking across a split block: verify that when a
    block spans two entries, the skip distance for entry[i+1] equals
    `entry[i+1].length` minus the bytes of entry[i+1] already consumed from
    the current block.

**Confirmation policy tests** — these are the most critical class:

-   All sub-processors return `kReady` and all writes succeed →
    `ConfirmCurrentImage()` succeeds and `softwareVersion` is confirmed.
-   All sub-processors return `kAlreadyUpToDate` →
    `ConfirmCurrentImage()` succeeds (every entry counts as verified).
-   One sub-processor returns `kNotReady`, all others `kReady` →
    `ConfirmCurrentImage()` returns error; the SDK clears `kApplying` state
    but does **not** trigger a reboot; the device continues running on
    unconfirmed firmware; `softwareVersion` is **not** confirmed; rollback
    occurs passively on the next reboot; `OnPendingNodeUpdates` is invoked.
-   Mix of `kReady` + `kAlreadyUpToDate` with no `kNotReady` →
    `ConfirmCurrentImage()` succeeds.
-   `kNotReady` entry recorded during download; on the next boot the component
    has recovered → verify `ConfirmCurrentImage()` calls `Confirm(expectedVersion)`
    using the persisted `{imageId, version}` record, the component now reports
    the correct version, and confirmation succeeds.

**Retry policy tests:**

-   Download ends with one `kNotReady` entry → verify `OnPendingNodeUpdates()`
    is called with that image ID in `pendingImageIds` and `attemptCount = 1`.
-   `OnPendingNodeUpdates` schedules a re-trigger; verify the re-trigger fires
    after the configured delay and a new BDX session begins.
-   `attemptCount` increments correctly across rollback reboots; verify NVS
    persistence by simulating power-cycle between attempts.
-   Platform sets `kMaxRetries = 3`; verify `OnPendingNodeUpdates()` stops
    scheduling retries after the third call and raises an alert instead.
-   `kAlreadyUpToDate` entry is **not** included in `pendingImageIds`; verify
    it does not trigger a retry.

### 13.2 Integration tests

A hardware- or emulator-based test that:

1. Builds a primary firmware image and a stub secondary image.
2. Packages them into a two-component `.ota` bundle.
3. Serves the bundle via a Matter OTA Provider.
4. Runs the device under test, confirms it downloads, applies, reboots, confirms
   the new firmware, and reports the updated version.

### 13.3 Failure injection

Inject failures at: mid-write (return error from the write method),
`Finalize()`, `Apply()`. For each case, verify the device remains on the
previously running firmware after the failure.

---

## 14. Open Questions / Future Work

1. **Resume on disconnect.** BDX retries restart from byte 0; there is no
   per-binary progress checkpoint. If the device reboots mid-download it
   re-downloads from the beginning. Acceptable for most products.

2. **`numImages = 0` validity.** The format allows `numImages = 0`, producing an
   8-byte preamble with no sub-images. The spec currently accepts this (the
   Dispatcher would switch immediately to routing mode with nothing to route).
   Whether a zero-image bundle should be rejected at parse time or silently
   allowed is unresolved. A future revision should make this explicit.

3. **`numImages` upper bound.** No maximum is defined. The NVS footprint for
   persisted `{imageId, version}` records is 8 bytes × `numImages` — 2,040
   bytes for 255 entries — which is manageable. However, the in-memory buffer
   required to accumulate the `MultiImageHeader` during parsing is
   `8 + numImages × 48` bytes (up to ~12 KB for 255 entries). Devices with
   constrained RAM should define a platform-specific limit and reject bundles
   that exceed it.

4. **Delta / compressed binaries.** All binaries are assumed to be raw flat
   images. Delta encoding or compression would reduce transfer size but requires
   per-component decompression in `Write()`. Out of scope for this revision;
   nothing in the format prevents a future `imageId` namespace assignment for
   compressed variants.

5. **Provider compliance for `BlockQueryWithSkip`.** C5 in Appendix A notes that
   a non-compliant Provider may reject skip messages. There is currently no
   negotiation mechanism. A future extension could add a capability bit in the
   bundle header so that Providers can indicate skip support, allowing the
   Dispatcher to fall back to read-and-discard if skip is unsupported.

---

## Appendix A. Design Rationale — SkipData Approach

#### Pros

| #   | Benefit                                                                                                                                                           |
| --- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| P1  | **Single BDX session.** No multi-session orchestration, no changes to the `DefaultOTARequestor` state machine.                                                    |
| P2  | **Spec-compliant.** `BlockQueryWithSkip` (0x15) is a standard BDX message. Any compliant Provider handles it.                                                     |
| P3  | **RAM stays constant.** All sub-processors share one block buffer. Registered sub-processors are just pointers in a map; no extra RAM per component.              |
| P4  | **Independent retry semantics.** If `IsReadyForOTA()` returns `kNotReady` today, the next OTA cycle offers another attempt with no persistent state to manage.    |
| P5  | **No session layer changes.** The existing BDX downloader and session layer are used as-is. The multi-image processor is a new `OTAImageProcessorInterface` implementation only. |
| P6  | **Backward compatible.** Default `IsReadyForOTA()` returns `kReady`, so existing single-image processors are unaffected.                                          |

#### Cons

| #   | Limitation                                                                                                                                                                                                                                                |
| --- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| C1  | **Skip = no update this cycle.** `IsReadyForOTA()` is a yes/no decision before bytes start flowing. There is no "wait and retry." If a component needs 10 seconds to enter update mode it misses this OTA cycle. Pre-session synchronization is required. |
| C2  | **Processing order locked to file layout.** Components are processed in the order they appear in the OTA file. The packaging tool must produce the file in the order that matches the application's write dependencies.                                   |
| C3  | **No mid-session pause.** The Dispatcher cannot pause the BDX session and resume later. Provider idle timeout terminates the session if `BlockQuery` or `BlockQueryWithSkip` is not sent promptly.                                                        |
| C4  | **Mixed-version state on skip.** A device where one component was skipped runs a new version of some components alongside old versions of others. The application must detect and handle this at boot time (see §10.6).                                    |
| C5  | **Provider must support `BlockQueryWithSkip`.** Most compliant Matter OTA Providers handle it, but a minimal or non-compliant Provider may not. Verify Provider compliance before relying on skip.                                                        |
