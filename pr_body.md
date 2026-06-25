#### Summary
**Problem**: The documentation in `all-devices-app` regarding supported clusters and device readiness was out of date. Several clusters that were recently migrated to the code-driven model (such as Operational State and its variants, and all Mode Base instances) were still marked as not code-driven in the SDK. Additionally, the `Binding` cluster's usage in `all-devices-app` (via `OnOffLightSwitchDevice`) was not documented, and several unimplemented device types had incorrect readiness statuses.

**Impact**: Developers referencing the documentation might have an incorrect understanding of which clusters are available as code-driven in the SDK, and which device types are ready or blocked for implementation.

**Solution**:
1. Updated `supported_clusters.md` to:
   - Mark `Ambient Context Sensing`, `Operational State`, `Oven Cavity Operational State`, `RVC Operational State`, and all 11 `ModeBase` instances as Code-Driven in the SDK.
   - Mark `Binding` as Used in All-Devices.
   - Update the totals at the bottom (Code-Driven: 86 -> 101, Used: 50 -> 51).
2. Updated `supported_device_types.md` to:
   - Reflect the new readiness of various unimplemented device types. Notably, `Dishwasher`, `Laundry Dryer`, `Laundry Washer`, and `Robotic Vacuum Cleaner` are now marked as `Minimally Ready` (instead of `Blocked`), and `Energy EVSE`, `Irrigation System`, and `Microwave Oven` are now marked as `Ready`.
3. Updated the "Updated as of" date to `2026-06-25` in both files.

#### Testing
This is a documentation-only change. No functional code was modified.
