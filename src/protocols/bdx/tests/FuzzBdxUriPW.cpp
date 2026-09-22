/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
 *      Seeded FuzzTest harness for chip::bdx::ParseURI / chip::bdx::MakeURI.
 *      BDX "bdx://" URIs are parsed during OTA Provider/Requestor exchanges, so
 *      ParseURI is exercised here on arbitrary input.
 *
 *      A raw-bytes mutator would have to discover the "bdx://" scheme +
 *      16-hex-char node id structure from random bytes, so we use a std::string
 *      domain seeded with real valid and edge-case URIs so the mutator starts
 *      near the legal grammar and exercises the scheme / hex / separator /
 *      file-designator boundaries.
 *
 *      Three properties under test:
 *        - ParseURI on arbitrary strings: no crash.
 *        - parse -> make -> parse round-trip: once a URI parses, re-encoding the
 *          (nodeId, fileDesignator) and re-parsing must recover the same nodeId
 *          and a byte-equal file designator (MakeURI emits canonical UPPERCASE
 *          hex, so the second parse is the fixed point).
 *        - make -> parse round-trip: starting from a valid operational NodeId and
 *          a non-empty file designator, MakeURI then ParseURI must recover both
 *          inputs exactly.
 */

#include <string>
#include <vector>

#include <pw_fuzzer/fuzztest.h>
#include <pw_unit_test/framework.h>

#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/NodeId.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <protocols/bdx/BdxUri.h>

namespace {

using namespace chip;
using namespace fuzztest;

void EnsureInitialized()
{
    static const bool sInitialized = [] {
        VerifyOrDie(Platform::MemoryInit() == CHIP_NO_ERROR);
        return true;
    }();
    (void) sInitialized;
}

// kScheme ("bdx://") is the only grammar constant exported by BdxUri.h. The
// node-id field is always sizeof(NodeId)*2 hex chars; '+1' for the '/' separator
// and '+1' for MakeURI's reserved NUL terminator.
constexpr size_t kSchemeLen      = sizeof(bdx::kScheme) - 1;
constexpr size_t kEncodeOverhead = kSchemeLen + sizeof(NodeId) * 2 + 1 /* '/' */ + 1 /* NUL */;

// A mix of valid (operational node id, non-empty file designator) URIs and the
// malformed forms ParseURI must reject without crashing. Drives the scheme,
// hex-decode, operational-node-id, separator and file-designator branches.
std::vector<std::string> UriStringSeeds()
{
    return {
        // ==== Valid, parseable ====
        "bdx://0000000000000001/the-file",          // smallest operational node id
        "bdx://1234567890ABCDEF/file_name",         // canonical (matches TestBdxUri)
        "bdx://1234567890abcdef/file_name",         // lowercase hex (HexToBytes accepts it)
        "bdx://FEDCBA9876543210/firmware.bin",      // upper-half operational id
        "bdx://1234567890ABCDEF/file/na/me",        // slashes are part of the designator
        "bdx://0000000000000001/a",                 // 1-char file designator (min length)
        "bdx://FFFFFFEFFFFFFFFF/ota.bin",           // largest operational node id
        "bdx://0000000000000001/dir/sub/img.ota",   // path-like designator
        "bdx://1234567890ABCDEF/%2e%2e%2ffirmware", // percent-encodings (NOT decoded by ParseURI)
        "bdx://1234567890ABCDEF/file with spaces",  // spaces in designator
        // ==== Malformed: scheme ====
        "",                                  // empty
        "bdx:/",                             // truncated scheme
        "bdx://",                            // scheme only, nothing after
        "bdy://1234567890ABCDEF/file_name",  // wrong scheme byte
        "BDX://1234567890ABCDEF/file_name",  // wrong-case scheme
        "http://1234567890ABCDEF/file_name", // unrelated scheme
        "1234567890ABCDEF/file_name",        // missing scheme entirely
        // ==== Malformed: node id ====
        "bdx://1234567890ABCDE/file_name",    // node id too short (15 hex)
        "bdx://1234567890ABCDEFEF/file_name", // node id too long (18 hex)
        "bdx://1234567890ABCDEG/file_name",   // non-hex digit 'G'
        "bdx://12345678 0ABCDEF/file_name",   // space inside node id
        "bdx://0000000000000000/file_name",   // kUndefinedNodeId (not operational)
        "bdx://FFFFFFFFFFFFFFFF/file_name",   // group id range (not operational)
        "bdx://FFFFFFFE00000000/file_name",   // temporary-local id (not operational)
        "bdx://FFFFFFFD00000000/file_name",   // CASE auth tag range (not operational)
        // ==== Malformed: separator / file designator ====
        "bdx://1234567890ABCDEF.file_name", // wrong separator (no '/')
        "bdx://1234567890ABCDEF/",          // empty file designator
        "bdx://1234567890ABCDEF",           // no separator, no designator
        // ==== Edge / overflow shapes ====
        std::string("bdx://1234567890ABCDEF/") + std::string(4096, 'A'), // very long designator
        std::string("bdx://1234567890ABCDEF/") + std::string("a\0b", 3), // embedded NUL in designator
    };
}

// Operational node ids only (1 .. kMaxOperationalNodeId), so MakeURI/ParseURI
// agree. kUndefinedNodeId (0) and the carved-out high ranges are excluded.
auto AnyOperationalNodeId()
{
    return InRange<NodeId>(1, kMaxOperationalNodeId);
}

// Non-empty file designators. MakeURI rejects an empty designator, so the
// make->parse oracle only makes sense for size >= 1.
auto AnyFileDesignator()
{
    return Arbitrary<std::string>().WithMinSize(1).WithMaxSize(512);
}

// Property 1: ParseURI on arbitrary bytes must never crash.
void ParseUriNoCrash(const std::string & uriStr)
{
    EnsureInitialized();

    CharSpan uri(uriStr.data(), uriStr.size());
    NodeId nodeId = 0;
    CharSpan fileDesignator;
    (void) bdx::ParseURI(uri, nodeId, fileDesignator);
}

FUZZ_TEST(BdxUriPW, ParseUriNoCrash).WithDomains(Arbitrary<std::string>().WithSeeds(UriStringSeeds()).WithMaxSize(8192));

// Property 2: parse -> make -> parse is a fixed point. Once a URI parses, the
// canonical re-encoding of (nodeId, fileDesignator) must parse back to the same
// nodeId and a byte-equal designator.
void ParseMakeRoundtrip(const std::string & uriStr)
{
    EnsureInitialized();

    CharSpan uri(uriStr.data(), uriStr.size());
    NodeId nodeId = 0;
    CharSpan fileDesignator;
    if (bdx::ParseURI(uri, nodeId, fileDesignator) != CHIP_NO_ERROR)
        return;

    // The parse succeeded, so nodeId is operational and the designator is non-empty.
    // Re-encode into a buffer large enough for scheme + node-id hex + '/' + designator + NUL.
    std::vector<char> buffer(kEncodeOverhead + fileDesignator.size());
    MutableCharSpan encoded(buffer.data(), buffer.size());
    ASSERT_EQ(bdx::MakeURI(nodeId, fileDesignator, encoded), CHIP_NO_ERROR);

    NodeId nodeId2 = 0;
    CharSpan fileDesignator2;
    ASSERT_EQ(bdx::ParseURI(encoded, nodeId2, fileDesignator2), CHIP_NO_ERROR);
    ASSERT_EQ(nodeId2, nodeId);
    ASSERT_TRUE(fileDesignator2.data_equal(fileDesignator));
}

FUZZ_TEST(BdxUriPW, ParseMakeRoundtrip).WithDomains(Arbitrary<std::string>().WithSeeds(UriStringSeeds()).WithMaxSize(8192));

// Property 3: make -> parse recovers structured input exactly. Starting from a
// valid operational NodeId and a non-empty file designator, MakeURI then
// ParseURI must yield the same nodeId and a byte-equal designator.
void MakeParseRoundtrip(NodeId nodeId, const std::string & fileDesignatorStr)
{
    EnsureInitialized();

    CharSpan fileDesignator(fileDesignatorStr.data(), fileDesignatorStr.size());

    std::vector<char> buffer(kEncodeOverhead + fileDesignator.size());
    MutableCharSpan encoded(buffer.data(), buffer.size());
    ASSERT_EQ(bdx::MakeURI(nodeId, fileDesignator, encoded), CHIP_NO_ERROR);

    NodeId parsedNodeId = 0;
    CharSpan parsedFileDesignator;
    ASSERT_EQ(bdx::ParseURI(encoded, parsedNodeId, parsedFileDesignator), CHIP_NO_ERROR);
    ASSERT_EQ(parsedNodeId, nodeId);
    ASSERT_TRUE(parsedFileDesignator.data_equal(fileDesignator));
}

FUZZ_TEST(BdxUriPW, MakeParseRoundtrip).WithDomains(AnyOperationalNodeId(), AnyFileDesignator());

} // namespace
