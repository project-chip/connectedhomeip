/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package com.matter.tv.server.tvapp;

public interface ChannelManager {

  ChannelInfo[] getChannelList();

  ChannelLineupInfo getLineup();

  ChannelInfo getCurrentChannel();

  ChannelInfo changeChannel(String match);

  boolean changeChannelByNumber(int majorNumber, int minorNumber);

  boolean skipChannel(int count);
}
