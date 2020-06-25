CHIP_ROOT="$(dirname "$0")"

git submodule update --init

source "$CHIP_ROOT/third_party/pigweed/bootstrap.sh"
