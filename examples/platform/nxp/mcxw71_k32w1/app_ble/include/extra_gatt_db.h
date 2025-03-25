PRIMARY_SERVICE_UUID128(service_wireless_uart, uuid_service_wireless_uart)
CHARACTERISTIC_UUID128(char_uart_stream, uuid_uart_stream, (gGattCharPropWriteWithoutRsp_c))
VALUE_UUID128_VARLEN(value_uart_stream, uuid_uart_stream, (gPermissionFlagWritable_c), gAttMaxWriteDataSize_d(gAttMaxMtu_c), 1,
                     0x00)
