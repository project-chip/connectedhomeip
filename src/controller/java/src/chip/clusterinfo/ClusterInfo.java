package chip.clusterinfo;

import java.util.*;

public class ClusterInfo {

  public ClusterInfo(String name) {
    this.name = name;
    this.commands = new ArrayList<CommandInfo>();
  }
  public String name;
  public Class<?> clusterClass;
  public List<CommandInfo> commands;

}