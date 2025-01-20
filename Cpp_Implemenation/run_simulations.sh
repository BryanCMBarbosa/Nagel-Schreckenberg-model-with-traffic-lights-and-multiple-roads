#!/usr/bin/env bash

# Default configuration values
DEFAULT_CONFIG_DIR="./configs"
DEFAULT_RESULTS_DIR="./results"
DEFAULT_EXECUTION_TYPE=2

# Check if the user provided the required arguments
if [ $# -lt 1 ]; then
    echo "Usage: $0 <num_runs> [config_dir] [results_dir] [execution_type]"
    exit 1
fi

# Assign command-line arguments or use default values
NUM_RUNS=$1
CONFIG_DIR=${2:-$DEFAULT_CONFIG_DIR}
RESULTS_DIR=${3:-$DEFAULT_RESULTS_DIR}
EXECUTION_TYPE=${4:-$DEFAULT_EXECUTION_TYPE}

# Create the results directory if it doesn't exist
mkdir -p "$RESULTS_DIR"

# Loop through all JSON files in the config directory
for configFile in "$CONFIG_DIR"/*.json; do
    echo "Processing config file: $configFile"
    echo "Results directory: $RESULTS_DIR"

    # Run simulation NUM_RUNS times for this config file
    for i in $(seq 1 "$NUM_RUNS"); do
        echo "  Run #$i"
        ./simulation "$configFile" "$RESULTS_DIR" "$EXECUTION_TYPE"
    done
done
