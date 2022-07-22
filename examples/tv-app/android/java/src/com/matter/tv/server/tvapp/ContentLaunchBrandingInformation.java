package com.matter.tv.server.tvapp;

public class ContentLaunchBrandingInformation {

  public class StyleInformation {
    /** Used for dimensions defined in a number of Pixels. */
    public static final int METRIC_PIXELS = 0;

    /**
     * Used for dimensions defined as a percentage of the overall display dimensions. For example,
     * if using a Percentage Metric type for a Width measurement of 50.0, against a display width of
     * 1920 pixels, then the resulting value used would be 960 pixels (50.0% of 1920) for that
     * dimension. Whenever a measurement uses this Metric type, the resulting values SHALL be
     * rounded ("floored") towards 0 if the measurement requires an integer final value.
     */
    public static final int METRIC_PERCENTAGE = 1;

    /**
     * The URL of image used for Styling different Video Player sections like Logo, Watermark etc.
     */
    public String imageUrl;

    /**
     * The color, in RGB or RGBA, used for styling different Video Player sections like Logo,
     * Watermark, etc. The value SHALL conform to the 6-digit or 8-digit format defined for CSS sRGB
     * hexadecimal color notation. Examples:
     *
     * <p>#76DE19 for R=0x76, G=0xDE, B=0x19, A absent #76DE1980 for R=0x76, G=0xDE, B=0x19, A=0x80
     */
    public String color;

    /** The width using the metric defined in Metric */
    public double width;

    /** The height using the metric defined in Metric */
    public double height;

    /** Tmetric used for defining Height/Width in METRIC_XXX */
    public int metric;
  }

  /** The name of of the provider for the given content. */
  public String providerName;

  /**
   * The background of the Video Player while content launch request is being processed by it. This
   * background information MAY also be used by the Video Player when it is in idle state.
   */
  public StyleInformation background;

  /** The style of progress bar for media playback. */
  public StyleInformation progressBar;

  /**
   * The screen shown when the Video Player is in an idle state. If this property is not populated,
   * the Video Player SHALL default to logo or the provider name.
   */
  public StyleInformation splash;

  /** The watermark shown when the media is playing. */
  public StyleInformation waterMark;
}
