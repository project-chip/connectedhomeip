#include "cluster-objects.h"
#include "lib/core/CHIPTLVTags.h"

#pragma GCC diagnostic ignored "-Wstack-usage="

namespace chip {
namespace app {
namespace clusters {
namespace TestCluster {
namespace TestListStructOctet {
CHIP_ERROR Type::Encode(TLV::TLVWriter & writer, uint64_t tag) const
{
    TLV::TLVType outer;

    ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outer));

    {
        ReturnErrorOnFailure(DataModel::Encode(writer, TLV::ContextTag(kFabricIndexFieldId), fabricIndex));
        ReturnErrorOnFailure(DataModel::Encode(writer, TLV::ContextTag(kOperationalCertFieldId), operationalCert));
    }

    ReturnErrorOnFailure(writer.EndContainer(outer));

    return CHIP_NO_ERROR;
}

CHIP_ERROR Type::Decode(TLV::TLVReader & reader)
{
    CHIP_ERROR err;
    TLV::TLVType outer;

    err = reader.EnterContainer(outer);
    ReturnErrorOnFailure(err);

    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        uint64_t tag = reader.GetTag();

        if (tag == TLV::ContextTag(kFabricIndexFieldId))
        {
            ReturnErrorOnFailure(DataModel::Decode(reader, fabricIndex));
        }
        else if (tag == TLV::ContextTag(kOperationalCertFieldId))
        {
            ReturnErrorOnFailure(DataModel::Decode(reader, operationalCert));
        }
    }

    ReturnErrorOnFailure(reader.ExitContainer(outer));
    return CHIP_NO_ERROR;
}
} // namespace TestListStructOctet

namespace SimpleStruct {
CHIP_ERROR Type::Encode(TLV::TLVWriter & writer, uint64_t tag) const
{
    TLV::TLVType outer;

    ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outer));

    {
        ReturnErrorOnFailure(DataModel::Encode(writer, TLV::ContextTag(kAFieldId), a));
        ReturnErrorOnFailure(DataModel::Encode(writer, TLV::ContextTag(kBFieldId), b));
        ReturnErrorOnFailure(DataModel::Encode(writer, TLV::ContextTag(kCFieldId), c));
        ReturnErrorOnFailure(DataModel::Encode(writer, TLV::ContextTag(kDFieldId), d));
        ReturnErrorOnFailure(DataModel::Encode(writer, TLV::ContextTag(kEFieldId), e));
    }

    ReturnErrorOnFailure(writer.EndContainer(outer));

    return CHIP_NO_ERROR;
}

CHIP_ERROR Type::Decode(TLV::TLVReader & reader)
{
    CHIP_ERROR err;
    TLV::TLVType outer;

    err = reader.EnterContainer(outer);
    ReturnErrorOnFailure(err);

    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        uint64_t tag = reader.GetTag();

        if (tag == TLV::ContextTag(kAFieldId))
        {
            ReturnErrorOnFailure(DataModel::Decode(reader, a));
        }
        else if (tag == TLV::ContextTag(kBFieldId))
        {
            ReturnErrorOnFailure(DataModel::Decode(reader, b));
        }
        else if (tag == TLV::ContextTag(kCFieldId))
        {
            uint8_t v;
            ReturnErrorOnFailure(DataModel::Decode(reader, v));
            c = (SimpleEnum) v;
        }
        else if (tag == TLV::ContextTag(kDFieldId))
        {
            ReturnErrorOnFailure(DataModel::Decode(reader, d));
        }
        else if (tag == TLV::ContextTag(kEFieldId))
        {
            ReturnErrorOnFailure(DataModel::Decode(reader, e));
        }
    }

    ReturnErrorOnFailure(reader.ExitContainer(outer));
    return CHIP_NO_ERROR;
}
} // namespace SimpleStruct

namespace NestedStruct {
CHIP_ERROR Type::Encode(TLV::TLVWriter & writer, uint64_t tag) const
{
    TLV::TLVType outer;

    ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outer));

    {
        ReturnErrorOnFailure(DataModel::Encode(writer, TLV::ContextTag(kAFieldId), a));
        ReturnErrorOnFailure(DataModel::Encode(writer, TLV::ContextTag(kBFieldId), b));
        ReturnErrorOnFailure(DataModel::Encode(writer, TLV::ContextTag(kCFieldId), c));
    }

    ReturnErrorOnFailure(writer.EndContainer(outer));

    return CHIP_NO_ERROR;
}

CHIP_ERROR Type::Decode(TLV::TLVReader & reader)
{
    CHIP_ERROR err;
    TLV::TLVType outer;

    err = reader.EnterContainer(outer);
    ReturnErrorOnFailure(err);

    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        uint64_t tag = reader.GetTag();

        if (tag == TLV::ContextTag(kAFieldId))
        {
            ReturnErrorOnFailure(DataModel::Decode(reader, a));
        }
        else if (tag == TLV::ContextTag(kBFieldId))
        {
            ReturnErrorOnFailure(DataModel::Decode(reader, b));
        }
        else if (tag == TLV::ContextTag(kCFieldId))
        {
            ReturnErrorOnFailure(DataModel::Decode(reader, c));
        }
    }

    ReturnErrorOnFailure(reader.ExitContainer(outer));
    return CHIP_NO_ERROR;
}
} // namespace NestedStruct

namespace NestedStructList {
CHIP_ERROR Type::Encode(TLV::TLVWriter & writer, uint64_t tag) const
{
    TLV::TLVType outer;

    ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outer));

    {
        ReturnErrorOnFailure(DataModel::Encode(writer, TLV::ContextTag(kAFieldId), a));
        ReturnErrorOnFailure(DataModel::Encode(writer, TLV::ContextTag(kBFieldId), b));
        ReturnErrorOnFailure(DataModel::Encode(writer, TLV::ContextTag(kCFieldId), c));
        ReturnErrorOnFailure(DataModel::Encode(writer, TLV::ContextTag(kDFieldId), d));
        ReturnErrorOnFailure(DataModel::Encode(writer, TLV::ContextTag(kEFieldId), e));
        ReturnErrorOnFailure(DataModel::Encode(writer, TLV::ContextTag(kFFieldId), f));
        ReturnErrorOnFailure(DataModel::Encode(writer, TLV::ContextTag(kGFieldId), g));
    }

    ReturnErrorOnFailure(writer.EndContainer(outer));

    return CHIP_NO_ERROR;
}

CHIP_ERROR Type::Decode(TLV::TLVReader & reader)
{
    CHIP_ERROR err;
    TLV::TLVType outer;

    err = reader.EnterContainer(outer);
    ReturnErrorOnFailure(err);

    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        uint64_t tag = reader.GetTag();

        if (tag == TLV::ContextTag(kAFieldId))
        {
            ReturnErrorOnFailure(DataModel::Decode(reader, a));
        }
        else if (tag == TLV::ContextTag(kBFieldId))
        {
            ReturnErrorOnFailure(DataModel::Decode(reader, b));
        }
        else if (tag == TLV::ContextTag(kCFieldId))
        {
            ReturnErrorOnFailure(DataModel::Decode(reader, c));
        }
        else if (tag == TLV::ContextTag(kDFieldId))
        {
            ReturnErrorOnFailure(DataModel::Decode(reader, d));
        }
        else if (tag == TLV::ContextTag(kEFieldId))
        {
            ReturnErrorOnFailure(DataModel::Decode(reader, e));
        }
        else if (tag == TLV::ContextTag(kFFieldId))
        {
            ReturnErrorOnFailure(DataModel::Decode(reader, f));
        }
    }

    ReturnErrorOnFailure(reader.ExitContainer(outer));
    return CHIP_NO_ERROR;
}
} // namespace NestedStructList

namespace DecodableNestedStructList {
CHIP_ERROR Type::Encode(TLV::TLVWriter & writer, uint64_t tag) const
{
    return CHIP_ERROR_BAD_REQUEST;
}

CHIP_ERROR Type::Decode(TLV::TLVReader & reader)
{
    CHIP_ERROR err;
    TLV::TLVType outer;

    err = reader.EnterContainer(outer);
    ReturnErrorOnFailure(err);

    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        uint64_t tag = reader.GetTag();

        if (tag == TLV::ContextTag(kAFieldId))
        {
            ReturnErrorOnFailure(DataModel::Decode(reader, a));
        }
        else if (tag == TLV::ContextTag(kBFieldId))
        {
            ReturnErrorOnFailure(DataModel::Decode(reader, b));
        }
        else if (tag == TLV::ContextTag(kCFieldId))
        {
            ReturnErrorOnFailure(DataModel::Decode(reader, c));
        }
        else if (tag == TLV::ContextTag(kDFieldId))
        {
            ReturnErrorOnFailure(DataModel::Decode(reader, d));
        }
        else if (tag == TLV::ContextTag(kEFieldId))
        {
            ReturnErrorOnFailure(DataModel::Decode(reader, e));
        }
        else if (tag == TLV::ContextTag(kFFieldId))
        {
            ReturnErrorOnFailure(DataModel::Decode(reader, f));
        }
        else if (tag == TLV::ContextTag(kGFieldId))
        {
            ReturnErrorOnFailure(DataModel::Decode(reader, g));
        }
    }

    ReturnErrorOnFailure(reader.ExitContainer(outer));
    return CHIP_NO_ERROR;
}
} // namespace DecodableNestedStructList

namespace DoubleNestedStructList {
CHIP_ERROR Type::Encode(TLV::TLVWriter & writer, uint64_t tag) const
{
    TLV::TLVType outer;

    ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outer));

    {
        ReturnErrorOnFailure(DataModel::Encode(writer, TLV::ContextTag(kAFieldId), a));
    }

    ReturnErrorOnFailure(writer.EndContainer(outer));

    return CHIP_NO_ERROR;
}

CHIP_ERROR Type::Decode(TLV::TLVReader & reader)
{
    CHIP_ERROR err;
    TLV::TLVType outer;

    err = reader.EnterContainer(outer);
    ReturnErrorOnFailure(err);

    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        uint64_t tag = reader.GetTag();

        if (tag == TLV::ContextTag(kAFieldId))
        {
            ReturnErrorOnFailure(DataModel::Decode(reader, a));
        }
    }

    ReturnErrorOnFailure(reader.ExitContainer(outer));
    return CHIP_NO_ERROR;
}
} // namespace DoubleNestedStructList

namespace DecodableDoubleNestedStructList {
CHIP_ERROR Type::Encode(TLV::TLVWriter & writer, uint64_t tag) const
{
    return CHIP_ERROR_BAD_REQUEST;
}

CHIP_ERROR Type::Decode(TLV::TLVReader & reader)
{
    CHIP_ERROR err;
    TLV::TLVType outer;

    err = reader.EnterContainer(outer);
    ReturnErrorOnFailure(err);

    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        uint64_t tag = reader.GetTag();

        if (tag == TLV::ContextTag(kAFieldId))
        {
            ReturnErrorOnFailure(DataModel::Decode(reader, a));
        }
    }

    ReturnErrorOnFailure(reader.ExitContainer(outer));
    return CHIP_NO_ERROR;
}
} // namespace DecodableDoubleNestedStructList
} // namespace TestCluster
} // namespace clusters
} // namespace app
} // namespace chip
