#!/bin/bash
cd $(dirname $0)

PROJECT_DIR=$PWD

./install_tools.sh

if [[ `uname` == 'Darwin' ]]; then
  if [ ! $(which emcc) ]; then
      echo "emscripten not found. Trying to install..."
      brew install emscripten
  fi
fi

if [ ! $(which depctl) ]; then
  echo "depctl not found. Trying to install..."
  brew install 0x1306a94/tap/depctl > /dev/null
else
  brew upgrade 0x1306a94/tap/depctl > /dev/null
fi

depctl || exit 1

# depsync

# cd $PROJECT_DIR/third_party/tgfx
# echo "[*] Applying patch for tgfx"
# git checkout -- .
# git clean -fdx
# git apply $PROJECT_DIR/patches/tgfx.patch