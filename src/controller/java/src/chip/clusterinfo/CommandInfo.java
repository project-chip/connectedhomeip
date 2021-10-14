package chip.clusterinfo;

import java.util.function.Function;
import java.util.*;


public class CommandInfo {
  // TODO make Class<?> to ClusterResponse
  public CommandInfo(String name, Function<List<CommandParameter>, Class<?>> fn, List<CommandParameter> parameters, Class<?> callbackClass) {
    this.name = name;
    this.fn = fn;
    this.parameters = parameters;
    this.callbackClass = callbackClass;
  }
  public CommandInfo() {}

  public String name;
  public Function<List<CommandParameter>, Class<?>> fn;
  public List<CommandParameter> parameters;
  public Class<?> callbackClass;
}