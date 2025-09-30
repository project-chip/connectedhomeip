/* @@@LICENSE
 *
 * Copyright (c) 2025 LG Electronics, Inc.
 *
 * Confidential computer software. Valid license from LG required for
 * possession, use or copying. Consistent with FAR 12.211 and 12.212,
 * Commercial Computer Software, Computer Software Documentation, and
 * Technical Data for Commercial Items are licensed to the U.S. Government
 * under vendor's standard commercial license.
 *
 * LICENSE@@@
 */
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceConfig.h>

namespace chip {
namespace Platform {
namespace Internal {

void AssertChipStackLockedByCurrentThread(const char * file, int line);

} // namespace Internal
} // namespace Platform
} // namespace chip
