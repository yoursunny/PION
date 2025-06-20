#!/bin/bash
set -euo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")"/..

git ls-files '**/*.[hc]pp' '**/*.ino' | xargs -r \
  docker run --rm -u $(id -u):$(id -g) -v $PWD:/mnt -w /mnt ams21/clang-format:15 clang-format -i -style=file

git ls-files -- '*.sh' | xargs -r \
  docker run --rm -u $(id -u):$(id -g) -v $PWD:/mnt -w /mnt mvdan/shfmt:v3 -l -w -s -i=2 -ci

git ls-files -- Dockerfile '*/Dockerfile' | xargs -r \
  docker run --rm -u $(id -u):$(id -g) -v $PWD:/mnt -w /mnt hadolint/hadolint hadolint -t error
