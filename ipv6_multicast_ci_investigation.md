# IPv6 Multicast in GitHub Actions macOS 26 Runners - Investigation Summary

**Date:** 2025-12-02
**Investigated by:** Kiel Oleson
**Issue:** Test_TC_SC_5_2 and other multicast tests fail in CI but work locally

## Problem Statement

The Matter test `Test_TC_SC_5_2` (and other multicast-related tests) consistently fail in GitHub Actions CI on macOS 26 runners with `OSError: [Errno 65] No route to host`, but work perfectly on local macOS machines.

## Investigation Timeline

### Initial Hypotheses

1. **TCC (Transparency, Consent, and Control)** - macOS permission system might be blocking local network access
2. **Network filtering** - GitHub Actions might have firewall rules blocking multicast
3. **Code issue** - Incorrect socket configuration for IPv6 multicast

### Diagnostic Approach

1. ✅ Created diagnostic workflow in sandbox repo to gather CI environment data
2. ✅ Examined TCC logs - found no relevant `kTCCServiceLocalNetwork` denials
3. ✅ Captured packet traces locally - multicast packets successfully sent
4. ✅ Compared network configurations between local and CI environments
5. ✅ Created minimal reproducible test cases for both link-local and site-local multicast
6. ✅ Tested with proper IPv6 multicast socket configuration including scope_id

## Key Technical Discovery: IPv6 Multicast Requires scope_id

**Critical breakthrough:** IPv6 multicast requires a 4-tuple address format with explicit scope_id:

```python
# INCORRECT (fails with "No route to host"):
sock.sendto(message, (MCAST_GRP, MCAST_PORT))

# CORRECT (works locally):
sock.sendto(message, (MCAST_GRP, MCAST_PORT, 0, if_index))
```

### Proper IPv6 Multicast Socket Configuration

```python
import socket
import struct

# Get interface index
if_index = socket.if_nametoindex('en0')

# Create socket
sock = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM, socket.IPPROTO_UDP)

# Bind to interface with scope_id
sock.bind(('::', 0, 0, if_index))

# Set multicast options
sock.setsockopt(socket.IPPROTO_IPV6, socket.IPV6_MULTICAST_HOPS, 1)
sock.setsockopt(socket.IPPROTO_IPV6, socket.IPV6_MULTICAST_IF, if_index)

# Join multicast group
group_bin = socket.inet_pton(socket.AF_INET6, MCAST_GRP)
mreq = group_bin + struct.pack('@I', if_index)
sock.setsockopt(socket.IPPROTO_IPV6, socket.IPV6_JOIN_GROUP, mreq)

# Send with 4-tuple address including scope_id
sock.sendto(message, (MCAST_GRP, MCAST_PORT, 0, if_index))
```

## Test Results - By macOS Version

### Summary Table

| macOS Version | Python | C | IPv6 Scopes | Status |
|---------------|--------|---|-------------|--------|
| **14** | ✅ PASS | ✅ PASS | ✅ 12/12 PASS | **All multicast works** |
| **15** | ❌ FAIL | ❌ FAIL | ❌ 0/12 PASS | **Complete IPv6 multicast blackout** |
| **26** | ❌ FAIL | ❌ FAIL | ❌ 0/12 PASS | **Complete IPv6 multicast blackout** |

### Comprehensive Scope Testing Results

Testing all IPv6 multicast scopes revealed the nature of the block:

**macOS 14** - All scopes work:
- ✅ Interface-local (ff01::) - 2/2 pass
- ✅ Link-local (ff02::) - 4/4 pass (including mDNS)
- ✅ Realm-local (ff03::) - 1/1 pass
- ✅ Admin-local (ff04::) - 1/1 pass
- ✅ Site-local (ff05::) - 2/2 pass (including Matter)
- ✅ Organization-local (ff08::) - 1/1 pass
- ✅ Global (ff0e::) - 1/1 pass

**macOS 15 & 26** - Everything fails:
- ❌ Interface-local (ff01::) - Complete block
- ❌ Link-local (ff02::) - Complete block (mDNS broken)
- ❌ All other scopes - Complete block
- Error: `OSError: [Errno 65] No route to host` for ALL addresses

### Critical Finding: Interface-Local Multicast Blocked

Even **interface-local multicast** (ff01::), which by definition should never leave the interface, is blocked. This indicates:
- Not a routing issue
- Not scope-specific filtering
- Complete kernel-level IPv6 multicast disable or blanket packet filter rule

### Local Machine (macOS)
```
============================================================
Testing Link-local multicast (all nodes): ff02::1:5540
============================================================
Interface en0 has index 14
Bound socket to :: with scope_id 14
Set multicast interface to en0
Joined multicast group ff02::1
✅ SUCCESS: Sent 33 bytes to ff02::1:5540

============================================================
Testing Matter site-local multicast: ff35:40:fd00::100:103:5540
============================================================
Interface en0 has index 14
Bound socket to :: with scope_id 14
Set multicast interface to en0
Joined multicast group ff35:40:fd00::100:103
✅ SUCCESS: Sent 49 bytes to ff35:40:fd00::100:103:5540

SUMMARY:
  Link-local (ff02::1): ✅ PASS
  Site-local (ff35:...): ✅ PASS
```

### GitHub Actions macOS 26 Runner
```
============================================================
Testing Link-local multicast (all nodes): ff02::1:5540
============================================================
Interface en0 has index 7
Bound socket to :: with scope_id 7
Set multicast interface to en0
Joined multicast group ff02::1
❌ FAILED: OSError: [Errno 65] No route to host

============================================================
Testing Matter site-local multicast: ff35:40:fd00::100:103:5540
============================================================
Interface en0 has index 7
Bound socket to :: with scope_id 7
Set multicast interface to en0
Joined multicast group ff35:40:fd00::100:103
❌ FAILED: OSError: [Errno 65] No route to host

SUMMARY:
  Link-local (ff02::1): ❌ FAIL
  Site-local (ff35:...): ❌ FAIL
```

## Conclusion

**Root Cause:** IPv6 multicast is completely disabled in GitHub Actions macOS 15+ runners.

**Evidence:**
- macOS 14 runners: All IPv6 multicast works (12/12 scopes pass)
- macOS 15+ runners: Complete IPv6 multicast blackout (0/12 scopes pass)
- Even interface-local multicast (ff01::) fails - indicates kernel-level disable
- Both Python and C implementations fail identically
- Proper socket configuration verified working locally
- No TCC permission denials

**Nature of the Block:**
- Complete blanket ban on IPv6 multicast, not scope-specific
- Affects all addresses from interface-local to global scope
- mDNS (ff02::fb) and neighbor discovery affected
- "No route to host" error suggests packet filter or routing table manipulation

**Historical Context:** This is a regression - multicast worked perfectly on macOS 14 runners but was disabled starting with macOS 15 runners.

**IPv4 Multicast Status:** Testing pending to determine if this is IPv6-specific or affects all multicast.

## Workaround

Skip multicast-dependent tests in CI, as already implemented in `.github/workflows/tests.yaml`:

```yaml
--target-skip-glob '{
  Test_TC_DGTHREAD_2_1,
  Test_TC_DGTHREAD_2_2,
  Test_TC_DGTHREAD_2_3,
  Test_TC_DGTHREAD_2_4,
  TestGroupMessaging,
  Test_TC_ACE_1_6,
  Test_TC_G_*,
  Test_TC_GRPKEY_*,
  Test_TC_S_*,
  Test_TC_SC_5_2
}'
```

## Matter-Specific Technical Details

**Matter Multicast Address:** `ff35:40:fd00::100:103`
- Format: `ffXY:...` where X=flags, Y=scope
- `ff35` = flags:3, scope:5 (site-local)
- Used for group communication in Matter protocol

**Default Port:** 5540 (Matter UDP port)

## Diagnostic Resources

Test files created during investigation:
- `/Users/koleson/Documents/Development/sandbox/test_multicast.py` - Comprehensive test of both multicast types
- `/Users/koleson/Documents/Development/sandbox/test_simple_multicast.py` - Basic link-local test
- `/Users/koleson/Documents/Development/sandbox/test_matter_with_scope.py` - Matter-specific with scope_id

CI diagnostic outputs:
- `/Users/koleson/Downloads/multicast-diagnostics-macos-26/` - Initial diagnostics
- `/Users/koleson/Downloads/multicast-diagnostics-macos-26-2/` - With scope_id in test
- `/Users/koleson/Downloads/multicast-diagnostics-macos-26-3/` - Final comprehensive test

## Recommendations

1. **Continue skipping multicast tests in CI** - This is not a code issue that can be fixed
2. **Report to GitHub Actions** - Consider filing an issue about the multicast regression on macOS 26 runners
3. **Monitor for fixes** - Periodically test if multicast is re-enabled in future runner updates
4. **Document the limitation** - Update test documentation to explain why certain tests only run locally

## References

- Matter Core Specification: IPv6 multicast addressing
- RFC 4291: IP Version 6 Addressing Architecture
- Python socket documentation: IPv6 multicast socket options
- GitHub Actions: macOS runner specifications
