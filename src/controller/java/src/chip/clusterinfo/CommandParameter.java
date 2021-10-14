package chip.devicecontroller;

public class CommandParameter {
  public CommandParameter() {}

  public CommandParameter(String name, ParameterType type) {
    this.name = name;
    this.type = type;
  }

  public String name;
  public ParameterType type;
}