#!/bin/bash
set -x

git pull upstream master
git submodule update --init --recursive third_party/zap/

git status

cd third_party/zap/repo/
npm ci
npm run version-stamp
npm rebuild canvas --update-binary
npm run build-spa

cd ../../../

scripts/tools/zap_regen_all.py

git status

git add .

git status

git commit -m "Regenerating ZAP"
