# Matter TLV Log Visualizer

This tool provides a web-based interface for visualizing and analyzing TLV
(Tag-Length-Value) log data from Matter devices. It consists of a Python Flask
server for parsing TLV files and a web app for visualization.

## Features

-   Upload and parse TLV log files
-   Interactive charts and visualizations
-   Timeline view of PAKE/Sigma messages
-   Statistical analysis of log data
-   Detailed table view of all entries

## Requirements

-   Python 3.6+
-   Matter SDK (CHIP)
-   Flask web server

## Installation

1. Create python environment

    ```
    python -m venv venv
    source venv/bin/activate
    ```

2. Install the required Python packages:

    ```
    pip install -r requirements.txt
    ```

3. Set the `CHIP_HOME` environment variable to point to your Matter (CHIP) SDK
   installation:
    ```
    export CHIP_HOME=/path/to/connectedhomeip
    ```

## Usage

1. Start the server:

    ```
    python tlv_server.py
    ```

2. Open a web browser and navigate to:

    ```
    http://localhost:8000
    ```

3. Upload a TLV log file or paste log data manually.

4. View visualizations across different tabs:
    - Overview: Summary of message types
    - Timeline: PAKE/Sigma message sequence
    - All Entries: Complete table of log entries
    - Statistics: Detailed analysis of the data

## Data Format

The tool expects TLV data with the following structure:

-   Timestamp (tag 0)
-   Label (tag 1)
-   Value (tag 2)

## Troubleshooting

If you encounter import errors related to `chip.tlv`, ensure:

1. The `CHIP_HOME` environment variable is set correctly
2. The Matter SDK is properly installed
