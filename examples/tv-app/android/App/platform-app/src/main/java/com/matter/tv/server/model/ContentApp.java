package com.matter.tv.server.model;

import com.matter.tv.app.api.SupportedCluster;
import java.util.Collections;
import java.util.Set;

public class ContentApp {

  private String appName;
  private String vendorName;
  private int vendorId;
  private int productId;
  private Set<SupportedCluster> supportedClusters;
  private int endpoint;

  public ContentApp(
      String appName,
      String vendorName,
      int vendorId,
      int productId,
      Set<SupportedCluster> supportedClusters) {
    this.vendorName = vendorName;
    this.appName = appName;
    this.vendorId = vendorId;
    this.productId = productId;
    this.supportedClusters = supportedClusters;
  }

  public String getAppName() {
    return appName;
  }

  public String getVendorName() {
    return vendorName;
  }

  public int getVendorId() {
    return vendorId;
  }

  public int getProductId() {
    return productId;
  }

  public int getEndpointId() {
    return endpoint;
  }

  public void setEndpointId(int endpoint) {
    this.endpoint = endpoint;
  }

  public Set<SupportedCluster> getSupportedClusters() {
    return Collections.unmodifiableSet(supportedClusters);
  }
}
