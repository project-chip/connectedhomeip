/*
 *   Copyright (c) 2022 Project CHIP Authors
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
package com.chip.casting;

public class TvCastingApp {
  private static final String TAG = TvCastingApp.class.getSimpleName();

  public native void setDACProvider(DACProvider provider);

  public native boolean openBasicCommissioningWindow(
      int duration, Object commissioningCompleteHandler);

  public native boolean sendUserDirectedCommissioningRequest(String address, int port);

  public native boolean discoverCommissioners();

  public native void init();

  /*
   * CONTENT LAUNCHER CLUSTER
   *
   * TODO: Add API to subscribe to AcceptHeader
   */
  public native boolean contentLauncherLaunchURL(
      String contentUrl, String contentDisplayStr, Object launchURLHandler);

  public native boolean contentLauncher_launchContent(
      ContentLauncherTypes.ContentSearch search,
      boolean autoPlay,
      String data,
      Object responseHandler);

  public native boolean contentLauncher_subscribeToSupportedStreamingProtocols(
      SuccessCallback<Integer> readSuccessHandler,
      FailureCallback readFailureHandler,
      int minInterval,
      int maxInterval,
      SubscriptionEstablishedCallback subscriptionEstablishedHandler);

  /*
   * LEVEL CONTROL CLUSTER
   */
  public native boolean levelControl_step(
      byte stepMode,
      byte stepSize,
      short transitionTime,
      byte optionMask,
      byte optionOverridem,
      Object responseHandler);

  public native boolean levelControl_moveToLevel(
      byte level,
      short transitionTime,
      byte optionMask,
      byte optionOverridem,
      Object responseHandler);

  public native boolean levelControl_subscribeToCurrentLevel(
      SuccessCallback<Byte> readSuccessHandler,
      FailureCallback readFailureHandler,
      int minInterval,
      int maxInterval,
      SubscriptionEstablishedCallback subscriptionEstablishedHandler);

  public native boolean levelControl_subscribeToMinLevel(
      SuccessCallback<Byte> readSuccessHandler,
      FailureCallback readFailureHandler,
      int minInterval,
      int maxInterval,
      SubscriptionEstablishedCallback subscriptionEstablishedHandler);

  public native boolean levelControl_subscribeToMaxLevel(
      SuccessCallback<Byte> readSuccessHandler,
      FailureCallback readFailureHandler,
      int minInterval,
      int maxInterval,
      SubscriptionEstablishedCallback subscriptionEstablishedHandler);

  /*
   * MEDIA PLAYBACK CLUSTER
   */
  public native boolean mediaPlayback_play(Object responseHandler);

  public native boolean mediaPlayback_pause(Object responseHandler);

  public native boolean mediaPlayback_stopPlayback(Object responseHandler);

  public native boolean mediaPlayback_next(Object responseHandler);

  public native boolean mediaPlayback_seek(long position, Object responseHandler);

  public native boolean mediaPlayback_skipForward(
      long deltaPositionMilliseconds, Object responseHandler);

  public native boolean mediaPlayback_skipBackward(
      long deltaPositionMilliseconds, Object responseHandler);

  public native boolean mediaPlayback_subscribeToCurrentState(
      SuccessCallback<MediaPlaybackTypes.PlaybackStateEnum> readSuccessHandler,
      FailureCallback readFailureHandler,
      int minInterval,
      int maxInterval,
      SubscriptionEstablishedCallback subscriptionEstablishedHandler);

  public native boolean mediaPlayback_subscribeToDuration(
      SuccessCallback<Long> readSuccessHandler,
      FailureCallback readFailureHandler,
      int minInterval,
      int maxInterval,
      SubscriptionEstablishedCallback subscriptionEstablishedHandler);

  public native boolean mediaPlayback_subscribeToSampledPosition(
      SuccessCallback<MediaPlaybackTypes.PlaybackPosition> readSuccessHandler,
      FailureCallback readFailureHandler,
      int minInterval,
      int maxInterval,
      SubscriptionEstablishedCallback subscriptionEstablishedHandler);

  public native boolean mediaPlayback_subscribeToPlaybackSpeed(
      SuccessCallback<Float> readSuccessHandler,
      FailureCallback readFailureHandler,
      int minInterval,
      int maxInterval,
      SubscriptionEstablishedCallback subscriptionEstablishedHandler);

  public native boolean mediaPlayback_subscribeToSeekRangeEnd(
      SuccessCallback<Long> readSuccessHandler,
      FailureCallback readFailureHandler,
      int minInterval,
      int maxInterval,
      SubscriptionEstablishedCallback subscriptionEstablishedHandler);

  public native boolean mediaPlayback_subscribeToSeekRangeStart(
      SuccessCallback<Long> readSuccessHandler,
      FailureCallback readFailureHandler,
      int minInterval,
      int maxInterval,
      SubscriptionEstablishedCallback subscriptionEstablishedHandler);

  /*
   * APPLICATION LAUNCHER CLUSTER
   */
  public native boolean applicationLauncher_launchApp(
      short catalogVendorId, String applicationId, byte[] data, Object responseHandler);

  public native boolean applicationLauncher_stopApp(
      short catalogVendorId, String applicationId, Object responseHandler);

  public native boolean applicationLauncher_hideApp(
      short catalogVendorId, String applicationId, Object responseHandler);

  /*
   * TARGET NAVIGATOR CLUSTER
   */
  public native boolean targetNavigator_navigateTarget(
      byte target, String data, Object responseHandler);

  public native boolean targetNavigator_subscribeToCurrentTarget(
      SuccessCallback<Byte> readSuccessHandler,
      FailureCallback readFailureHandler,
      int minInterval,
      int maxInterval,
      SubscriptionEstablishedCallback subscriptionEstablishedHandler);

  public native boolean targetNavigator_subscribeToTargetList(
      SuccessCallback<TargetNavigatorTypes.TargetInfo> readSuccessHandler,
      FailureCallback readFailureHandler,
      int minInterval,
      int maxInterval,
      SubscriptionEstablishedCallback subscriptionEstablishedHandler);

  /*
   * KEYPAD INPUT CLUSTER
   */
  public native boolean keypadInput_sendKey(byte keyCode, Object responseHandler);

  /**
   * APPLICATION BASIC
   *
   * <p>TODO: Add APIs to subscribe to Application, Status and AllowedVendorList
   */
  public native boolean applicationBasic_subscribeToVendorName(
      SuccessCallback<String> readSuccessHandler,
      FailureCallback readFailureHandler,
      int minInterval,
      int maxInterval,
      SubscriptionEstablishedCallback subscriptionEstablishedHandler);

  public native boolean applicationBasic_subscribeToVendorID(
      SuccessCallback<Short> readSuccessHandler,
      FailureCallback readFailureHandler,
      int minInterval,
      int maxInterval,
      SubscriptionEstablishedCallback subscriptionEstablishedHandler);

  public native boolean applicationBasic_subscribeToApplicationName(
      SuccessCallback<String> readSuccessHandler,
      FailureCallback readFailureHandler,
      int minInterval,
      int maxInterval,
      SubscriptionEstablishedCallback subscriptionEstablishedHandler);

  public native boolean applicationBasic_subscribeToProductID(
      SuccessCallback<Short> readSuccessHandler,
      FailureCallback readFailureHandler,
      int minInterval,
      int maxInterval,
      SubscriptionEstablishedCallback subscriptionEstablishedHandler);

  public native boolean applicationBasic_subscribeToApplicationVersion(
      SuccessCallback<String> readSuccessHandler,
      FailureCallback readFailureHandler,
      int minInterval,
      int maxInterval,
      SubscriptionEstablishedCallback subscriptionEstablishedHandler);

  static {
    System.loadLibrary("TvCastingApp");
  }
}
