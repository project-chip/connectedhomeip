import argparse
import subprocess
import time


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("pr", help="Pull request number")
    parser.add_argument("check", help="Pull request check to poll for")
    args = parser.parse_args()

    print(f"Gathering info on the {args.check} check being run for the current pull request.")
    polling = True
    poll_count = 0
    poll_max = 3600
    while (polling):
        for line in subprocess.run(f"gh pr checks -R project-chip/connectedhomeip {args.pr}", stdout=subprocess.PIPE, shell=True).stdout.decode("utf-8").splitlines():
            poll_count += 1
            if args.check in line:
                print(line)
                if "pending" in line:
                    if poll_count == poll_max:
                        polling = False
                    else:
                        time.sleep(1)
                    break
                elif "pass" in line:
                    print(f"Fast CI {args.check} has passed; if all fast CI passes, builds and tests may commence.")
                    exit(0)
                elif "fail" in line:
                    print(f"Fast CI {args.check} has failed; please resolve this issue before running builds and tests.")
                    exit(1)

    print(
        f"Polling for completion of fast CI {args.check} failed. Please ensure the name of the check was entered correctly and verify that it should take less than {poll_max} seconds to run.")
    exit(1)


if __name__ == "__main__":
    main()
