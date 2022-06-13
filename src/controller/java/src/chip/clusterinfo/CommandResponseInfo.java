package chip.clusterinfo;

/** CommandResponseInfo captures the name and type of a command response */
public class CommandResponseInfo {
  public CommandResponseInfo() {}

  public CommandResponseInfo(String name, String type) {
    this.name = name;
    this.type = type;
  }

  public String name;
  public String type;
}
