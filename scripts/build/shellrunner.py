import logging
import os
import subprocess
import threading


class LogPipe(threading.Thread):

  def __init__(self, level):
    """Setup the object with a logger and a loglevel

        and start the thread
        """
    threading.Thread.__init__(self)
    self.daemon = False
    self.level = level
    self.fd_read, self.fd_write = os.pipe()
    self.pipeReader = os.fdopen(self.fd_read)
    self.start()

  def fileno(self):
    """Return the write file descriptor of the pipe"""
    return self.fd_write

  def run(self):
    """Run the thread, logging everything."""
    for line in iter(self.pipeReader.readline, ''):
      logging.log(self.level, line.strip('\n'))

    self.pipeReader.close()

  def close(self):
    """Close the write end of the pipe."""
    os.close(self.fd_write)


class ShellRunner:

  def Run(self, *args, **kargs):
    outpipe = LogPipe(logging.INFO)
    errpipe = LogPipe(logging.WARN)

    with subprocess.Popen(*args, **kargs, stdout=outpipe, stderr=errpipe) as s:
      outpipe.close()
      errpipe.close()
      code = s.wait()
      if code != 0:
        raise Exception('Command %r failed: %d' % (args, code))
      else:
        logging.info('Command %r completed', args)
