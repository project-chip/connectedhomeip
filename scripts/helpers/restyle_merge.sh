#!/usr/bin/env bash

env

git remote add upstream https://github.com/project-chip/connectedhomeip.git
git fetch upstream pull/"$1"/head
git merge --ff-only FETCH_HEAD
git push
