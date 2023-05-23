package chip.onboardingpayload

data class OptionalQRCodeInfo(
  var tag: Int = 0,
  var type: OptionalQRCodeInfoType = OptionalQRCodeInfoType.TYPE_UNKNOWN,
  var data: String? = null,
  var int32: Int = 0
) {
  enum class OptionalQRCodeInfoType {
    TYPE_UNKNOWN,
    TYPE_STRING,
    TYPE_INT32,
    TYPE_INT64,
    TYPE_UINT32,
    TYPE_UINT64
  }
}
