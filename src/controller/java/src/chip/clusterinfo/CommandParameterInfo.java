package chip.clusterinfo;

/** CommandParameterInfo includes a name filed, which represents the name of the paremeter. It */
/** also contains a type field to specify what type this commandParameter is, i.e String.class */
public class CommandParameterInfo {
  public CommandParameterInfo() {}

  public CommandParameterInfo(String name, Class<?> type) {
    this.name = name;
    this.type = type;
  }

  public String name;
  public Class<?> type;
}
