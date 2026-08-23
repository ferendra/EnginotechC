# EnginotechC++ — Installation Guide

## Quick Install (Linux/macOS)

### Option 1: Build from Source (Recommended)

```bash
# Install LLVM 18 (required)
# Ubuntu/Debian
wget -qO- https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
sudo add-apt-repository "deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-18 main"
sudo apt-get update
sudo apt-get install llvm-18 clang-18

# macOS
brew install llvm

# Clone and build
git clone https://github.com/enginotech/enginotech-cpp.git
cd enginotech-cpp
mkdir build && cd build
cmake .. -DLLVM_DIR=$(llvm-config-18 --cmakedir) -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Install
sudo make install
# or manual:
cp engc /usr/local/bin/
```

### Option 2: Download Pre-built Binary

```bash
# Linux x86_64
curl -L https://github.com/enginotech/enginotech-cpp/releases/download/v0.2.2/engc-0.2.2-linux.tar.gz | tar xz
chmod +x engc-linux-x86_64
sudo mv engc-linux-x86_64 /usr/local/bin/engc

# macOS
curl -L https://github.com/enginotech/enginotech-cpp/releases/download/v0.2.2/engc-0.2.2-macos.tar.gz | tar xz
chmod +x engc-macos-arm64
sudo mv engc-macos-arm64 /usr/local/bin/engc
```

## Verify Installation

```bash
engc --version
# Output: EnginotechC++ v0.2.2

engc run examples/hello.ec
# Output: Hello, World!
```

## Uninstall

```bash
sudo rm /usr/local/bin/engc
```

## Platform Support

| Platform | Status | Notes |
|----------|--------|-------|
| Linux x86_64 | ✅ Supported | LLVM 18 required |
| Linux ARM64 | ✅ Supported | LLVM 18 required |
| macOS (Apple Silicon) | ✅ Supported | Homebrew LLVM |
| macOS (Intel) | ⚠️ Limited | May need cross-compile |
| Windows | ⚠️ Coming Soon | Requires WSL2 or mingw |
