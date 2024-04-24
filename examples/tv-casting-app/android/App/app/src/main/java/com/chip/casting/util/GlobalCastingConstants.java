package com.chip.casting.util;

public class GlobalCastingConstants {
  public static final String CommissionerServiceType = "_matterd._udp.";
  public static final int CommissioningWindowDurationSecs = 3 * 60;
  public static final int SetupPasscode = 20202021;
  public static final int Discriminator = 0xF00;
  public static final boolean ChipCastingSimplified =
      true; // set to true, to demo the simplified casting APIs. Otherwise, the older deprecated
  // APIs are invoked
}
