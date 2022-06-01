package com.chip.casting.dnssd;

import android.net.nsd.NsdServiceInfo;
import java.net.InetAddress;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.List;
import java.util.Map;

public class DiscoveredNodeData {
  private static final int MAX_IP_ADDRESSES = 5;
  private static final int MAX_ROTATING_ID_LEN = 50;
  private static final String KEY_DEVICE_NAME = "DN";
  private static final String KEY_DEVICE_TYPE = "DT";
  private static final String KEY_VENDOR_PRODUCT = "VP";

  private String hostName;
  private String instanceName;
  private long longDiscriminator;
  private long vendorId;
  private long productId;
  private byte commissioningMode;
  private long deviceType;
  private String deviceName;
  private byte rotatingId[] = new byte[MAX_ROTATING_ID_LEN];
  private int rotatingIdLen;
  private short pairingHint;
  private String pairingInstruction;
  private int port;
  private int numIPs;
  private List<InetAddress> ipAddresses;

  public DiscoveredNodeData(NsdServiceInfo serviceInfo) {
    Map<String, byte[]> attributes = serviceInfo.getAttributes();
    this.deviceName = new String(attributes.get(KEY_DEVICE_NAME), StandardCharsets.UTF_8);
    this.deviceType =
        Long.parseLong(new String(attributes.get(KEY_DEVICE_TYPE), StandardCharsets.UTF_8));

    String vp = new String(attributes.get(KEY_VENDOR_PRODUCT), StandardCharsets.UTF_8);
    if (vp != null) {
      String[] vpArray = vp.split("\\+");
      if (vpArray.length > 0) {
        this.vendorId = Long.parseLong(vpArray[0]);
        if (vpArray.length == 2) {
          this.productId = Long.parseLong(vpArray[1]);
        }
      }
    }

    this.port = serviceInfo.getPort();
    this.ipAddresses = Arrays.asList(serviceInfo.getHost());
    this.numIPs = 1;
  }

  public String getHostName() {
    return hostName;
  }

  public String getInstanceName() {
    return instanceName;
  }

  public long getLongDiscriminator() {
    return longDiscriminator;
  }

  public long getVendorId() {
    return vendorId;
  }

  public long getProductId() {
    return productId;
  }

  public byte getCommissioningMode() {
    return commissioningMode;
  }

  public long getDeviceType() {
    return deviceType;
  }

  public String getDeviceName() {
    return deviceName;
  }

  public byte[] getRotatingId() {
    return rotatingId;
  }

  public int getRotatingIdLen() {
    return rotatingIdLen;
  }

  public short getPairingHint() {
    return pairingHint;
  }

  public String getPairingInstruction() {
    return pairingInstruction;
  }

  public int getPort() {
    return port;
  }

  public int getNumIPs() {
    return numIPs;
  }

  public List<InetAddress> getIpAddresses() {
    return ipAddresses;
  }

  @Override
  public String toString() {
    return "DiscoveredNodeData{"
        + "hostName='"
        + hostName
        + '\''
        + ", instanceName='"
        + instanceName
        + '\''
        + ", longDiscriminator="
        + longDiscriminator
        + ", vendorId="
        + vendorId
        + ", productId="
        + productId
        + ", commissioningMode="
        + commissioningMode
        + ", deviceType="
        + deviceType
        + ", deviceName='"
        + deviceName
        + '\''
        + ", rotatingId="
        + Arrays.toString(rotatingId)
        + ", rotatingIdLen="
        + rotatingIdLen
        + ", pairingHint="
        + pairingHint
        + ", pairingInstruction='"
        + pairingInstruction
        + '\''
        + ", port="
        + port
        + ", numIPs="
        + numIPs
        + ", ipAddresses="
        + ipAddresses
        + '}';
  }
}
