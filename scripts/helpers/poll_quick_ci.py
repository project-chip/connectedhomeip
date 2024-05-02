import logging
import subprocess

def main():
  logging.info("Gathering info on checks being run for the current pull request.")
  subprocess.run("gh pr checks -R project-chip/connectedhomeip ${{ github.event.number }} -L 500", shell=True)

if __name__ == "__main__":
  main()
