package com.matter.tv.server.model;

import com.matter.tv.app.api.SupportedCluster;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class ContentApp {

  public static final int INVALID_ENDPOINTID = -1;
  private final String version;
  private String appName;
  private String vendorName;
  private int vendorId;
  private int productId;
  private Set<SupportedCluster> supportedClusters;
  // initially set to an invalid value.
  private int endpoint = INVALID_ENDPOINTID;

  public ContentApp(
      String appName, String vendorName, int vendorId, int productId, String version) {
    this.vendorName = vendorName;
    this.appName = appName;
    this.vendorId = vendorId;
    this.productId = productId;
    this.version = version;
    this.supportedClusters = Collections.EMPTY_SET;
  }

  public ContentApp(
      String appName,
      String vendorName,
      int vendorId,
      int productId,
      String version,
      Set<SupportedCluster> supportedClusters) {
    this.vendorName = vendorName;
    this.appName = appName;
    this.vendorId = vendorId;
    this.productId = productId;
    this.version = version;
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

  public String getVersion() {
    return version;
  }

  public void setSupportedClusters(List<SupportedCluster> supportedClusters) {
    this.supportedClusters = new HashSet<>(supportedClusters);
  }
}
