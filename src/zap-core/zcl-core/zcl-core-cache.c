/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#include EMBER_AF_API_BUFFER_MANAGEMENT
#include EMBER_AF_API_BUFFER_QUEUE
#include EMBER_AF_API_HAL
#include EMBER_AF_API_ZCL_CORE

// -----------------------------------------------------------------------------
// Description.

// The ZCL Address Cache contains mappings of key/value pairs where the key
// is a network host identifier (e.g., UID or hostname) and the value is a
// network address for that host (e.g., IPv6 address).
//
// Cache entries are unique by key. More than one entry (different keys) MAY
// resolve to the same value/address.
//
// The cache is implemented as a singly-linked circular queue of Buffers,
// managed using the buffer-queue API. The queue anchor 'addressCache' points
// to the tail buffer in the queue, which in turn points to the head buffer
// in the queue. Only the buffer-queue API functions should be used to
// insert/remove queue entries, to ensure the queue anchor/tail/head linkages
// are properly maintained.
//
// The entries in the queue are kept in Most Recently Used (MRU) order from
// head to tail. When a new entry is inserted, or when an existing entry is
// accessed to resolve an address, the entry is placed at the head of the
// queue, pushing unused entries toward the tail. Searches proceed linearly
// from head to tail. MRU order ensures that more frequently accessed entries
// are found most quickly.
//
// If a new entry needs to be added, but the cache is at maximum capacity or a
// new Buffer cannot be allocated for it, the Least Recently Used (LRU) cache
// entry (tail) is reused for the new entry.
//
// If a new entry needs to be added and there is an existing entry with the
// same key, the existing entry is reused for the new entry.
//
// Each entry is assigned an index number when created, which is used as a
// reference to the entry for CLI operations. Initially the index number
// space runs from 0x0000-0xFFFF. Upon wrapping, an unused range from within
// that number space is located and (re)used. When the upper bound of that
// range is reached, another unused range is chosen, and so on.

// -----------------------------------------------------------------------------
// Structures.
typedef struct {
  EmZclCacheIndex_t index;
  EmZclCacheIndex_t lower;
  EmZclCacheIndex_t upper;
} CacheIndexGenerator;

typedef struct {
  const EmberIpv6Address *prefix;
  uint8_t prefixLengthInBits;
} CacheIpv6Prefix;

// -----------------------------------------------------------------------------
// Statics.

static Buffer addressCache = NULL_BUFFER; // buffer-queue tail element
static size_t entryCount = 0;
static CacheIndexGenerator indexGenerator = {
  .index = 0,
  .lower = 0,
  .upper = 0xFFFF,
};

// -----------------------------------------------------------------------------
// Macros.

#define findCacheEntryBufferByKey(key) \
  findCacheEntryBuffer((key), matchCacheKey)

#define findCacheEntryBufferByValue(value) \
  findCacheEntryBuffer((value), matchCacheValue)

#define findCacheEntryBufferByIndex(index) \
  findCacheEntryBuffer((&index), matchCacheIndex)

// -----------------------------------------------------------------------------
// Plugin handlers.

void emZclCacheNetworkStatusHandler(EmberNetworkStatus newNetworkStatus,
                                    EmberNetworkStatus oldNetworkStatus,
                                    EmberJoinFailureReason reason)
{
  // If the device is no longer associated with a network, its address
  // cache is cleared.
  if (newNetworkStatus == EMBER_NO_NETWORK) {
    emZclCacheRemoveAll();
  }
}

void emZclCacheMarkApplicationBuffersHandler(void)
{
  emMarkBuffer(&addressCache);
}

// -----------------------------------------------------------------------------
// Utility functions.

static void initCacheIndexGenerator(void)
{
  indexGenerator.index = 0;
  indexGenerator.lower = 0;
  indexGenerator.upper = 0xFFFF;
}

static bool resolveGeneratorIndexBounds(const void *criteria, const EmZclCacheEntry_t *entry);
static EmZclCacheIndex_t generateIndex(void)
{
  // Choose a random trial value within the number space; then scan cache
  // entries and check their index values to determine the lower and
  // upper bounds about the trial value that delimit unused index numbers.
  // Repeat if necessary (rare) until a suitable range is found.
  if (indexGenerator.index == indexGenerator.upper) {
    // Resolve a range of at least 100 unused index values.
    do {
      indexGenerator.index = halCommonGetRandom();
      indexGenerator.lower = 0x0;
      indexGenerator.upper = 0xFFFF;
      emZclCacheScan(&indexGenerator, resolveGeneratorIndexBounds);
    } while (indexGenerator.index == 0xFFFF
             || (indexGenerator.upper - indexGenerator.lower) < 100);
    indexGenerator.index = indexGenerator.lower;
  }
  return indexGenerator.index++;
}

static bool canAddEntry(void)
{
  // If default cache table size has been reached, can exceed it to
  // accommodate a greater number of active bindings.
  if (entryCount < EMBER_ZCL_CACHE_TABLE_SIZE
      || entryCount < emZclGetBindingCount()) {
    return true;
  }
  return false;
}

static void fillCacheEntry(EmZclCacheEntry_t *entry,
                           const EmberZclUid_t *key,
                           const EmberIpv6Address *value,
                           bool assignIndex)
{
  MEMCOPY(&entry->key, key, sizeof(EmberZclUid_t));
  MEMCOPY(&entry->value, value, sizeof(EmberIpv6Address));
  if (assignIndex) {
    entry->index = generateIndex();
  }
}

static Buffer findCacheEntryBuffer(const void *criteria,
                                   EmZclCacheScanPredicate match)
{
  for (Buffer buffer = emBufferQueueHead(&addressCache);
       buffer != NULL_BUFFER;
       buffer = emBufferQueueNext(&addressCache, buffer)) {
    const EmZclCacheEntry_t *entry
      = (const EmZclCacheEntry_t *)emGetBufferPointer(buffer);
    if ((*match)(criteria, entry)) {
      return buffer;
    }
  }
  return NULL_BUFFER;
}

static size_t removeAllForCriteria(const void *criteria,
                                   EmZclCacheScanPredicate match)
{
  // Retain only the entries that DON'T match the criteria.
  Buffer temp = addressCache;
  addressCache = NULL_BUFFER;
  size_t removedCount = 0;
  entryCount = 0;
  while (!emBufferQueueIsEmpty(&temp)) {
    Buffer next = emBufferQueueRemoveHead(&temp);
    const EmZclCacheEntry_t *entry
      = (const EmZclCacheEntry_t *)emGetBufferPointer(next);
    if ((*match)(criteria, entry)) {
      removedCount++;
    } else {
      emBufferQueueAdd(&addressCache, next);
      entryCount++;
    }
  }
  return removedCount;
}

static void makeMostRecentlyUsed(Buffer buffer)
{
  if (buffer != emBufferQueueHead(&addressCache)) {
    emBufferQueueRemove(&addressCache, buffer);
    emBufferQueueAddToHead(&addressCache, buffer);
  }
}

// -----------------------------------------------------------------------------
// EmZclCacheScanPredicates to search/match entry per criteria

static bool matchCacheKey(const void *criteria, const EmZclCacheEntry_t *entry)
{
  return (MEMCOMPARE((uint8_t *) criteria,
                     &entry->key,
                     sizeof(EmberZclUid_t))
          == 0);
}

static bool matchCacheValue(const void *criteria, const EmZclCacheEntry_t *entry)
{
  return (MEMCOMPARE((uint8_t *) criteria,
                     &entry->value,
                     sizeof(EmberIpv6Address))
          == 0);
}

static bool matchCacheIndex(const void *criteria, const EmZclCacheEntry_t *entry)
{
  EmZclCacheIndex_t index = *((const EmZclCacheIndex_t *)criteria);
  return (index == entry->index);
}

static bool matchCacheValueIpv6Prefix(const void *criteria,
                                      const EmZclCacheEntry_t *entry)
{
  const CacheIpv6Prefix *cacheIpv6Prefix = (const CacheIpv6Prefix *)criteria;
  return (emMatchingPrefixBitLength(cacheIpv6Prefix->prefix->bytes,
                                    cacheIpv6Prefix->prefixLengthInBits,
                                    entry->value.bytes,
                                    EMBER_IPV6_BITS)
          == cacheIpv6Prefix->prefixLengthInBits);
}

static bool resolveGeneratorIndexBounds(const void *criteria,
                                        const EmZclCacheEntry_t *entry)
{
  CacheIndexGenerator *generator = (CacheIndexGenerator *)criteria;
  if (entry->index == generator->index) {
    // The trial value itself is used. Set failure value and abort scan.
    generator->index = 0xFFFF;
    return true;
  }
  if (entry->index > generator->index && entry->index < generator->upper) {
    generator->upper = entry->index;
  }
  if (entry->index < generator->index && entry->index > generator->lower) {
    generator->lower = entry->index;
  }
  return false; // force traversal of all entries
}

// -----------------------------------------------------------------------------
// Internal.

size_t emZclCacheGetEntryCount(void)
{
  return entryCount;
}

void emZclCacheScan(const void *criteria, EmZclCacheScanPredicate match)
{
  findCacheEntryBuffer(criteria, match);
}

bool emZclCacheAdd(const EmberZclUid_t *key,
                   const EmberIpv6Address *value,
                   EmZclCacheIndex_t *index)
{
  Buffer buffer = findCacheEntryBufferByKey(key);
  if (buffer == NULL_BUFFER) {
    if (canAddEntry()) {
      // Allocate new buffer.
      EmZclCacheEntry_t entry;
      fillCacheEntry(&entry, key, value, true); // assign entry index
      buffer = emFillBuffer((const uint8_t *)&entry, sizeof(EmZclCacheEntry_t));
      if (buffer != NULL_BUFFER) {
        emBufferQueueAddToHead(&addressCache, buffer);
        entryCount++;
        if (index != NULL) {
          *index = entry.index;
        }
        return true;
      }
    }
    // Either cache is full, or new buffer allocation failed; reuse the
    // least recently used entry (queue tail).
    buffer = addressCache;
  }

  if (buffer != NULL_BUFFER) {
    // Reuse an existing buffer, which was found by key or is LRU entry.
    EmZclCacheEntry_t *entry = (EmZclCacheEntry_t *)emGetBufferPointer(buffer);
    fillCacheEntry(entry, key, value, false); // reuse existing entry index
    makeMostRecentlyUsed(buffer);
    if (index != NULL) {
      *index = entry->index;
    }
    return true;
  }

  return false;
}

bool emZclCacheGet(const EmberZclUid_t *key,
                   EmberIpv6Address *value)
{
  Buffer buffer = findCacheEntryBufferByKey(key);
  if (buffer != NULL_BUFFER) {
    if (value != NULL) {
      const EmZclCacheEntry_t *entry
        = (const EmZclCacheEntry_t *)emGetBufferPointer(buffer);
      MEMCOPY(value, &entry->value, sizeof(EmberIpv6Address));
    }
    makeMostRecentlyUsed(buffer);
    return true;
  }
  return false;
}

bool emZclCacheGetByIndex(EmZclCacheIndex_t index,
                          EmberZclUid_t *key,
                          EmberIpv6Address *value)
{
  Buffer buffer = findCacheEntryBufferByIndex(index);
  if (buffer != NULL_BUFFER) {
    const EmZclCacheEntry_t *entry
      = (const EmZclCacheEntry_t *)emGetBufferPointer(buffer);
    if (key != NULL) {
      MEMCOPY(key, &entry->key, sizeof(EmberZclUid_t));
    }
    if (value != NULL) {
      MEMCOPY(value, &entry->value, sizeof(EmberIpv6Address));
    }
    return true;
  }
  return false;
}

bool emZclCacheGetFirstKeyForValue(const EmberIpv6Address *value,
                                   EmberZclUid_t *key)
{
  Buffer buffer = findCacheEntryBufferByValue(value);
  if (buffer != NULL_BUFFER) {
    if (key != NULL) {
      const EmZclCacheEntry_t *entry
        = (const EmZclCacheEntry_t *)emGetBufferPointer(buffer);
      MEMCOPY(key, &entry->key, sizeof(EmberZclUid_t));
    }
    return true;
  }
  return false;
}

bool emZclCacheGetIndex(const EmberZclUid_t *key,
                        EmZclCacheIndex_t *index)
{
  Buffer buffer = findCacheEntryBufferByKey(key);
  if (buffer != NULL_BUFFER) {
    if (index != NULL) {
      const EmZclCacheEntry_t *entry
        = (const EmZclCacheEntry_t *)emGetBufferPointer(buffer);
      *index = entry->index;
    }
    return true;
  }
  return false;
}

bool emZclCacheRemove(const EmberZclUid_t *key)
{
  Buffer buffer = findCacheEntryBufferByKey(key);
  if (buffer != NULL_BUFFER) {
    emBufferQueueRemove(&addressCache, buffer);
    entryCount--;
    return true;
  }
  return false;
}

bool emZclCacheRemoveByIndex(EmZclCacheIndex_t index)
{
  Buffer buffer = findCacheEntryBufferByIndex(index);
  if (buffer != NULL_BUFFER) {
    emBufferQueueRemove(&addressCache, buffer);
    entryCount--;
    return true;
  }
  return false;
}

void emZclCacheRemoveAll(void)
{
  addressCache = NULL_BUFFER;
  entryCount = 0;
  initCacheIndexGenerator();
}

size_t emZclCacheRemoveAllByValue(const EmberIpv6Address *value)
{
  return removeAllForCriteria(value, matchCacheValue);
}

size_t emZclCacheRemoveAllByIpv6Prefix(const EmberIpv6Address *prefix,
                                       uint8_t prefixLengthInBits)
{
  if (prefixLengthInBits > EMBER_IPV6_BITS) {
    return false;
  }
  CacheIpv6Prefix cacheIpv6Prefix = {
    .prefix = prefix,
    .prefixLengthInBits = prefixLengthInBits,
  };
  return removeAllForCriteria(&cacheIpv6Prefix, matchCacheValueIpv6Prefix);
}
