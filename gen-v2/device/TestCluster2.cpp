#include "TestCluster2-Gen.h"
#include "core/CHIPTLVTags.h"
#include "core/CHIPTLVTypes.h"

#pragma GCC diagnostic ignored "-Wstack-usage="

namespace chip {
namespace app {
namespace Cluster {
namespace TestCluster2 {
    namespace StructA {
        CHIP_ERROR Type::Encode(TLV::TLVWriter &writer, uint64_t tag) {
            TLV::TLVType outer;
            ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outer));
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kFieldIdJ), x));
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kFieldIdK), y));
            ReturnErrorOnFailure(writer.PutBytes(TLV::ContextTag(kFieldIdL), l.data(), l.size()));
            ReturnErrorOnFailure(writer.PutString(TLV::ContextTag(kFieldIdM), m.data(), m.size()));
            ReturnErrorOnFailure(writer.EndContainer(outer));
            ReturnErrorOnFailure(writer.Finalize());
            return CHIP_NO_ERROR;
        }

        CHIP_ERROR Type::Decode(TLV::TLVReader &reader) {
            CHIP_ERROR err;
            TLV::TLVType outer;

            err = reader.EnterContainer(outer);
            ReturnErrorOnFailure(err);

            while ((err = reader.Next()) == CHIP_NO_ERROR) {
                uint64_t tag = reader.GetTag();

                if (tag == TLV::ContextTag(kFieldIdJ)) {
                    ReturnErrorOnFailure(reader.Get(x));
                }
                else if (tag == TLV::ContextTag(kFieldIdK)) {
                    ReturnErrorOnFailure(reader.Get(y));
                }
                else if (tag == TLV::ContextTag(kFieldIdL)) {
                   const uint8_t *p;
                   ReturnErrorOnFailure(reader.GetDataPtr(p));
                   l = chip::ByteSpan(p, reader.GetLength());
                }
                else if (tag == TLV::ContextTag(kFieldIdM)) {
                   const uint8_t *p;
                   ReturnErrorOnFailure(reader.GetDataPtr(p));
                   m = chip::Span<char>((char *)p, reader.GetLength());
                }
            }

            ReturnErrorOnFailure(reader.ExitContainer(outer));
            return CHIP_NO_ERROR;
        }
    }

    namespace StructC {
        CHIP_ERROR Type::Encode(TLV::TLVWriter &writer, uint64_t tag) {
            TLV::TLVType outer;
            ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outer));
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kFieldIdA), a));
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kFieldIdB), b));
            ReturnErrorOnFailure(c.Encode(writer, TLV::ContextTag(kFieldIdC)));

            {
                TLV::TLVType outer1;
                ReturnErrorOnFailure(writer.StartContainer(TLV::ContextTag(kFieldIdD), TLV::kTLVType_Array, outer1));
                for (auto item : d) {
                    ReturnErrorOnFailure(writer.Put(TLV::AnonymousTag, item));
                }
                ReturnErrorOnFailure(writer.EndContainer(outer1));
            }

            {
                TLV::TLVType outer1;
                ReturnErrorOnFailure(writer.StartContainer(TLV::ContextTag(kFieldIdE), TLV::kTLVType_Array, outer1));
                for (auto item : e) {
                    ReturnErrorOnFailure(item.Encode(writer, TLV::AnonymousTag));
                }
                ReturnErrorOnFailure(writer.EndContainer(outer1));
            }

            ReturnErrorOnFailure(writer.EndContainer(outer));
            ReturnErrorOnFailure(writer.Finalize());
            return CHIP_NO_ERROR;
        }

        CHIP_ERROR Type::Decode(TLV::TLVReader &reader) {
            CHIP_ERROR err;
            TLV::TLVType outer;

            err = reader.EnterContainer(outer);
            ReturnErrorOnFailure(err);

            while ((err = reader.Next()) == CHIP_NO_ERROR) {
                uint64_t tag = reader.GetTag();

                if (tag == TLV::ContextTag(kFieldIdA)) {
                    ReturnErrorOnFailure(reader.Get(a));
                }
                else if (tag == TLV::ContextTag(kFieldIdB)) {
                    ReturnErrorOnFailure(reader.Get(b));
                }
                else if (tag == TLV::ContextTag(kFieldIdC)) {
                    ReturnErrorOnFailure(c.Decode(reader));
                }
                else if (tag == TLV::ContextTag(kFieldIdD)) {
                    TLV::TLVType outer1;
                    size_t destBufSize = d.size();
                    uint32_t i = 0;

                    ReturnErrorOnFailure(reader.EnterContainer(outer1));

                    while ((err = reader.Next()) == CHIP_NO_ERROR) {
                        VerifyOrReturnError(destBufSize, CHIP_ERROR_BUFFER_TOO_SMALL);
                        ReturnErrorOnFailure(reader.Get(d.data()[i]));
                    }

                    ReturnErrorOnFailure(reader.ExitContainer(outer1));
                }
                else if (tag == TLV::ContextTag(kFieldIdE)) {
                    TLV::TLVType outer1;
                    size_t destBufSize = e.size();
                    uint32_t i = 0;

                    ReturnErrorOnFailure(reader.EnterContainer(outer1));

                    while ((err = reader.Next()) == CHIP_NO_ERROR) {
                        VerifyOrReturnError(destBufSize, CHIP_ERROR_BUFFER_TOO_SMALL);
                        ReturnErrorOnFailure(e.data()[i].Decode(reader));
                    }

                    ReturnErrorOnFailure(reader.ExitContainer(outer1));
                }
            }

            ReturnErrorOnFailure(reader.ExitContainer(outer));
            return CHIP_NO_ERROR;
        }
    }

    namespace IteratableStructC {
        CHIP_ERROR Type::Encode(TLV::TLVWriter &writer, uint64_t tag) {
            return CHIP_ERROR_BAD_REQUEST;
        }

        CHIP_ERROR Type::Decode(TLV::TLVReader &reader) {
            CHIP_ERROR err;
            TLV::TLVType outer;

            err = reader.EnterContainer(outer);
            ReturnErrorOnFailure(err);

            while ((err = reader.Next()) == CHIP_NO_ERROR) {
                uint64_t tag = reader.GetTag();

                if (tag == TLV::ContextTag(kFieldIdA)) {
                    ReturnErrorOnFailure(reader.Get(a));
                }
                else if (tag == TLV::ContextTag(kFieldIdB)) {
                    ReturnErrorOnFailure(reader.Get(b));
                }
                else if (tag == TLV::ContextTag(kFieldIdC)) {
                    ReturnErrorOnFailure(c.Decode(reader));
                }
                else if (tag == TLV::ContextTag(kFieldIdD)) {
                    d.SetReader(reader);
                }
                else if (tag == TLV::ContextTag(kFieldIdE)) {
                    e.SetReader(reader);
                }
            }

            ReturnErrorOnFailure(reader.ExitContainer(outer));
            return CHIP_NO_ERROR;
        }
    }
}
}
}
}
