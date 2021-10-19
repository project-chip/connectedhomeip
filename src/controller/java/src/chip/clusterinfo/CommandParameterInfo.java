package chip.clusterinfo;

<<<<<<< HEAD
/** CommandParameterInfo captures the name and type of a parameter */
=======
>>>>>>> fde1d13c0 (fix comments)
public class CommandParameterInfo {
  public CommandParameterInfo() {}

  public CommandParameterInfo(String name, Class<?> type) {
    this.name = name;
    this.type = type;
  }

  public String name;
  public Class<?> type;
}
