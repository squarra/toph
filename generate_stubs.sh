#!/usr/bin/env bash
set -euo pipefail

cmake -B build -DUSE_PYBIND=ON
cmake --build build
cp build/pytoph.cpython-3*-x86_64-linux-gnu.so .
stubgen -p pytoph -o . --include-docstrings -v
