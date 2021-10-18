package chip.clusterinfo;

import chip.clusterinfo.ClusterCommandCallback;

public interface DelegatedClusterCallback {
  void setCallbackDelegate(ClusterCommandCallback callback);
}
