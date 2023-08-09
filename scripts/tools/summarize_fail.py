import subprocess

subprocess.run("gh run list -R project-chip/connectedhomeip -b master -s failure -L 100")
