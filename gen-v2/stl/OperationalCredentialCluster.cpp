#include "OperationalCredentialCluster-Gen.h"
#include "core/CHIPTLVTags.h"
#include "core/CHIPTLVTypes.h"

#pragma GCC diagnostic ignored "-Wstack-usage="

namespace chip {
namespace app {
namespace Cluster {
namespace OperationalCredentialCluster {
    namespace AddOpCert {
        CHIP_ERROR Type::Encode(TLV::TLVWriter &writer, uint64_t tag) {
            TLV::TLVType outer;
            ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outer));
            ReturnErrorOnFailure(writer.PutBytes(TLV::ContextTag(kFieldIdNoc), &noc[0], noc.size()));
            ReturnErrorOnFailure(writer.PutBytes(TLV::ContextTag(kFieldIdIpkValue), &ipkValue[0], ipkValue.size()));
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kFieldIdCaseAdminNode), caseAdminNode));
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kFieldIdAdminVendorId), adminVendorId));
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

                if (tag == TLV::ContextTag(kFieldIdNoc)) {
                   noc.resize(reader.GetLength());
                   ReturnErrorOnFailure(reader.GetBytes(noc.data(), reader.GetLength()));
                }
                else if (tag == TLV::ContextTag(kFieldIdIpkValue)) {
                   ipkValue.resize(reader.GetLength());
                   ReturnErrorOnFailure(reader.GetBytes(ipkValue.data(), reader.GetLength()));
                }
                else if (tag == TLV::ContextTag(kFieldIdCaseAdminNode)) {
                    ReturnErrorOnFailure(reader.Get(caseAdminNode));
                }
                else if (tag == TLV::ContextTag(kFieldIdAdminVendorId)) {
                    ReturnErrorOnFailure(reader.Get(adminVendorId));
                }
            }

            ReturnErrorOnFailure(reader.ExitContainer(outer));
            return CHIP_NO_ERROR;
        }
    }

    namespace OpCertResponse {
        CHIP_ERROR Type::Encode(TLV::TLVWriter &writer, uint64_t tag) {
            TLV::TLVType outer;
            ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outer));
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kFieldIdStatusCode), statusCode));
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kFieldIdFabricIndex), fabricIndex));
            ReturnErrorOnFailure(writer.PutString(TLV::ContextTag(kFieldIdDebugText), debugText.c_str(), debugText.length()));
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


                if (tag == TLV::ContextTag(kFieldIdStatusCode)) {
                    ReturnErrorOnFailure(reader.Get(statusCode));
                }
                else if (tag == TLV::ContextTag(kFieldIdFabricIndex)) {
                    ReturnErrorOnFailure(reader.Get(fabricIndex));
                }
                else if (tag == TLV::ContextTag(kFieldIdDebugText)) {
                   int len = reader.GetLength() + 1;
                   char tmp[len];
                   ReturnErrorOnFailure(reader.GetString(tmp, len + 1));
                   debugText = std::string(tmp, len + 1);
                   debugText.resize(len - 1);
                }
            }

            ReturnErrorOnFailure(reader.ExitContainer(outer));
            return CHIP_NO_ERROR;
        }
    }

    namespace OpCsrRequest {
        CHIP_ERROR Type::Encode(TLV::TLVWriter &writer, uint64_t tag) {
            TLV::TLVType outer;
            ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outer));
            ReturnErrorOnFailure(writer.PutBytes(TLV::ContextTag(kFieldIdCsrNonce), &csrNonce[0], csrNonce.size()));
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

                if (tag == TLV::ContextTag(kFieldIdCsrNonce)) {
                   csrNonce.resize(reader.GetLength());
                   ReturnErrorOnFailure(reader.GetBytes(csrNonce.data(), reader.GetLength()));
                }
            }

            ReturnErrorOnFailure(reader.ExitContainer(outer));
            return CHIP_NO_ERROR;
        }
    }

    namespace OpCsrResponse {
        CHIP_ERROR Type::Encode(TLV::TLVWriter &writer, uint64_t tag) {
            TLV::TLVType outer;
            ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outer));
            ReturnErrorOnFailure(writer.PutBytes(TLV::ContextTag(kFieldIdCsr), &csr[0], csr.size()));
            ReturnErrorOnFailure(writer.PutBytes(TLV::ContextTag(kFieldIdCsrNonce), &csrNonce[0], csrNonce.size()));
            ReturnErrorOnFailure(writer.PutBytes(TLV::ContextTag(kFieldIdVendorReserved1), &vendorReserved1[0], vendorReserved1.size()));
            ReturnErrorOnFailure(writer.PutBytes(TLV::ContextTag(kFieldIdVendorReserved2), &vendorReserved2[0], vendorReserved2.size()));
            ReturnErrorOnFailure(writer.PutBytes(TLV::ContextTag(kFieldIdVendorReserved3), &vendorReserved3[0], vendorReserved3.size()));
            ReturnErrorOnFailure(writer.PutBytes(TLV::ContextTag(kFieldIdSignature), &signature[0], signature.size()));
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

                if (tag == TLV::ContextTag(kFieldIdCsr)) {
                   csr.resize(reader.GetLength());
                   ReturnErrorOnFailure(reader.GetBytes(csr.data(), reader.GetLength()));
                }
                else if (tag == TLV::ContextTag(kFieldIdCsrNonce)) {
                   csrNonce.resize(reader.GetLength());
                   ReturnErrorOnFailure(reader.GetBytes(csrNonce.data(), reader.GetLength()));
                }
                else if (tag == TLV::ContextTag(kFieldIdVendorReserved1)) {
                   vendorReserved1.resize(reader.GetLength());
                   ReturnErrorOnFailure(reader.GetBytes(vendorReserved1.data(), reader.GetLength()));
                }
                else if (tag == TLV::ContextTag(kFieldIdVendorReserved2)) {
                   vendorReserved2.resize(reader.GetLength());
                   ReturnErrorOnFailure(reader.GetBytes(vendorReserved2.data(), reader.GetLength()));
                }
                else if (tag == TLV::ContextTag(kFieldIdVendorReserved3)) {
                   vendorReserved3.resize(reader.GetLength());
                   ReturnErrorOnFailure(reader.GetBytes(vendorReserved3.data(), reader.GetLength()));
                }
                else if (tag == TLV::ContextTag(kFieldIdSignature)) {
                   signature.resize(reader.GetLength());
                   ReturnErrorOnFailure(reader.GetBytes(signature.data(), reader.GetLength()));
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
