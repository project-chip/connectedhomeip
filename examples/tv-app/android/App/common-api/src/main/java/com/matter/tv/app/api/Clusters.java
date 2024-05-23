package com.matter.tv.app.api;

/**
 * Helper class to hold the IDs and corresponding constants for media related clusters. TODO : Add
 * the rest of the media clusters TODO : Maybe generate using ZAP tool
 */
public class Clusters {

  // Clusters
  public static class AccountLogin {
    public static final int Id = 0x050E;

    public static class Commands {
      public static class GetSetupPIN {
        public static final int ID = 0x00;

        public static class Fields {
          public static final int TempAccountIdentifier = 0x00;
        }
      }

      public static class GetSetupPINResponse {
        public static final int ID = 0x01;

        public static class Fields {
          public static final int SetupPIN = 0x00;
        }
      }

      public static class Login {
        public static final int ID = 0x02;
      }

      public static class Logout {
        public static final int ID = 0x03;
      }
    }
  }

  public static class MediaPlayback {
    public static final int Id = 0x0506;

    public static class Commands {
      public static class Play {
        public static final int ID = 0x00;
      }

      public static class Pause {
        public static final int ID = 0x01;
      }

      public static class StopPlayback {
        public static final int ID = 0x02;
      }

      public static class StartOver {
        public static final int ID = 0x03;
      }

      public static class Previous {
        public static final int ID = 0x04;
      }

      public static class Next {
        public static final int ID = 0x05;
      }

      public static class Rewind {
        public static final int ID = 0x06;
      }

      public static class FastForward {
        public static final int ID = 0x07;
      }

      public static class SkipForward {
        public static final int ID = 0x08;

        public static class Fields {
          public static final int DeltaPositionMilliseconds = 0x00;
        }
      }

      public static class SkipBackward {
        public static final int ID = 0x09;

        public static class Fields {
          public static final int DeltaPositionMilliseconds = 0x00;
        }
      }

      public static class Seek {
        public static final int ID = 0x0B;

        public static class Fields {
          public static final int Position = 0x00;
        }
      }

      public static class PlaybackResponse {
        public static final int ID = 0x0A;

        public static class Fields {
          public static final int Status = 0x00;
          public static final int Data = 0x01;
        }
      }
    }

    public static class Attributes {
      public static final int CurrentState = 0x00;
      public static final int StartTime = 0x01;
      public static final int Duration = 0x02;
      public static final int SampledPosition = 0x03;
      public static final int PlaybackSpeed = 0x04;
      public static final int SeekRangeEnd = 0x05;
      public static final int SeekRangeStart = 0x06;
    }

    public static class Types {
      public static class PlaybackStateEnum {
        public static final int Playing = 0x00;
        public static final int Paused = 0x01;
        public static final int NotPlaying = 0x02;
        public static final int Buffering = 0x03;
      }

      public static class StatusEnum {
        public static final int Success = 0x00;
        public static final int InvalidStateForCommand = 0x01;
        public static final int NotAllowed = 0x02;
        public static final int NotActive = 0x03;
        public static final int SpeedOutOfRange = 0x04;
        public static final int SeekOutOfRange = 0x05;
      }

      public static class PlaybackPosition {
        public static final int UpdatedAt = 0x00;
        public static final int Position = 0x01;
      }
    }
  }

  public static class ContentLauncher {
    public static final int Id = 0x050A;

    public static class Commands {
      public static class LaunchContent {
        public static final int ID = 0x00;

        public static class Fields {
          public static final int Search = 0x00;
          public static final int AutoPlay = 0x01;
          public static final int Data = 0x02;
        }
      }

      public static class LaunchURL {
        public static final int ID = 0x01;

        public static class Fields {
          public static final int ContentURL = 0x00;
          public static final int DisplayString = 0x01;
          public static final int BrandingInformation = 0x02;
        }
      }

      public static class LaunchResponse {
        public static final int ID = 0x02;

        public static class Fields {
          public static final int Status = 0x00;
          public static final int Data = 0x01;
        }
      }
    }

    public static class Attributes {
      public static final int AcceptHeader = 0x00;
      public static final int SupportedStreamingProtocols = 0x01;
    }

    public static class Types {
      public static class ContentSearch {
        public static final int ParameterList = 0x00;
      }

      public static class StatusEnum {
        public static final int Success = 0x00;
        public static final int UrlNotAvailable = 0x01;
        public static final int AuthFailed = 0x02;
      }

      public static class Parameter {
        public static final int Type = 0x00;
        public static final int Value = 0x01;
        public static final int ExternalIDList = 0x02;
      }

      public static class ParameterEnum {
        public static final int Actor = 0x00;
        public static final int Channel = 0x01;
        public static final int Character = 0x02;
        public static final int Director = 0x03;
        public static final int Event = 0x04;
        public static final int Franchise = 0x05;
        public static final int Genre = 0x06;
        public static final int League = 0x07;
        public static final int Popularity = 0x08;
        public static final int Provider = 0x09;
        public static final int Sport = 0x0A;
        public static final int SportsTeam = 0x0B;
        public static final int Type = 0x0C;
        public static final int Video = 0x0D;
      }

      public static class AdditionalInfo {
        public static final int Name = 0x00;
        public static final int Value = 0x01;
      }

      public static class BrandingInformation {
        public static final int ProviderName = 0x00;
        public static final int Background = 0x01;
        public static final int Logo = 0x02;
        public static final int ProgressBar = 0x03;
        public static final int Splash = 0x04;
        public static final int WaterMark = 0x05;
      }

      public static class StyleInformation {
        public static final int ProviderName = 0x00;
        public static final int Background = 0x01;
        public static final int Logo = 0x02;
      }

      public static class Dimension {
        public static final int ImageUrl = 0x00;
        public static final int Color = 0x01;
        public static final int Size = 0x02;
      }

      public static class MetricTypeEnum {
        public static final int Pixels = 0x00;
        public static final int Percentage = 0x01;
      }
    }
  }

  public static class TargetNavigator {
    public static final int Id = 0x0505;

    public static class Commands {
      public static class NavigateTarget {
        public static final int ID = 0x00;

        public static class Fields {
          public static final int Target = 0x00;
          public static final int Data = 0x01;
        }
      }

      public static class NavigateTargetResponse {
        public static final int ID = 0x01;

        public static class Fields {
          public static final int Status = 0x00;
          public static final int Data = 0x01;
        }
      }
    }

    public static class Attributes {
      public static final int TargetList = 0x00;
      public static final int CurrentTarget = 0x01;
    }

    public static class Types {
      public static class TargetInfo {
        public static final int Identifier = 0x00;
        public static final int Name = 0x01;
      }

      public static class StatusEnum {
        public static final int Success = 0x00;
        public static final int TargetNotFound = 0x01;
        public static final int NotAllowed = 0x02;
      }
    }
  }
}
