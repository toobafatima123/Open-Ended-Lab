#!/bin/bash

# Path to your processed file
processed_file="/home/toothpick/Desktop/work/processed.txt"

# Path to your report file
report_file="/home/toothpick/Desktop/work/report.txt"

# Reset the processed file (you might need to adjust this based on your file structure)
> "$processed_file"

# Reset or create a new report file
echo "" > "$report_file"


