#!/bin/bash
set -x

git pull upstream master

git status

scripts/tools/zap_regen_all.py

git status

git add .

git status

git commit -m "Regenerating ZAP"
