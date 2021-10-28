package chip.clusterinfo;

import java.util.List;

/**
 * Interface for making customized callback that implements both onSuccess and onFailure functions.
 */
public interface ClusterCommandCallback {
  void onSuccess(List<Object> responseValues);

  void onFailure(Exception exception);
}