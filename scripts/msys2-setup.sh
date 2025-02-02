#!/bin/bash -eu

function usage
{
    echo "
Setup script for building NuSMV under MSYS2 UCRT64 environment.

Usage: ${0##*/} [OPTIONS]

Options:
  -d  Install extra packages for building NuSMV documentation
  -h  This help
  -p  Print a list of packages that would be installed
  -t  Install extra packages for testing NuSMV:
  -u  Pass --sysupgrade to pacman
  -y  Pass --refresh to pacman
"
}

function parse_options
{
    while getopts 'dhptuy' opt
    do
        case $opt in
        d)
            packages+=(${PACKAGES_DOC[*]})
            ;;
        p)
            print_pkg=1
            ;;
        t)
            packages+=(${PACKAGES_TEST[*]})
            ;;
        u)
            pacman_args+=('--sysupgrade')
            ;;
        y)
            pacman_args+=('--refresh')
            ;;
        *)
            usage
            exit 1
            ;;
        esac
    done
}


### main ####################################################################

if [[ ${MSYSTEM:-} != UCRT64 ]]
then
  echo "error: not MSYS2 UCRT64 environment: MSYSTEM='${MSYSTEM:-}'" >&2
  exit 1
fi

readonly -a PACKAGES_BASE=(
    # dependencies
    "${MINGW_PACKAGE_PREFIX}-gmp"
    "${MINGW_PACKAGE_PREFIX}-libiconv"
    "${MINGW_PACKAGE_PREFIX}-libsystre" # implementation for regex.h
    "${MINGW_PACKAGE_PREFIX}-libxml2"
    "${MINGW_PACKAGE_PREFIX}-wineditline"
    # for building
    "bison"
    "flex"
    "patch"
    "${MINGW_PACKAGE_PREFIX}-gcc"
    "${MINGW_PACKAGE_PREFIX}-meson"
    "${MINGW_PACKAGE_PREFIX}-python"
)
readonly -a PACKAGES_DOC=(
    "${MINGW_PACKAGE_PREFIX}-doxygen"
    "${MINGW_PACKAGE_PREFIX}-texlive-bin"
    "${MINGW_PACKAGE_PREFIX}-texlive-core"
    "${MINGW_PACKAGE_PREFIX}-texlive-extra-utils"
    "${MINGW_PACKAGE_PREFIX}-texlive-fonts-recommended"
    "${MINGW_PACKAGE_PREFIX}-texlive-latex-extra"
    "${MINGW_PACKAGE_PREFIX}-texlive-latex-recommended"
    "${MINGW_PACKAGE_PREFIX}-texlive-pictures"
    "${MINGW_PACKAGE_PREFIX}-texlive-plain-generic"
)
readonly -a PACKAGES_TEST=(
    "${MINGW_PACKAGE_PREFIX}-cppcheck"
    "${MINGW_PACKAGE_PREFIX}-gtest"
)

declare -a packages=(${PACKAGES_BASE[*]})

pacman_args=(
    '--sync'
    '--needed'
)

parse_options "${*}"

if [[ -v print_pkg ]]
then
    for p in ${packages[*]}
    do
        echo $p
    done
    exit 0
fi

pacman ${pacman_args[*]} ${packages[*]}
