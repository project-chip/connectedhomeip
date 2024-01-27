package com.matter.tv.server.tvapp;

public class ChannelProgramResponse {
  ChannelProgramInfo[] programs;
  int previousLimit = -1;
  String previousAfter;
  String previousBefore;
  int nextLimit = -1;
  String nextAfter;
  String nextBefore;
}
