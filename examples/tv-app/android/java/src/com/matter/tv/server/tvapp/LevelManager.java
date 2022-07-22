package com.matter.tv.server.tvapp;

public interface LevelManager {
  /**
   * Notify that the Level value be changed by matter and should effect it. Note, set by TvApp will
   * also trigger this function, so must check if value is same
   *
   * @param value
   */
  void HandleLevelChanged(int value);
}
