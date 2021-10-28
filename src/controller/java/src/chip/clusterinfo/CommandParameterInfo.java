package chip.clusterinfo;

/** CommandParameterInfo captures the name and type of a parameter */
public class CommandParameterInfo {
  public CommandParameterInfo() {}

  public CommandParameterInfo(String name, Class<?> type) {
    this.name = name;
    this.type = type;
  }

  public String name;
  public Class<?> type;
}
