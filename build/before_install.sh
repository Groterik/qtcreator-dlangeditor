#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source "${DIR}/setup-env.sh"

if [[ "$BUILD_OS_NAME" == "osx" ]]; then
  brew install p7zip
  brew install wget

  QT_BIN_LINK="http://download.qt.io/official_releases/qtcreator/3.6/3.6.0/installer_source/mac_x64/qtcreator.7z"

elif [[ $CI == "true" && "$BUILD_OS_NAME" == "linux" ]]; then
  sudo add-apt-repository -y ppa:beineri/opt-qt551
  # GCC 4.8
  sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
  sudo apt-get update -qq

  sudo apt-get install p7zip
  sudo apt-get install zip

  QT_BIN_LINK="http://download.qt.io/official_releases/qtcreator/3.6/3.6.0/installer_source/linux_gcc_64_rhel66/qtcreator.7z"

fi

mkdir -p "${DOWNLOAD_DIR}" && cd "${DOWNLOAD_DIR}"
wget "http://download.qt-project.org/official_releases/qtcreator/3.6/3.6.0/qt-creator-opensource-src-3.6.0.tar.gz"
tar xzf qt-creator-opensource-src-3.6.0.tar.gz
wget "${QT_BIN_LINK}"
7zr x -oqtcbuild/3.6.0/ qtcreator.7z

if [[ "$BUILD_OS_NAME" == "osx" ]]; then
  cd qtcbuild/3.6.0/ && ln -s ./ bin
fi

