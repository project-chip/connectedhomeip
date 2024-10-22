package com.chip.casting.util;

public class GlobalCastingConstants {
  public static final int CommissioningWindowDurationSecs = 3 * 60;
  public static final int SetupPasscode = 20202021;
  public static final int Discriminator = 0xF00;

  // set to true, to demo the simplified casting APIs.
  // Otherwise, the deprecated casting APIs are invoked
  public static final boolean ChipCastingSimplified = true;
}
