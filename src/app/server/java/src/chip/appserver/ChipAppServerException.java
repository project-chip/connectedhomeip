package chip.appserver;

public class ChipAppServerException extends RuntimeException {
  private static final long serialVersionUID = 1L;

  public int errorCode;

  public ChipAppServerException() {}

  public ChipAppServerException(int errorCode, String message) {
    super(message != null ? message : String.format("Error Code %d", errorCode));
    this.errorCode = errorCode;
  }
}
