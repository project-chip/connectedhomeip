
import sys, os

class PythonPath(object):
    def __init__(self, *path, **kwargs):
        """
        path: defines 
        """

        relative_to = kwargs.get("relative_to", None)
        path = os.path.join(*path)

        if not os.path.isabs(path):
            if relative_to is None:
                base = os.getcwd()
            else:
                base = relative_to

            base_is_file = not os.path.isdir(base)

            if base_is_file:
                base = os.path.dirname(base)

            path = os.path.join(base, path)

        self.dir_path = os.path.realpath(path)


    def __enter__(self):
        sys.path.insert(0, self.dir_path)
        return self.dir_path

    def __exit__(self, type, value, traceback):
        sys.path.remove(self.dir_path)
