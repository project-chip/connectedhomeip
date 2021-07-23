#include "TestCluster-Gen.h"
#include "core/CHIPTLVTags.h"
#include "core/CHIPTLVTypes.h"

#pragma GCC diagnostic ignored "-Wstack-usage="

namespace chip {
namespace app {
namespace Cluster {
namespace TestCluster {
    namespace StructA {
        CHIP_ERROR Type::Encode(TLV::TLVWriter &writer, uint64_t tag) {
            TLV::TLVType outer;
            ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outer));
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kFieldIdJ), x));
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kFieldIdK), y));
            ReturnErrorOnFailure(writer.PutBytes(TLV::ContextTag(kFieldIdL), &l[0], l.size()));
            ReturnErrorOnFailure(writer.PutString(TLV::ContextTag(kFieldIdM), m.c_str(), m.length()));
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
                   l.resize(reader.GetLength());
                   ReturnErrorOnFailure(reader.GetBytes(l.data(), reader.GetLength()));
                }
                else if (tag == TLV::ContextTag(kFieldIdM)) {
                   int len = reader.GetLength() + 1;
                   char tmp[len];
                   ReturnErrorOnFailure(reader.GetString(tmp, len + 1));
                   m = std::string(tmp, len + 1);
                   m.resize(len - 1);
                }
            }

            ReturnErrorOnFailure(reader.ExitContainer(outer));
            return CHIP_NO_ERROR;
        }
    }

    namespace StructB {
        CHIP_ERROR Type::Encode(TLV::TLVWriter &writer, uint64_t tag) {
            TLV::TLVType outer;
            ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outer));
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kFieldIdX), x));
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kFieldIdY), y));
            ReturnErrorOnFailure(z.Encode(writer, TLV::ContextTag(kFieldIdZ)));
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

                if (tag == TLV::ContextTag(kFieldIdX)) {
                    ReturnErrorOnFailure(reader.Get(x));
                }
                else if (tag == TLV::ContextTag(kFieldIdY)) {
                    ReturnErrorOnFailure(reader.Get(y));
                }
                else if (tag == TLV::ContextTag(kFieldIdZ)) {
                    ReturnErrorOnFailure(z.Decode(reader));
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
            ReturnErrorOnFailure(writer.PutBytes(TLV::ContextTag(kFieldIdD), &d[0], d.size()));

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
                    d.resize(reader.GetLength());
                    ReturnErrorOnFailure(reader.GetBytes(d.data(), reader.GetLength()));
                }
                else if (tag == TLV::ContextTag(kFieldIdE)) {
                    e.clear();

                    {
                        TLV::TLVType outer1;

                        ReturnErrorOnFailure(reader.EnterContainer(outer1));

                        while ((err = reader.Next()) == CHIP_NO_ERROR) {
                            StructA::Type tmp;

                            ReturnErrorOnFailure(tmp.Decode(reader));
                            e.push_back(tmp);
                        }

                        ReturnErrorOnFailure(reader.ExitContainer(outer1));
                    }
                }
            }

            ReturnErrorOnFailure(reader.ExitContainer(outer));
            return CHIP_NO_ERROR;
        }
    }

    namespace CommandA {
        CHIP_ERROR Type::Encode(TLV::TLVWriter &writer, uint64_t tag) {
            TLV::TLVType outer;
            ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outer));
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kFieldIdA), a));
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kFieldIdB), b));
            ReturnErrorOnFailure(c.Encode(writer, TLV::ContextTag(kFieldIdC)));
            ReturnErrorOnFailure(writer.PutBytes(TLV::ContextTag(kFieldIdD), &d[0], d.size()));
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
                    d.resize(reader.GetLength());
                    ReturnErrorOnFailure(reader.GetBytes(d.data(), reader.GetLength()));
                }
            }

            ReturnErrorOnFailure(reader.ExitContainer(outer));
            return CHIP_NO_ERROR;
        }
    }

    namespace CommandB {
        CHIP_ERROR Type::Encode(TLV::TLVWriter &writer, uint64_t tag) {
            TLV::TLVType outer;
            ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outer));
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kFieldIdA), a));
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kFieldIdB), b));
            ReturnErrorOnFailure(c.Encode(writer, TLV::ContextTag(kFieldIdC)));
            ReturnErrorOnFailure(writer.PutBytes(TLV::ContextTag(kFieldIdD), &d[0], d.size()));

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
                    d.resize(reader.GetLength());
                    ReturnErrorOnFailure(reader.GetBytes(d.data(), reader.GetLength()));
                }
                else if (tag == TLV::ContextTag(kFieldIdE)) {
                    e.clear();

                    {
                        TLV::TLVType outer1;

                        ReturnErrorOnFailure(reader.EnterContainer(outer1));

                        while ((err = reader.Next()) == CHIP_NO_ERROR) {
                            StructA::Type tmp;

                            ReturnErrorOnFailure(tmp.Decode(reader));
                            e.push_back(tmp);
                        }

                        ReturnErrorOnFailure(reader.ExitContainer(outer1));
                    }
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
