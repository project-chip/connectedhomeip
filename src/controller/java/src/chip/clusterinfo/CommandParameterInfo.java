package chip.clusterinfo;

/** CommandParameterInfo captures the name and type of a parameter */
public class CommandParameterInfo {
  public CommandParameterInfo() {}

  /*
   * If 'type' is a complex type, such as ArrayList or Optional, 'underlyingType'
   * stores information about the wrapped or element type.
   */
  public CommandParameterInfo(String name, Class<?> type, Class<?> underlyingType) {
    this.name = name;
    this.type = type;
    this.underlyingType = underlyingType;
  }

  public String name;
  public Class<?> type;
  public Class<?> underlyingType;
}
