#!/bin/bash

#Exit on error
set -e

if [[ -d "build" ]]; then
  echo "build folder exists, proceeding to build...\n"
else
  echo "build folder not found, making build folder...\n"
  mkdir build
fi

cd build
cmake ..
make
./VulkanTriangle
