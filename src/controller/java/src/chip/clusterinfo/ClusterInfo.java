package chip.devicecontroller;

public class ClusterInfo {

  public ClusterInfo(String name) {
    this.name = name;
    this.commands = new ArrayList<CommandInfo>();
  }
  public String name;
  public Class<T> clusterClass;
  public List<CommandInfo> commands;

}