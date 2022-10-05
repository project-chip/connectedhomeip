function supportedCluster(clusterName) {
  switch (clusterName) {
    case "Network Commissioning": return false
    case "Groups": return false   
    case "Test Cluster": return false
    default: return true
  }
}


exports.supportedCluster = supportedCluster
