#!/bin/bash

# Specify the directory
directory="./evaluation_motion"


# List all files in the directory
for filename in "$directory"/*; do
    if [ -f "$filename" ]; then
        python motion_detection.py "$filename"
    fi
done