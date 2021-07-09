import shlex


class PrintOnlyRunner:

  def Run(self, cmd, cwd=None, title=None):
    if title:
      print("# " + title)

    if cwd:
      print('cd "%s"' % cwd)

    print(" ".join([shlex.quote(part) for part in cmd]))

    if cwd:
      print("cd -")

    print()
