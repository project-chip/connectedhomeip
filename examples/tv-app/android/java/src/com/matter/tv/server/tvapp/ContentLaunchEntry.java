package com.matter.tv.server.tvapp;

public class ContentLaunchEntry {
  /** Name of the content that customer will see on the screen */
  public String name;
  /** List of possible query matches */
  public ContentLaunchSearchParameter[] parameters;

  public ContentLaunchEntry(String name, ContentLaunchSearchParameter[] parameters) {
    this.name = name;
    this.parameters = parameters;
  }
}
