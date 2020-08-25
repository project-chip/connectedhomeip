package chip.setuppayload;

public class OptionalQRCodeInfo {
  public enum OptionalQRCodeInfoType {
    TYPE_UNKNOWN,
    TYPE_STRING,
    TYPE_INT32,
    TYPE_INT64,
    TYPE_UINT32,
    TYPE_UINT64
  };

  public int tag;
  public OptionalQRCodeInfoType type;
  public String data;
  public int int32;
};
