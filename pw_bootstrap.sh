CHIP_ROOT="$(dirname "$0")"

export PW_BRANDING_BANNER="${CHIP_ROOT}/.chip-banner.txt"
export PW_BRANDING_BANNER_COLOR="bold_white"

git submodule update --init

source "${CHIP_ROOT}/third_party/pigweed/bootstrap.sh"
