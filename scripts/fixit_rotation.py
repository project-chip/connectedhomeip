#!/usr/bin/env python
import random

ROTATION_CHOICES = [
    'Andrei (Google)',
    'Boris (Apple)',
    'Cecille (Google)',
    'Damian (Nordic)',
    'Etienne (Silabs)',
    'Junior (Silabs)',
    'Kamil (Nordic)',
    'Kevin (Google)',
    # 'Martin (Google)', # TO be enabled July 2021
    'Michael (Google)',
    'Mingjie (Google)',
    'Pankaj (Apple)',
    'Ricardo (Silabs)',
    'Rob (Google)',
    'Song (Google)',
    'Timothy (Qorvo)',
    'Victor (Samsung)',
    'Vivien (Apple)',
    'Yufeng (Google)',
    'Yunhan (Google)',
]


def main():
    """Main task if executed standalone."""
    print("Rolling dice....")

    results = ROTATION_CHOICES[:]
    random.shuffle(results)

    print("Results: ")
    for idx, name in enumerate(results):
        print("    %2d: %s" % (idx + 1, name))


if __name__ == "__main__":
    main()
