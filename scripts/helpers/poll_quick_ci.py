import argparse
import logging
import subprocess
import time

def main():
  parser = argparse.ArgumentParser()
  parser.add_argument("pr", help="Pull request number")
  parser.add_argument("check", help="Pull request check to poll for")
  args = parser.parse_args()
  
  logging.info(f"Gathering info on the {args.check} check being run for the current pull request.")
  polling = True
  poll_count = 0
  poll_max = 300
  check_passed = False
  while(polling):
    for line in subprocess.run(f"gh pr checks -R project-chip/connectedhomeip {args.pr}", stdout=subprocess.PIPE, shell=True).stdout.decode("utf-8").splitlines():
      if args.check in line:
        logging.info(line)
        if "pending" in line:
          if poll_count == poll_max:
            polling = false
          else:
            poll_count += 1
            time.sleep(1)
          break
        elif "pass" in line:
          polling = False
          logging.info("Fast CI {args.check} has passed; if all fast CI passes, builds and tests may commence.")
          exit(0)
  logging.info(f"Polling for completion of fast CI {args.check} failed. Please ensure the name of the check was entered correctly and verify that it should take less than {poll_max} seconds to run.")
  exit(1)

if __name__ == "__main__":
  main()
