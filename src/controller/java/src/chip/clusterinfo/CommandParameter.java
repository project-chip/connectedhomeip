package chip.clusterinfo;

public class CommandParameter {
  public CommandParameter() {}

  public CommandParameter(String name, Class<?> type) {
    this.name = name;
    this.type = type;
  }

  public String name;
  public Class<?> type;
}