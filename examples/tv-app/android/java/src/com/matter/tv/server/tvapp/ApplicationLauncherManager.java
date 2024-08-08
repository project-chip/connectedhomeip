package com.matter.tv.server.tvapp;

public interface ApplicationLauncherManager {

  /**
   * Return a list of available catalogs
   *
   * @return list of int
   */
  int[] getCatalogList();

  /**
   * Launch an app
   *
   * @param app that you want to launch
   * @param data to send addditional data if needed
   * @return launcher response with status
   */
  LauncherResponse launchApp(Application app, String data);

  /**
   * Stop an app
   *
   * @param app that you want to stop
   * @return launcher response with status
   */
  LauncherResponse stopApp(Application app);

  /**
   * hide an app
   *
   * @param app that you want to hide
   * @return launcher response with status
   */
  LauncherResponse hideApp(Application app);
}
