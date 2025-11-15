#!/bin/bash
cd /Users/niveditasarkar/workspace/Matter/connectedhomeip/src/python_testing
source venv/bin/activate
PYTHONPATH=/Users/niveditasarkar/workspace/Matter/connectedhomeip/src/controller/python:$PYTHONPATH python3 TC_TSTAT_4_3.py "$@"
