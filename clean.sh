#! /usr/bin/env bash
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

echo "Killing $SCRIPT_DIR/build"
read -r -p "Proceed with removal? [y/N]:" inpt
if [[ ${inpt^^} == 'Y' || ${inpt^^} == 'YES' ]]; then
    rm -rf "$SCRIPT_DIR/build"
else
    echo "Cancelled."
fi
