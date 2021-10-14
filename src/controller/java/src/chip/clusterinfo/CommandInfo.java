package chip.clusterinfo;


public class CommandInfo {
  public CommandInfo(String name, Function<List<CommandParameter>, ClusterResponse> fn, List<CommandParameter> parameters, Class callbackClass) {
    this.name = name;
    this.fn = fn;
    this.parameters = parameters;
    this.callbackClass = callbackClass;
  }
  public CommandInfo() {}

  public String name;
  public Function<List<CommandParameter>, ClusterResponse> fn;
  public List<CommandParameter> parameters;
  public Class callbackClass;
}