#! /bin/sh

set -e

git submodule update --init --depth 1

# Stock build and tests. This also builds build/host/mrbc, which
# yk_build_config needs.
rake -m -j "$(nproc)" all test

export CARGO_HOME="`pwd`/.cargo"
export RUSTUP_HOME="`pwd`/.rustup"
export RUSTUP_INIT_SKIP_PATH_CHECK="yes"
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs > rustup.sh
sh rustup.sh --default-host x86_64-unknown-linux-gnu \
    --default-toolchain nightly \
    --no-modify-path \
    --profile minimal \
    -y
export PATH=`pwd`/.cargo/bin/:$PATH

git clone --depth 1 --recurse-submodules --shallow-submodules https://github.com/ykjit/yk
cd yk
echo "yk commit: $(git show -s --format=%H)"

cd ykllvm
ykllvm_hash=$(git rev-parse HEAD)
if [ -f /opt/ykllvm_cache/ykllvm-release-with-assertions-"${ykllvm_hash}".tgz ]; then
    mkdir inst
    cd inst
    tar xfz /opt/ykllvm_cache/ykllvm-release-with-assertions-"${ykllvm_hash}".tgz
    cd ..
    # Minimally check that we can at least run `clang --version`: if we can't,
    # we assume the cached binary is too old (e.g. linking against old shared
    # objects) and that we should build our own version.
    if inst/bin/clang --version > /dev/null; then
        export YKB_YKLLVM_BIN_DIR="$(pwd)/inst/bin"
    else
        echo "Warning: cached ykllvm not runnable; building from scratch" > /dev/stderr
        rm -rf inst
    fi
fi
cd ..

YKB_YKLLVM_BUILD_ARGS="define:CMAKE_C_COMPILER=/usr/bin/clang,define:CMAKE_CXX_COMPILER=/usr/bin/clang++" \
    cargo build --release
cd ..

# yk build and tests.
export YK_CONFIG="`pwd`/yk/bin/yk-config"
export YK_PROFILE=release
YK_SIDETRACE_THRESHOLD=1 YKD_SERIALISE_COMPILATION=1 YK_HOT_THRESHOLD=1 rake -m -j "$(nproc)" MRUBY_CONFIG=yk_build_config all test
