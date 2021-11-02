package chip.clusterinfo;

import java.util.List;
import java.util.Map;

/**
 * Interface for making customized callback that implements both onSuccess and onFailure functions.
 */
public interface ClusterCommandCallback {
  void onSuccess(Map<String, Object> responseValues);

  void onFailure(Exception exception);
}