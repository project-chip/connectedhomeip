/*
 *   Copyright (c) 2021 Project CHIP Authors
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
 *
 */

package com.matter.tv.server.tvapp;

public interface ContentLaunchManager {

  int STREAMING_TYPE_DASH = 1;
  int STREAMING_TYPE_HLS = 2;

  /**
   * @return The list of content types supported by the Video Player or Content App in the form of
   *     entries in the HTTP "Accept" request header.
   */
  String[] getAcceptHeader();

  /** @return The list information about supported streaming protocols in STREAMING_TYPE_XXX. */
  long getSupportedStreamingProtocols();

  /**
   * Launch the specified content with optional search criteria.
   *
   * @param search search list for content for display or playback.
   * @param autoplay whether to automatically start playing content.
   * @param data Optional app-specific data.
   */
  ContentLaunchResponse launchContent(
      ContentLaunchSearchParameter[] search, boolean autoplay, String data);

  /**
   * Launch content from the specified URL.
   *
   * <p>The content types supported include those identified in the AcceptHeader and
   * SupportedStreamingProtocols attributes.
   *
   * @param url The URL of content to launch.
   * @param display If present, SHALL provide a string that MAY be used to describe the content
   *     being accessed at the given URL.
   * @param branding If present, SHALL indicate the BrandingInformation that MAY be displayed when
   *     playing back the given content.
   */
  ContentLaunchResponse launchUrl(
      String url, String display, ContentLaunchBrandingInformation branding);
}
