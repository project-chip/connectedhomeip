package chip.clusterinfo;

<<<<<<< HEAD
<<<<<<< HEAD
/** CommandParameterInfo captures the name and type of a parameter */
=======
>>>>>>> fde1d13c0 (fix comments)
=======
/** CommandParameterInfo includes a name filed, which represents the name of the paremeter. It  */
/** also contains a type field to specify what type this commandParameter is, i.e String.class  */

>>>>>>> 2185ce827 (add descriptive documentation on each new class)
public class CommandParameterInfo {
  public CommandParameterInfo() {}

  public CommandParameterInfo(String name, Class<?> type) {
    this.name = name;
    this.type = type;
  }

  public String name;
  public Class<?> type;
}
