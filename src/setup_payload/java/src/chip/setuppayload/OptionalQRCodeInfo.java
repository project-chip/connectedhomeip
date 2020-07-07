package chip.setuppayload;

public class OptionalQRCodeInfo
{
    enum optionalQRCodeInfoType {
       optionalQRCodeInfoTypeUnknown,
       optionalQRCodeInfoTypeString,
       optionalQRCodeInfoTypeInt32,
       optionalQRCodeInfoTypeInt64,
       optionalQRCodeInfoTypeUInt32,
       optionalQRCodeInfoTypeUInt64
    };

    public int tag;
    public optionalQRCodeInfoType type;
    public String data;
    public int int32;
};
