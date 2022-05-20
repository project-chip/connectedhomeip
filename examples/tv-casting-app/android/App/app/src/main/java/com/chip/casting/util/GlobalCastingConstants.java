package com.chip.casting.util;

import java.util.Arrays;
import java.util.List;

public class GlobalCastingConstants {
  public static final String CommissionerServiceType = "_matterd._udp.";
  public static final int CommissioningWindowDurationSecs = 3 * 60;
  public static int SetupPasscode = 20202021;
  public static int Discriminator = 0xF00;
  public static List<Long> CommissionerDeviceTypeFilter = Arrays.asList(35L); // Video player = 35
}
