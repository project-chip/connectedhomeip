package chip.clusterinfo;

/** Interface for a callback that delegates to {@link ClusterCommandCallback}. */
public interface DelegatedClusterCallback {
  void setCallbackDelegate(ClusterCommandCallback callback);
}
