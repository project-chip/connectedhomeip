import argparse
import logging
import subprocess

def main():
  parser = argparse.ArgumentParser()
  parser.add_argument("pr", help="Pull request number")
  parser.add_argument("check", help="Pull request check to poll for")
  args = parser.parse_args()
  
  logging.info("Gathering info on checks being run for the current pull request.")
  for line in subprocess.run(f"gh pr checks -R project-chip/connectedhomeip {args.pr}", stdout=subprocess.PIPE, shell=True).stdout.decode("utf-8").splitlines():
    if args.check in line:
      print(line)

if __name__ == "__main__":
  main()
