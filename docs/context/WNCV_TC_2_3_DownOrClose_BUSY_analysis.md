# TC-WNCV-2.3: DownOrClose BUSY (0x9c) — Analysis Reference

This document summarizes an investigation into why **Window Covering** certification test **TC-WNCV-2.3** sometimes fails at **Step 3b**, where **DownOrClose** is expected to return **BUSY** (`0x9c`) while the DUT is in **Maintenance mode**, but device logs show a **success-style path** (“no delegate”) instead.

Use this as **context for LLMs or engineers** debugging similar failures on Matter Window Covering servers.

---

## 1. Problem statement

- **Symptom:** In the **failure** run, `DownOrClose` during Maintenance mode does **not** surface as BUSY; logs match an **unlocked** execution path (`WindowCovering has no delegate set`, successful invoke handling).
- **Expected:** Step **3b** requires **BUSY** (`0x9c`) when Maintenance mode is active (`Mode` attribute bit 2 / value `0x04`).
- **Contrast:** In a **success** run on similar hardware, the same step shows **`Err device locked`** with status **`0x9c`**.

---

## 2. Certification test definition

**File:** `src/app/tests/suites/certification/Test_TC_WNCV_2_3.yaml`

**Test name:** `36.2.3. [TC-WNCV-2.3] Mode attribute [DUT as Server]`

**Relevant step — Step 3b (Maintenance mode):**

- After Step 3a sets **`Mode`** with bit 2 (Maintenance), the **Test Harness (TH)** sends **`DownOrClose`**.
- **Expected response:** `error: BUSY` (Matter status **0x9c**).

```yaml
# Step 3a: Set Maintenance mode (Mode bit 2 → 0x04)
- label: "Step 3a: TH set the Mode Attribute bit2 of the DUT"
  command: "writeAttribute"
  attribute: "Mode"
  PICS: "WNCV.S.M.Maintenance && WNCV.S.A0017"
  arguments:
      value: 0x04

# Step 3b: Expect BUSY on DownOrClose
- label: "Step 3b: TH send DownOrClose command to the DUT"
  command: "DownOrClose"
  PICS: "WNCV.S.M.Maintenance && WNCV.S.C01.Rsp"
  response:
      error: BUSY
```

**PICS note:** Step runs only when `WNCV.S.M.Maintenance && WNCV.S.C01.Rsp` applies.

---

## 3. Observed logs — success vs failure

Logs referenced in the original analysis (paths may be local to a workspace):

| Artifact | Typical path |
|----------|----------------|
| Success device log | `wncv_success_logs.txt` |
| Failure device log | `wncv_failure_logs.txt` |

### 3.1 Step 3b behavior

| Run | DownOrClose outcome | DMG / ZCL signature |
|-----|----------------------|---------------------|
| **Success** | BUSY | `Err device locked` → command status **`0x9c`** |
| **Failure** | Not BUSY | `WindowCovering has no delegate set for endpoint:1` (same pattern as an **accepted** motion command when not locked) |

### 3.2 Timing / ordering difference (critical)

**Success log (`wncv_success_logs.txt`):**

- **`Mode 0x04`** (and associated ZCL prints for Mode / ConfigStatus) appears **before** the Step 3b **`DownOrClose`** handling.
- Sequence is **quiet** between the Maintenance **write** and **`DownOrClose`** (no large burst of simulated movement between those steps).

**Failure log (`wncv_failure_logs.txt`):**

- The **`DownOrClose`** that should be Step 3b is processed **before** the delayed **`Mode 0x04`** log line appears in the trace.
- Heavy **asynchronous activity** appears around the same window:
  - `Lift[1] Position Set` / `Tilt[1] Position Set`
  - `OperationalStatus raw=...` lines
- **Platform errors:** e.g. `Failed to post event to app task event queue` — suggests **event-queue pressure** or dropped/delayed work.

**Interpretation:** In the failure case, **`DownOrClose` runs in a window where the cluster’s notion of “locked for maintenance” is not yet consistent** with what the test assumes (see §5).

---

## 4. Server implementation flow (Matter tree)

### 4.1 Primary source file

**File:** `src/app/clusters/window-covering-server/WindowCoveringCluster.cpp`

This file defines:

- **`GetMotionLockStatus(endpoint)`** — decides whether motion commands are rejected with **Failure**, **Busy**, or allowed (**Success**).
- **`emberAfWindowCoveringClusterDownOrCloseCallback`** — **`DownOrClose`** command handler.
- **`emberAfWindowCoveringClusterUpOrOpenCallback`** — **`UpOrOpen`** command handler (ordering differs slightly from **`DownOrClose`**; see §4.4).
- **`PostAttributeChange`** — reacts to attribute updates; for **`Mode`**, calls **`ModeSet`** (which updates **`ConfigStatus`** vs **`Mode`**).

Related headers / delegate:

- `src/app/clusters/window-covering-server/window-covering-server.h`
- `src/app/clusters/window-covering-server/window-covering-delegate.h`

### 4.2 How Maintenance mode ties to ConfigStatus

**`ModeSet`** (same file) clears **`ConfigStatus::kOperational`** when **Calibration** or **Maintenance** mode is active:

```cpp
newStatus.Set(ConfigStatus::kOperational, !newMode.HasAny(Mode::kMaintenanceMode, Mode::kCalibrationMode));
```

So **in steady state**, Maintenance mode ⇒ **`kOperational`** false on **`ConfigStatus`**.

### 4.3 PostAttributeChange for Mode

When **`Mode`** attribute changes, **`PostAttributeChange`** runs:

```cpp
case Attributes::Mode::Id:
    mode = ModeGet(endpoint);
    ModePrint(mode);
    ModeSet(endpoint, mode); // refilter mode if needed
    break;
```

This is where **`Mode`** and **`ConfigStatus`** are reconciled after writes (subject to **when** this runs relative to other requests).

### 4.4 GetMotionLockStatus — central gate for BUSY / FAILURE

```cpp
Status GetMotionLockStatus(chip::EndpointId endpoint)
{
    BitMask<Mode> mode                 = ModeGet(endpoint);
    BitMask<ConfigStatus> configStatus = ConfigStatusGet(endpoint);

    if (!configStatus.Has(ConfigStatus::kOperational))
    {
        if (mode.Has(Mode::kMaintenanceMode))
            return Status::Busy;

        if (mode.Has(Mode::kCalibrationMode))
            return Status::Failure;
    }

    return Status::Success;
}
```

**Important logical property:**

- **Maintenance BUSY** is only returned from inside `if (!configStatus.Has(ConfigStatus::kOperational))`.
- If **`kOperational`** is still **set** (`Has(kOperational) == true`), the function returns **`Status::Success`** immediately — **it does not evaluate Maintenance or Calibration mode** in that branch.

Therefore **any transient state where `Mode` indicates Maintenance but `ConfigStatus` still has `kOperational` true** will **not** produce BUSY.

### 4.5 DownOrClose handler (abbreviated flow)

1. Log: `DownOrClose command received`.
2. **`status = GetMotionLockStatus(endpoint)`**.
3. If **`status != Success`**: log `Err device locked`, **`AddStatus(commandPath, status)`**, return (**BUSY or FAILURE path**).
4. Else: set target positions (lift/tilt), **`AddStatus(Success)`**, then delegate / “no delegate” log.

**Ordering note vs `UpOrOpen`:**

- **`DownOrClose`** calls **`commandObj->AddStatus(..., Success)`** **before** the delegate block.
- **`UpOrOpen`** adds **Success** **after** the delegate / “no delegate” block.

That asymmetry can matter for response composition in edge cases, but **BUSY vs Success for Step 3b is decided at step 2** (`GetMotionLockStatus`), before targets are set.

---

## 5. Root cause hypothesis (why BUSY “disappears”)

**Hypothesis:** Step 3b sometimes runs **before `PostAttributeChange` → `ModeSet` has updated `ConfigStatus`** to clear **`kOperational`**, or under load **`ConfigStatus` and `Mode` are temporarily inconsistent**.

Supporting observations:

1. **`GetMotionLockStatus`** keys off **`ConfigStatus::kOperational` first**; stale **`kOperational`** ⇒ **Success** ⇒ no BUSY even if **`Mode`** already reflects Maintenance in storage (depending on exact read ordering and persistence).
2. Failure logs show **heavy simulated movement** and **event queue failures**, consistent with **delayed** attribute post-processing.
3. Failure logs show **`Mode 0x04`** printed **after** the problematic **`DownOrClose`**, while success logs show **`Mode 0x04`** **before** that command — consistent with **ordering/timing**, not with a different firmware branch per se.

**Not the primary explanation:** The string **`WindowCovering has no delegate set`** alone — that log appears whenever **Success** was already chosen and the handler runs the no-delegate branch; it is a **consequence**, not the cause of missing BUSY.

---

## 6. Potential remediation directions (for implementers)

These are **design directions**, not mandated patches:

1. **Make lock detection depend on `Mode` directly for Maintenance/Calibration**  
   e.g. evaluate **`mode.Has(Mode::kMaintenanceMode)`** / **`kCalibrationMode`** **before** or **independent of** stale **`kOperational`**, so BUSY/Failure cannot be skipped due to **`ConfigStatus`** lag.

2. **Ensure `ModeSet` / `ConfigStatus` update runs synchronously** with **`Mode`** writes on the server path that commits the attribute (so **`ConfigStatus`** cannot lag **`Mode`** visible to **`GetMotionLockStatus`**).

3. **Reduce app-thread starvation** on the DUT so **`PostAttributeChange`** is not delayed behind motor simulation (addresses symptoms seen with **`Failed to post event to app task event queue`**).

4. **Optional:** Align **`DownOrClose`** **`AddStatus(Success)`** placement with **`UpOrOpen`** for consistency (secondary; does not fix BUSY by itself).

---

## 7. Quick reference — files touched by this analysis

| Path | Role |
|------|------|
| `src/app/tests/suites/certification/Test_TC_WNCV_2_3.yaml` | Certification YAML; Step 3b expects **`error: BUSY`** on **`DownOrClose`** in Maintenance mode |
| `src/app/clusters/window-covering-server/WindowCoveringCluster.cpp` | **`GetMotionLockStatus`**, **`DownOrClose`** / **`UpOrOpen`**, **`PostAttributeChange`**, **`ModeSet`** |
| `src/app/clusters/window-covering-server/window-covering-delegate.h` | Delegate API (**`HandleMovement`**, etc.) |
| `examples/all-clusters-app/linux/WindowCoveringManager.cpp` | Example delegate + timers (Linux **all-clusters-app** sets a delegate; logs saying **no delegate** imply a different app/image) |

---

## 8. Sequence diagrams (conceptual)

### 8.1 Intended steady state after Maintenance write

```text
TH writes Mode (0x04)
    → Mode persisted
    → PostAttributeChange(Mode)
        → ModeSet()
            → ConfigStatus.kOperational cleared
TH invokes DownOrClose
    → GetMotionLockStatus()
        → !kOperational && Maintenance → Busy (0x9c)
```

### 8.2 Suspected failure window

```text
TH writes Mode (0x04)
    → Mode persisted (possibly visible to ModeGet)
    → PostAttributeChange delayed OR ConfigStatus not yet updated
TH invokes DownOrClose (too early or under race)
    → GetMotionLockStatus()
        → kOperational still true → Success
    → Handler sets targets, logs "no delegate"
```

---

## 9. Revision note

This document was produced from log comparison and code reading in the **connectedhomeip** tree. Line numbers in **`WindowCoveringCluster.cpp`** may drift; always verify against the current file.

---

## Document metadata

- **Purpose:** LLM / engineer context for TC-WNCV-2.3 Step 3b **`DownOrClose`** BUSY behavior.
- **Location:** `docs/context/WNCV_TC_2_3_DownOrClose_BUSY_analysis.md`
