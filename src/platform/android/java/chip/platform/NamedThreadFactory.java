package chip.platform;

import java.util.concurrent.ThreadFactory;
import java.util.concurrent.atomic.AtomicInteger;

public class NamedThreadFactory implements ThreadFactory {
  private final String prefix;
  private final AtomicInteger counter = new AtomicInteger(1);

  public NamedThreadFactory(final String prefix) {
    this.prefix = prefix;
  }

  @Override
  public Thread newThread(Runnable r) {
    final Thread thread = new Thread(r);
    thread.setName(prefix + "-" + counter.getAndIncrement());
    return thread;
  }
}
