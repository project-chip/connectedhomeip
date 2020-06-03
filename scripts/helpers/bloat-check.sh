#!/usr/bin/env bash

set -x

JOBNAME=$1
SOURCEDIR=$2
OUTPUTDIR=$3

mkdir -p ${OUTPUTDIR}
scripts/helpers/ci-fetch-artifacts.py \
    --token "${CIRCLECI_API_TOKEN}" \
    --job "${JOBNAME}" \
    --download-dir "${OUTPUTDIR}"

for masterbinary in ${OUTPUTDIR}/*; do
    newbinary=${SOURCEDIR}/$(basename $masterbinary)
    echo "Checking $newbinary against $masterbinary"
    echo "Size difference:"
    bloaty $newbinary -- $masterbinary
    echo "Largest symbols information:"
    bloaty -d symbols -C short $newbinary | head -n 20
done
