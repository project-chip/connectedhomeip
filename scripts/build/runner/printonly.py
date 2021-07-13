import shlex


class PrintOnlyRunner:
  def __init__(self, output_file):
    self.output_file = output_file

  def Run(self, cmd, cwd=None, title=None):
    if title:
      self.output_file.write("# " + title + "\n")

    if cwd:
      self.output_file.write('cd "%s"\n' % cwd)

    self.output_file.write(" ".join([shlex.quote(part) for part in cmd]) + "\n")

    if cwd:
      self.output_file.write("cd -\n")

    self.output_file.write("\n")
