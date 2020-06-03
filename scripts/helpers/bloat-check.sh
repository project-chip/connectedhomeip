#!/usr/bin/env bash

set -x

JOBNAME=$1
SOURCEDIR=$2
OUTPUTDIR=$3
REPORTFILE=$4

mkdir -p ${OUTPUTDIR}
scripts/helpers/ci-fetch-artifacts.py \
    --token "${CIRCLECI_API_TOKEN}" \
    --job "${JOBNAME}" \
    --download-dir "${OUTPUTDIR}"

echo "BLOAT REPORT" >${REPORTFILE}

for masterbinary in ${OUTPUTDIR}/*; do
    newbinary=${SOURCEDIR}/$(basename $masterbinary)
    echo "Checking $newbinary against $masterbinary" >${REPORTFILE}
    bloaty $newbinary -- $masterbinary >${REPORTFILE}
done
