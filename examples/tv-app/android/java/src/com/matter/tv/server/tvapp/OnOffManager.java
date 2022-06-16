package com.matter.tv.server.tvapp;

public interface OnOffManager {
  /**
   * Notify that the OnOff value be changed by matter and should effect it. Note, set by TvApp will
   * also trigger this function, so must check if value is same
   *
   * @param value
   */
  void HandleOnOffChanged(boolean value);
}
