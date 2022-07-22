package com.matter.tv.server;

import android.graphics.Bitmap;
import android.graphics.Color;
import com.google.zxing.BarcodeFormat;
import com.google.zxing.common.BitMatrix;
import com.google.zxing.qrcode.QRCodeWriter;

public class QRUtils {
  public static Bitmap createQRCodeBitmap(String content, int width, int height) {
    try {
      BitMatrix bitMatrix =
          new QRCodeWriter().encode(content, BarcodeFormat.QR_CODE, width, height);
      Bitmap bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);

      for (int y = 0; y < bitMatrix.getHeight(); y++) {
        for (int x = 0; x < bitMatrix.getWidth(); x++) {
          if (bitMatrix.get(x, y)) {
            bitmap.setPixel(x, y, Color.BLACK);
          }
        }
      }
      return bitmap;
    } catch (Exception e) {
      e.printStackTrace();
      return null;
    }
  }
}
