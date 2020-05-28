/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019-2020 Google LLC.
 *    Copyright (c) 2018 Nest Labs, Inc.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *          This is a just a wrapper around NSPR's parameterized dynamic
 *          has table.
 */

#ifndef NLHASHTABLE_H
#define NLHASHTABLE_H

#include <nspr/plhash.h>
#include <nspr/prtypes.h>

typedef PLHashTable HashTable;

PLHashNumber hash_key(const void * key);

#define HASHTABLE_NEW() PL_NewHashTable(0, hash_key, PL_CompareStrings, PL_CompareValues, NULL, NULL)
#define HASHTABLE_NEW_ALLOCOPS(o) PL_NewHashTable(0, hash_key, PL_CompareStrings, PL_CompareValues, (o), NULL)
#define HASHTABLE_DESTROY(h) PL_HashTableDestroy((h))
#define HASHTABLE_LOOKUP(h, k) PL_HashTableLookupConst((h), (k))
#define HASHTABLE_ADD(h, k, v) PL_HashTableAdd((h), (k), (v))
#define HASHTABLE_REMOVE(h, k) PL_HashTableRemove((h), (k))
#define HASHTABLE_ENUMERATE(h, f, c) PL_HashTableEnumerateEntries((h), (f), (c))

#endif
