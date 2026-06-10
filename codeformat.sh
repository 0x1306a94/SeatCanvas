#!/usr/bin/env bash
cd $(dirname $0)

if [[ $(uname) == 'Darwin' ]]; then
  MAC_REQUIRED_TOOLS="python3 pipx"
  for TOOL in ${MAC_REQUIRED_TOOLS[@]}; do
    if [ ! $(which $TOOL) ]; then
      if [ ! $(which brew) ]; then
        echo "Homebrew not found. Trying to install..."
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)" ||
          exit 1
      fi
      echo "$TOOL not found. Trying to install..."
      brew install $TOOL || exit 1
    fi
  done

  export PATH=${HOME}/.local/bin:/opt/homebrew/bin:$PATH
  CLANG_FORMAT_VERSION=`clang-format --version`
  if [[ "${CLANG_FORMAT_VERSION}" =~ "14." ]]
  then
      echo "----${CLANG_FORMAT_VERSION}----"
  else
      echo "----install clang-format----"
      pipx install clang-format==14
  fi
fi

echo "----clang-format version----"
clang-format --version

echo "----swiftformat version----"
swiftformat --version

echo "----begin to scan code format----"

# 优化的文件查找和格式化
format_cpp_files() {
    local dir="$1"
    if [ -d "$dir" ]; then
        find "$dir" \( -name "*.cpp" -o -name "*.c" -o -name "*.h" -o -name "*.hpp" -o -name "*.mm" -o -name "*.m" \) \
            -print0 | xargs -0 -P 4 -n 20 clang-format -i
    fi
}

format_swift_files() {
    local dir="$1"
    if [ -d "$dir" ]; then
        find "$dir" -name "*.swift" -print0 | xargs -0 -P 4 -n 20 swiftformat
    fi
}

# 并行格式化不同目录
format_cpp_files "include" &
format_cpp_files "src" &
format_cpp_files "tests/src" &
format_cpp_files "ios/SeatCanvasSample" &
wait

format_swift_files "src" &
format_swift_files "ios/SeatCanvasSample" &
wait

git diff
result=`git diff`
if [[ $result =~ "diff" ]]
then
    echo "----Failed to pass the code format check----"
    exit 1
else
    echo "----Complete the code format check-----"
fi