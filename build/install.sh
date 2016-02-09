#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source "${DIR}/setup-env.sh"

if [[ $BUILD_OS_NAME == "osx" ]]; then
  brew install qt5
  brew link --force qt5
elif [[ $CI == "true" && $BUILD_OS_NAME == "linux" ]]; then
  sudo apt-get install qt55-meta-full
  sudo apt-get install -qq gcc-4.8 g++-4.8 
  sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.8 90 
fi
