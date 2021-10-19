package chip.clusterinfo;

public class CommandParameterInfo {
  public CommandParameterInfo() {}

  public CommandParameterInfo(String name, Class<?> type) {
    this.name = name;
    this.type = type;
  }

  public String name;
  public Class<?> type;
}
