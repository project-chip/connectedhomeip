package chip.clusterinfo;

/** CommandParameterInfo captures the name and type of a parameter */
public class ResponseValueInfo {
  public ResponseValueInfo() {}

  public ResponseValueInfo(String name, String type) {
    this.name = name;
    this.type = type;
  }

  public String name;
  public String type;
}
