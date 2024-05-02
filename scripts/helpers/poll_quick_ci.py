import argparse
import logging
import subprocess

def main():
  parser = argparse.ArgumentParser()
  parser.add_argument("pr", help="Pull request number")
  args = parser.parse_args()
  logging.info("Gathering info on checks being run for the current pull request.")
  subprocess.run(f"gh pr checks -R project-chip/connectedhomeip {args.pr}", shell=True)

if __name__ == "__main__":
  main()
