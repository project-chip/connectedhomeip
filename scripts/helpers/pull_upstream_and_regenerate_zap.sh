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

git add zzz_generated/*
git add src/darwin/Framework/*
git add src/controller/python/chip/clusters/*
git add src/controller/java/zap-generated/*

git status

git commit -m "Regenerating ZAP"
