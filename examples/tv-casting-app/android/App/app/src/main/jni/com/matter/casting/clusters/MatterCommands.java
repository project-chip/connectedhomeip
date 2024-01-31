/*
 *   Copyright (c) 2024 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */
package com.matter.casting.clusters;

import com.matter.casting.core.MatterCommand;
import java.util.Optional;

public class MatterCommands {

  public static class ContentLauncherClusterDimensionStruct {
    public Double width;
    public Double height;
    public Integer metric;
  }

  public static class ContentLauncherClusterStyleInformationStruct {
    public Optional<String> imageURL;
    public Optional<String> color;
    public Optional<ContentLauncherClusterDimensionStruct> size;
  }

  public static class ContentLauncherClusterBrandingInformationStruct {
    public String providerName;
    public Optional<ContentLauncherClusterStyleInformationStruct> background;
    public Optional<ContentLauncherClusterStyleInformationStruct> logo;
    public Optional<ContentLauncherClusterStyleInformationStruct> progressBar;
    public Optional<ContentLauncherClusterStyleInformationStruct> splash;
    public Optional<ContentLauncherClusterStyleInformationStruct> waterMark;
  }

  public static class ContentLauncherClusterLaunchURLRequest {
    public String contentURL;
    public String displayString;
    Optional<ContentLauncherClusterBrandingInformationStruct> brandingInformation;
  }

  public static class ContentLauncherClusterResponse {
    public String data;
    public Integer status;
  }

  public static class ContentLauncherClusterLaunchURLCommand
      extends MatterCommand<
          ContentLauncherClusterLaunchURLRequest, ContentLauncherClusterResponse> {}
}
