package com.matter.tv.app.api;

public class Clusters {
  // Clusters
  public static class MediaPlayback {
    public static final int Id = 1286;

    public static class Attributes {
      public static final int CurrentState = 0;
    }

    public static class PlaybackStateEnum {
      public static final int Playing = 0;
      public static final int Paused = 1;
      public static final int NotPlaying = 2;
      public static final int Buffering = 3;
    }
  }

  public static class ContentLauncher {
    public static final int Id = 1290;

    public static class Attributes {
      public static final int AcceptHeader = 0;
      public static final int SupportedStreamingProtocols = 1;
    }
  }
}
