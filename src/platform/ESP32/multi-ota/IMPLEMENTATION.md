# Multi-Image OTA — Implementation Preview

> Class sketches derived from DESIGN.md. No production code yet.

---

## File layout

```
src/platform/ESP32/multi-ota/
    MultiImageHeader.h          — wire format structs (§3.2, §3.3)
    OTAReadiness.h              — enum shared by all files
    SubImageProcessor.h         — abstract interface (§4.1)
    MultiImageOTAProcessor.h/cpp — Main Image Processor + Dispatcher (§4.1)
    AppImageProcessor.h/cpp     — built-in sub-processor for primary firmware
```

---

## 1. Wire format — `MultiImageHeader.h`

```cpp
#pragma once
#include <stdint.h>

#define MULTI_IMAGE_HEADER_MAGIC 0x4D494F54u  // "MIOT"

// Fixed 8 bytes at the start of every multi-image OTA payload.
struct __attribute__((packed)) MultiImageHeader
{
    uint32_t magic;        // must equal MULTI_IMAGE_HEADER_MAGIC
    uint8_t  numImages;    // number of SubImageHeader entries (max 255)
    uint8_t  reserved[3]; // must be zero
};
static_assert(sizeof(MultiImageHeader) == 8);

// Fixed 48 bytes per component entry.
struct __attribute__((packed)) SubImageHeader
{
    uint8_t  imageId;      // identifies which sub-processor handles this binary
    uint32_t version;      // expected installed version of this binary
    uint32_t offset;       // byte offset of binary data from payload start
    uint32_t length;       // exact byte count of the binary
    uint8_t  sha256[32];   // mandatory SHA-256 digest of [offset, offset+length)
    uint8_t  reserved[3];  // must be zero; reserved for future extensions
};
static_assert(sizeof(SubImageHeader) == 48);

// Safe field read from a packed/potentially-unaligned pointer.
// Always use this instead of direct field access on SubImageHeader*.
template <typename T>
inline T ReadField(const void * src)
{
    T val;
    memcpy(&val, src, sizeof(T));
    return val;
}
```

---

## 2. Readiness enum — `OTAReadiness.h`

```cpp
#pragma once

enum class OTAReadiness : uint8_t
{
    kReady,           // proceed — Dispatcher calls Init() then Write()
    kAlreadyUpToDate, // skip — component already at targetVersion; counts as verified
    kNotReady,        // skip — component unavailable; blocks softwareVersion confirmation
};
```

---

## 3. Sub Image Processor interface — `SubImageProcessor.h`

```cpp
#pragma once
#include "MultiImageHeader.h"
#include "OTAReadiness.h"
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>

namespace chip {
namespace ESP32 {
namespace MultiOTA {

class SubImageProcessor
{
public:
    virtual ~SubImageProcessor() = default;

    // Called once before any bytes are delivered.
    // targetVersion == SubImageHeader.version for this entry.
    // Must return within milliseconds — no blocking I/O.
    virtual OTAReadiness IsReadyForOTA(uint32_t targetVersion) = 0;

    // Called once with the full SubImageHeader immediately before the
    // first Write(), only when IsReadyForOTA() returned kReady.
    // Store entry.length to self-track progress; store entry.sha256
    // to verify integrity on the last chunk.
    virtual CHIP_ERROR Init(const SubImageHeader & entry) = 0;

    // Called per chunk. Chunks arrive in order; exactly entry.length
    // bytes total across all calls. Raw binary bytes only — no headers.
    // On the last chunk (running total == entry.length set in Init):
    //   verify SHA-256, commit, finalize transport.
    virtual CHIP_ERROR Write(ByteSpan block) = 0;

    // Called if the OTA session is aborted after Init() was called.
    // Component firmware rollback is the application's responsibility.
    virtual void Abort() {}
};

} // namespace MultiOTA
} // namespace ESP32
} // namespace chip
```

---

## 4. Main Image Processor — `MultiImageOTAProcessor.h`

```cpp
#pragma once
#include "MultiImageHeader.h"
#include "OTAReadiness.h"
#include "SubImageProcessor.h"

#include <app/clusters/ota-requestor/OTADownloader.h>
#include <include/platform/OTAImageProcessor.h>
#include <lib/core/OTAImageHeader.h>
#include <map>
#include <vector>

namespace chip {
namespace ESP32 {
namespace MultiOTA {

class MultiImageOTAProcessor : public OTAImageProcessorInterface
{
public:
    // ── OTAImageProcessorInterface ──────────────────────────────────────────
    CHIP_ERROR PrepareDownload() override;
    CHIP_ERROR Finalize() override;
    CHIP_ERROR Apply() override;
    CHIP_ERROR Abort() override;
    CHIP_ERROR ProcessBlock(ByteSpan & block) override;
    bool       IsFirstImageRun() override;
    CHIP_ERROR ConfirmCurrentImage() override;

    void SetOTADownloader(OTADownloader * downloader) { mDownloader = downloader; }

    // ── Registration ────────────────────────────────────────────────────────
    // Register a sub-processor for imageId. Must be called at application
    // initialisation, before any BDX session can start. Calling after a
    // session has begun is undefined behaviour.
    // Each imageId may have at most one processor; duplicate = error.
    CHIP_ERROR RegisterProcessor(uint8_t imageId, SubImageProcessor * processor);

    // ── Retry hook (R3) — override for custom retry policy ─────────────────
    // Called after any cycle that ends with kNotReady entries, or when
    // ConfirmCurrentImage() withholds confirmation.
    // pendingIds: image IDs whose processors returned kNotReady this cycle.
    // attemptCount: persisted across reboots for this softwareVersion.
    // Default: do nothing (fall back to periodic QueryImage timer).
    virtual void OnPendingNodeUpdates(const std::vector<uint8_t> & pendingIds,
                                      uint32_t attemptCount) {}

    static MultiImageOTAProcessor & GetDefaultInstance();

private:
    // ── Matter-thread handlers ───────────────────────────────────────────────
    static void HandlePrepareDownload(intptr_t context);
    static void HandleFinalize(intptr_t context);
    static void HandleApply(intptr_t context);
    static void HandleAbort(intptr_t context);
    static void HandleProcessBlock(intptr_t context);

    // ── Block processing pipeline ────────────────────────────────────────────
    CHIP_ERROR ProcessHeader(ByteSpan & block);   // strips outer Matter OTA header
    CHIP_ERROR ProcessPayload(ByteSpan & block);  // Phase 1 + Phase 2 dispatch

    // Phase 1: accumulate MultiImageHeader + SubImageHeader[] before routing
    CHIP_ERROR AccumulateHeader(ByteSpan & block);

    // Phase 2: route bytes to sub-processors
    CHIP_ERROR RoutePayload(ByteSpan & block);

    void FetchNextData();
    void SkipData(uint64_t numBytes);  // issues BlockQueryWithSkip

    // ── Confirmation helpers ─────────────────────────────────────────────────
    CHIP_ERROR LoadReadinessMapFromNVS();
    CHIP_ERROR SaveReadinessMapToNVS();
    void       EraseNVSState();         // called on successful confirmation
    bool       AllComponentsVerified() const;

    // ── Block buffer ─────────────────────────────────────────────────────────
    CHIP_ERROR SetBlock(ByteSpan & block);
    void       ReleaseBlock();

    // ── State ────────────────────────────────────────────────────────────────
    OTADownloader *    mDownloader    = nullptr;
    OTAImageHeaderParser mHeaderParser;
    MutableByteSpan    mBlock;

    // Header accumulation (Phase 1)
    bool               mHeaderParsed  = false;
    uint8_t *          mHeaderBuf     = nullptr; // heap-allocated; freed after parse
    uint32_t           mHeaderNeeded  = sizeof(MultiImageHeader);
    uint32_t           mAccumulated   = 0;

    // Parsed header
    uint8_t            mNumImages     = 0;
    SubImageHeader     mSubImages[255];

    // Routing state (Phase 2)
    uint8_t            mCurrentIndex  = 0;
    bool               mEntryStarted  = false;
    uint32_t           mBytesDelivered       = 0;
    uint32_t           mCurrentStreamOffset  = 0;

    // Per-entry readiness recorded during download; persisted in NVS
    std::map<uint8_t, OTAReadiness> mReadinessMap;
    uint32_t           mAttemptCount  = 0;

    // Processor registry
    std::map<uint8_t, SubImageProcessor *> mProcessorMap;
};

} // namespace MultiOTA
} // namespace ESP32
} // namespace chip
```

---

## 5. Built-in application firmware sub-processor — `AppImageProcessor.h`

```cpp
#pragma once
#include "SubImageProcessor.h"
#include "esp_ota_ops.h"

namespace chip {
namespace ESP32 {
namespace MultiOTA {

// Handles the primary ESP32 application firmware slot.
// Registered automatically at startup for the platform-defined app image ID.
class AppImageProcessor : public SubImageProcessor
{
public:
    OTAReadiness IsReadyForOTA(uint32_t targetVersion) override;
    CHIP_ERROR   Init(const SubImageHeader & entry) override;
    CHIP_ERROR   Write(ByteSpan block) override;
    void         Abort() override;

    // Called by MultiImageOTAProcessor::Apply() to make the new firmware
    // the active boot target on next reboot.
    CHIP_ERROR Apply();

private:
    // Verifies SHA-256 of the full binary on the last chunk.
    CHIP_ERROR VerifyDigest();

    const esp_partition_t * mPartition     = nullptr;
    esp_ota_handle_t        mOtaHandle     = 0;
    uint64_t                mTotalLength   = 0;
    uint64_t                mBytesReceived = 0;
    uint8_t                 mExpectedSha256[32]{};
    // SHA-256 running context — platform crypto API
};

} // namespace MultiOTA
} // namespace ESP32
} // namespace chip
```

---

## 6. Key method signatures — `MultiImageOTAProcessor.cpp` (sketch)

```cpp
// ProcessPayload — called on Matter thread for every block after outer header stripped
CHIP_ERROR MultiImageOTAProcessor::ProcessPayload(ByteSpan & block)
{
    if (!mHeaderParsed)
        return AccumulateHeader(block);  // Phase 1; falls through when done

    return RoutePayload(block);          // Phase 2
}

// RoutePayload — implements the dispatch loop from DESIGN.md §5.2
CHIP_ERROR MultiImageOTAProcessor::RoutePayload(ByteSpan & block)
{
    while (mCurrentIndex < mNumImages && !block.empty())
    {
        SubImageHeader entry;
        memcpy(&entry, &mSubImages[mCurrentIndex], sizeof(entry));

        if (!mEntryStarted)
        {
            uint64_t gap = entry.offset - mCurrentStreamOffset;
            if (gap > 0)
            {
                mCurrentStreamOffset += gap;
                SkipData(gap);
                return CHIP_NO_ERROR;  // SkipData is the next-block request
            }

            SubImageProcessor * proc = nullptr;
            auto it = mProcessorMap.find(entry.imageId);
            if (it != mProcessorMap.end())
                proc = it->second;

            // No registered processor → assumed already up to date; does not block confirmation
            OTAReadiness readiness = proc ? proc->IsReadyForOTA(entry.version) : OTAReadiness::kAlreadyUpToDate;
            mReadinessMap[entry.imageId] = readiness;
            SaveReadinessMapToNVS();

            if (readiness != OTAReadiness::kReady)
            {
                mCurrentStreamOffset += entry.length;
                SkipData(entry.length);
                mCurrentIndex++;
                mEntryStarted    = false;
                mBytesDelivered  = 0;
                return CHIP_NO_ERROR;
            }

            ReturnErrorOnFailure(proc->Init(entry));
            mEntryStarted = true;
        }

        SubImageProcessor * proc = mProcessorMap[entry.imageId];
        uint64_t toDeliver = std::min(entry.length - mBytesDelivered,
                                      static_cast<uint64_t>(block.size()));

        ReturnErrorOnFailure(proc->Write(block.SubSpan(0, toDeliver)));

        mBytesDelivered      += toDeliver;
        mCurrentStreamOffset += toDeliver;
        block = block.SubSpan(toDeliver);

        if (mBytesDelivered == entry.length)
        {
            mCurrentIndex++;
            mEntryStarted   = false;
            mBytesDelivered = 0;
        }
    }

    FetchNextData();
    return CHIP_NO_ERROR;
}

// ConfirmCurrentImage — gated on all components verified
CHIP_ERROR MultiImageOTAProcessor::ConfirmCurrentImage()
{
    ReturnErrorOnFailure(LoadReadinessMapFromNVS());

    if (!AllComponentsVerified())
    {
        // Invoke retry hook before rollback reboot
        std::vector<uint8_t> pending;
        for (auto & [id, r] : mReadinessMap)
            if (r == OTAReadiness::kNotReady) pending.push_back(id);

        mAttemptCount++;
        SaveReadinessMapToNVS();
        OnPendingNodeUpdates(pending, mAttemptCount);
        return CHIP_ERROR_INCORRECT_STATE;  // triggers platform rollback
    }

    // All verified — commit and clean up
    ReturnErrorOnFailure(esp_ota_mark_app_valid_cancel_rollback());
    EraseNVSState();
    return CHIP_NO_ERROR;
}
```

---

## 7. SDK note — `BDXDownloader::SkipData`

No SDK change is required. `BDXDownloader::SkipData` takes `uint32_t`, which
matches `SubImageHeader.length` and `offset` directly — no cast needed at call
sites.

---

## 8. Packaging tool — `esp32_multi_ota_tool.py`

Located at `scripts/tools/esp32/ota/esp32_multi_ota_tool.py`.
Produces the final `.ota` file directly — no intermediate files, no second step.

Internally it builds the multi-image header blob in memory, then feeds
(header blob + binary files) into the same `ota_image_tool.py` functions
that the standard single-image flow uses.

### Usage

```bash
python3 scripts/tools/esp32/ota/esp32_multi_ota_tool.py create \
    --manifest manifest.csv \
    --vendor-id 0x1234 \
    --product-id 0x5678 \
    --version 15 \
    --version-string "1.0.15" \
    --output firmware.ota
```

### Manifest

```csv
id,version,path
1,14,build/app.bin
128,17,build/coproc.bin
```

Rows are processed top-to-bottom — list entries in write-dependency order
(the order in which sub-processors must receive bytes). See §10.5 of DESIGN.md.

### Implementation sketch

```python
#!/usr/bin/env python3
"""
ESP32 multi-image OTA tool.
Produces a single .ota file ready to serve from a Matter OTA Provider.
"""
import csv
import hashlib
import os
import struct
import sys

import click

# Reuse the outer Matter OTA header logic from the SDK tool
OTA_TOOL_DIR = os.path.join(os.path.dirname(__file__), '../../../../src/app')
sys.path.insert(0, OTA_TOOL_DIR)
import ota_image_tool  # noqa: E402

MULTI_IMAGE_HEADER_MAGIC = 0x4D494F54  # "MIOT"

# Little-endian packed structs
MULTI_HDR_FMT  = '<I B 3s'            # magic(4) + numImages(1) + reserved(3) = 8 B
SUB_HDR_FMT    = '<B I I I 32s 3s'    # id(1)+ver(4)+off(4)+len(4)+sha256(32)+rsv(3) = 48 B
MULTI_HDR_SIZE = struct.calcsize(MULTI_HDR_FMT)   # 8
SUB_HDR_SIZE   = struct.calcsize(SUB_HDR_FMT)     # 48


def sha256_file(path: str) -> bytes:
    h = hashlib.sha256()
    with open(path, 'rb') as f:
        for chunk in iter(lambda: f.read(65536), b''):
            h.update(chunk)
    return h.digest()


def build_multi_image_header(images: list) -> bytes:
    """Build MultiImageHeader + SubImageHeader[] as a bytes blob."""
    num = len(images)
    offset = MULTI_HDR_SIZE + num * SUB_HDR_SIZE

    blob = struct.pack(MULTI_HDR_FMT, MULTI_IMAGE_HEADER_MAGIC, num, b'\x00' * 3)

    for img in images:
        length = os.path.getsize(img['path'])
        digest = sha256_file(img['path'])
        blob += struct.pack(SUB_HDR_FMT,
                            img['id'], img['version'],
                            offset, length,
                            digest, b'\x00' * 3)
        offset += length

    return blob


@click.group()
def cli():
    """ESP32 multi-image OTA builder."""


@cli.command()
@click.option('--manifest',       required=True, type=click.Path(exists=True), help='CSV manifest (id,version,path)')
@click.option('--vendor-id',      required=True, type=lambda x: int(x, 0),    help='Vendor ID  (hex or decimal)')
@click.option('--product-id',     required=True, type=lambda x: int(x, 0),    help='Product ID (hex or decimal)')
@click.option('--version',        required=True, type=int,                     help='softwareVersion (uint32)')
@click.option('--version-string', required=True,                               help='softwareVersionString')
@click.option('--output',         required=True, type=click.Path(),            help='Output .ota file')
def create(manifest, vendor_id, product_id, version, version_string, output):
    """Build a multi-image .ota file from a CSV manifest."""

    with open(manifest, newline='') as f:
        images = [{'id': int(r['id']), 'version': int(r['version']), 'path': r['path']}
                  for r in csv.DictReader(f)]

    # 1. Build multi-image header blob in memory
    header_blob = build_multi_image_header(images)

    # 2. Compute outer OTA payload digest over (header_blob + all binaries)
    outer_digest = hashlib.sha256(header_blob)
    total_payload = len(header_blob)
    for img in images:
        with open(img['path'], 'rb') as f:
            for chunk in iter(lambda: f.read(65536), b''):
                outer_digest.update(chunk)
                total_payload += len(chunk)

    # 3. Build outer Matter OTA header using SDK tool functions
    class _Args:
        vendor_id        = vendor_id
        product_id       = product_id
        version          = version
        version_str      = version_string
        digest_algorithm = 'sha256'
        min_version      = None
        max_version      = None
        release_notes    = None

    header_tlv   = ota_image_tool.generate_header_tlv(_Args(), total_payload, outer_digest.digest())
    outer_header = ota_image_tool.generate_header(header_tlv, total_payload)

    # 4. Write: outer_header + header_blob + binary files
    with open(output, 'wb') as out:
        out.write(outer_header)
        out.write(header_blob)
        for img in images:
            with open(img['path'], 'rb') as f:
                for chunk in iter(lambda: f.read(65536), b''):
                    out.write(chunk)

    click.echo(f"Created {output}: {len(images)} image(s), {os.path.getsize(output)} bytes")


if __name__ == '__main__':
    cli()
```

### What it does

| Step               | Detail                                                                           |
| ------------------ | -------------------------------------------------------------------------------- |
| Header blob        | Built entirely in memory — 8 + N×48 bytes, tiny for any realistic N             |
| Per-binary SHA-256 | Streamed from disk; stored in `SubImageHeader.sha256` for on-device verification |
| Outer OTA digest   | SHA-256 over (header_blob + all binaries) — computed in one pass                |
| Outer header       | Built via imported `ota_image_tool` functions — no duplication of TLV logic     |
| Output             | Single `.ota` = outer Matter header + header blob + binary files                |
| No temp files      | Everything written directly to the output in one sequential pass                 |

---

## 9. NVS key layout

| NVS key              | Type       | Content                                      |
| -------------------- | ---------- | -------------------------------------------- |
| `mota_rmap`          | blob       | Serialised `mReadinessMap` (id → readiness)  |
| `mota_attempt`       | `uint32_t` | `mAttemptCount` for current softwareVersion  |

Both keys are erased by `EraseNVSState()` on successful `ConfirmCurrentImage()`.

### `mota_rmap` blob format

Each entry is 2 bytes, written sequentially for every recorded `imageId`:

```
[ imageId (1 byte) | readiness (1 byte) ][ imageId | readiness ] ...
```

`readiness` encoding: `0` = `kReady`, `1` = `kAlreadyUpToDate`, `2` = `kNotReady`.
Entries are stored in the order they were recorded during the download session.
