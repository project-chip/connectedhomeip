import os
import fileinput

WORKFLOWS_DIR = '.github/workflows/'
GCB_DIR = 'integrations/cloudbuild/'
VERSION_FILE = 'integrations/docker/images/chip-build/version'

def get_version(version_file):
    with open(version_file) as f:
        l = f.readline()
        return l[:l.index(' ')]

def update_line(version, line, end_quote=False):
    if not end_quote:
        return line[:line.rindex(':')+1]+version
    else:
        return line[:line.rindex(':')+1]+version+'"'

def update_file(version, file, search_term, end_quote=False):
    with fileinput.input(file, inplace=True) as f:
        for line in f:
            if search_term in line:
                print(update_line(version, line, end_quote=end_quote))
            else:
                print(line, end='')

def update_workflows(version, directory):
    for workflow in os.listdir(directory):
        update_file(version,
            os.path.join(directory, workflow),
            'image:')

def update_gcb_configs(version, directory):
    for item in os.listdir(directory):
        item_path = os.path.join(directory, item)
        if str(item_path).endswith('.yaml'):
            update_file(version, item_path, 'name: "connectedhomeip', end_quote=True)

def main():
    version = get_version(VERSION_FILE)
    update_workflows(version, WORKFLOWS_DIR)
    update_gcb_configs(version, GCB_DIR)

if __name__ == '__main__':
    main()
