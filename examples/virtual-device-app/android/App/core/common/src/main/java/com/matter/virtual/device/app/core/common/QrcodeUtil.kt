package com.matter.virtual.device.app.core.common

import android.graphics.Bitmap
import android.graphics.Color
import com.google.zxing.BarcodeFormat
import com.google.zxing.qrcode.QRCodeWriter

object QrcodeUtil {

  fun createQrCodeBitmap(contents: String, width: Int, height: Int): Bitmap? {
    try {
      val bitMatrix = QRCodeWriter().encode(contents, BarcodeFormat.QR_CODE, width, height)
      val bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888)

      for (y in 0 until bitMatrix.height) {
        for (x in 0 until bitMatrix.width) {
          if (bitMatrix.get(x, y)) {
            bitmap.setPixel(x, y, Color.BLACK)
          }
        }
      }

      return bitmap
    } catch (e: Exception) {
      e.printStackTrace()
      return null
    }
  }
}
