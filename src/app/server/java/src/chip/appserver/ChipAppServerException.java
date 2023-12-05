package chip.appserver;

public class ChipAppServerException extends RuntimeException {
  private static final long serialVersionUID = 1L;

  public long errorCode;

  public ChipAppServerException() {}

  public ChipAppServerException(long errorCode, String message) {
    super(message != null ? message : String.format("Error Code %d", errorCode));
    this.errorCode = errorCode;
  }
}
