package chip.onboardingpayload

class OptionalQRCodeInfo {
  enum class OptionalQRCodeInfoType {
    TYPE_UNKNOWN,
    TYPE_STRING,
    TYPE_INT32,
    TYPE_INT64,
    TYPE_UINT32,
    TYPE_UINT64
  }

  var tag = 0
  var type: OptionalQRCodeInfoType? = null
  var data: String? = null
  var int32 = 0
}
