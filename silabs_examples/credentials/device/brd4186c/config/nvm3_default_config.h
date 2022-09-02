#ifndef NVM3_DEFAULT_CONFIG_H
#define NVM3_DEFAULT_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <h>NVM3 Default Instance Configuration

#ifndef NVM3_DEFAULT_CACHE_SIZE
// <o NVM3_DEFAULT_CACHE_SIZE> NVM3 Default Instance Cache Size
// <i> Number of NVM3 objects to cache. To reduce access times this number
// <i> should be equal to or higher than the number of NVM3 objects in the
// <i> default NVM3 instance.
// <i> Default: 200
#define NVM3_DEFAULT_CACHE_SIZE  200
#endif

#ifndef NVM3_DEFAULT_MAX_OBJECT_SIZE
// <o NVM3_DEFAULT_MAX_OBJECT_SIZE> NVM3 Default Instance Max Object Size
// <i> Max NVM3 object size that can be stored.
// <i> Default: 254
#define NVM3_DEFAULT_MAX_OBJECT_SIZE  254
#endif

#ifndef NVM3_DEFAULT_REPACK_HEADROOM
// <o NVM3_DEFAULT_REPACK_HEADROOM> NVM3 Default Instance User Repack Headroom
// <i> Headroom determining how many bytes below the forced repack limit the user
// <i> repack limit should be placed. The default is 0, which means the user and
// <i> forced repack limits are equal.
// <i> Default: 0
#define NVM3_DEFAULT_REPACK_HEADROOM  0
#endif

#ifndef NVM3_DEFAULT_NVM_SIZE
// <o NVM3_DEFAULT_NVM_SIZE> NVM3 Default Instance Size
// <i> Size of the NVM3 storage region in flash. This size should be aligned with
// <i> the flash page size of the device.
// <i> Default: 40960
#define NVM3_DEFAULT_NVM_SIZE  32768
#endif

// </h>

// <<< end of configuration section >>>

#endif // NVM3_DEFAULT_CONFIG_H
