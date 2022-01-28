package com.chip.casting.dnssd;

import java.net.InetAddress;
import java.util.Arrays;
import java.util.List;

public class DiscoveredNodeData {
  private static final int MAX_IP_ADDRESSES = 5;
  private static final int MAX_ROTATING_ID_LEN = 50;

  private String hostName;
  private String instanceName;
  private short longDiscriminator;
  private short vendorId;
  private short productId;
  private byte commissioningMode;
  private short deviceType;
  private String deviceName;
  private byte rotatingId[] = new byte[MAX_ROTATING_ID_LEN];
  private int rotatingIdLen;
  private short pairingHint;
  private String pairingInstruction;
  private short port;
  private int numIPs;
  private List<InetAddress> ipAddresses;

  public String getHostName() {
    return hostName;
  }

  public void setHostName(String hostName) {
    this.hostName = hostName;
  }

  public String getInstanceName() {
    return instanceName;
  }

  public void setInstanceName(String instanceName) {
    this.instanceName = instanceName;
  }

  public short getLongDiscriminator() {
    return longDiscriminator;
  }

  public void setLongDiscriminator(short longDiscriminator) {
    this.longDiscriminator = longDiscriminator;
  }

  public short getVendorId() {
    return vendorId;
  }

  public void setVendorId(short vendorId) {
    this.vendorId = vendorId;
  }

  public short getProductId() {
    return productId;
  }

  public void setProductId(short productId) {
    this.productId = productId;
  }

  public byte getCommissioningMode() {
    return commissioningMode;
  }

  public void setCommissioningMode(byte commissioningMode) {
    this.commissioningMode = commissioningMode;
  }

  public short getDeviceType() {
    return deviceType;
  }

  public void setDeviceType(short deviceType) {
    this.deviceType = deviceType;
  }

  public String getDeviceName() {
    return deviceName;
  }

  public void setDeviceName(String deviceName) {
    this.deviceName = deviceName;
  }

  public byte[] getRotatingId() {
    return rotatingId;
  }

  public void setRotatingId(byte[] rotatingId) {
    this.rotatingId = rotatingId;
  }

  public int getRotatingIdLen() {
    return rotatingIdLen;
  }

  public void setRotatingIdLen(int rotatingIdLen) {
    this.rotatingIdLen = rotatingIdLen;
  }

  public short getPairingHint() {
    return pairingHint;
  }

  public void setPairingHint(short pairingHint) {
    this.pairingHint = pairingHint;
  }

  public String getPairingInstruction() {
    return pairingInstruction;
  }

  public void setPairingInstruction(String pairingInstruction) {
    this.pairingInstruction = pairingInstruction;
  }

  public short getPort() {
    return port;
  }

  public void setPort(short port) {
    this.port = port;
  }

  public int getNumIPs() {
    return numIPs;
  }

  public void setNumIPs(int numIPs) {
    this.numIPs = numIPs;
  }

  public List<InetAddress> getIpAddresses() {
    return ipAddresses;
  }

  public void setIpAddresses(List<InetAddress> ipAddresses) {
    this.ipAddresses = ipAddresses;
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
