#!/usr/bin/env bash
# EnginotechC++ — Production Build Script
# Supports: parallel builds, incremental, debug/release, config file, cross-platform
#
# Usage:
#   ./scripts/build.sh [options] [target]
#   Targets: all, compiler, stdlib, tools, test, clean, install
#   Options: --debug, --release, --jobs N, --config FILE, --verbose

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$PROJECT_ROOT"

# Default configuration
BUILD_TYPE="Release"
JOBS="$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)"
OUT_DIR=".build"
CONFIG_FILE="build.config"
VERBOSE=0
TARGET="all"
REBUILD=0
INSTALL_PREFIX="/usr/local"

# Color output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

log() { echo -e "${BLUE}[BUILD]${NC} $*"; }
warn() { echo -e "${YELLOW}[WARN]${NC} $*"; }
err() { echo -e "${RED}[ERROR]${NC} $*" >&2; }
success() { echo -e "${GREEN}[OK]${NC} $*"; }

usage() {
    cat <<EOF
EnginotechC++ Build Script v0.5.0

Usage: $0 [OPTIONS] [TARGET]

TARGETS:
    all         Build everything (default)
    compiler    Build engc compiler only
    stdlib      Build standard library only
    tools       Build auxiliary tools
    test        Build and run tests
    clean       Remove build artifacts
    install     Install to system (requires sudo)

OPTIONS:
    --debug           Build with debug symbols
    --release         Build optimized release (default)
    -j, --jobs N      Parallel jobs (default: $JOBS)
    -c, --config FILE Use config file (default: build.config)
    -o, --out DIR     Output directory (default: .build)
    --prefix PATH     Install prefix (default: /usr/local)
    -v, --verbose     Verbose output
    --rebuild         Force clean rebuild
    -h, --help        Show this help

EXAMPLES:
    $0                          # Release build, all targets
    $0 --debug -j8              # Debug build with 8 parallel jobs
    $0 test                     # Build and run tests
    $0 --config my.config       # Use custom config
    $0 clean && $0              # Clean rebuild

CONFIG FILE (build.config):
    CXX=clang++
    CXXFLAGS="-O3 -march=native"
    LDFLAGS="-static"
    MODULES="compiler std runtime"
    TARGET_ARCH="x86_64"
EOF
    exit 0
}

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --debug) BUILD_TYPE="Debug" ;;
        --release) BUILD_TYPE="Release" ;;
        -j|--jobs) JOBS="$2"; shift ;;
        -c|--config) CONFIG_FILE="$2"; shift ;;
        -o|--out) OUT_DIR="$2"; shift ;;
        --prefix) INSTALL_PREFIX="$2"; shift ;;
        -v|--verbose) VERBOSE=1 ;;
        --rebuild) REBUILD=1 ;;
        -h|--help) usage ;;
        all|compiler|stdlib|tools|test|clean|install) TARGET="$1" ;;
        *) err "Unknown option: $1"; usage ;;
    esac
    shift
done

# Load config file
if [[ -f "$CONFIG_FILE" ]]; then
    log "Loading config: $CONFIG_FILE"
    # shellcheck disable=SC1090
    source "$CONFIG_FILE"
fi

# Detect compiler
detect_compiler() {
    if [[ -n "${CXX:-}" ]]; then
        echo "$CXX"
        return
    fi
    for c in "clang++" "g++" "c++" "zig c++"; do
        if command -v ${c%% *} >/dev/null 2>&1; then
            echo "$c"
            return
        fi
    done
    err "No C++ compiler found. Install clang++, g++, or zig."
    exit 1
}

CXX="$(detect_compiler)"
log "Using compiler: $CXX"
log "Build type: $BUILD_TYPE"
log "Jobs: $JOBS"
log "Output: $OUT_DIR"

# Version
VERSION="$(awk 'NF && $1 !~ /^#/ {print; exit}' VERSION 2>/dev/null | tr -d '[:space:]')"
[[ -z "$VERSION" ]] && VERSION="dev"
DEFS="-DENG_VERSION=\"${VERSION}\" -DBUILD_TYPE=\"${BUILD_TYPE}\""

# Compile flags
case "$BUILD_TYPE" in
    Debug)
        CXXFLAGS="${CXXFLAGS:-} -std=c++20 -g -O0 -Wall -Wextra -DDEBUG"
        ;;
    Release)
        CXXFLAGS="${CXXFLAGS:-} -std=c++20 -O3 -DNDEBUG -march=native"
        ;;
    RelWithDebInfo)
        CXXFLAGS="${CXXFLAGS:-} -std=c++20 -O2 -g -DNDEBUG"
        ;;
esac

# Linker flags
LDFLAGS="${LDFLAGS:-}"

# Module definitions
# Format: "module_name:source_dirs:exclude_patterns"
MODULES=(
    "compiler:compiler:tests"
    "std:std:tests"
    "runtime:runtime:tests"
    "build-system:build-system:tests"
    "formatter:formatter:tests"
    "linter:linter:tests"
    "langserver:langserver:tests"
    "packages:packages:tests"
)

# Collect sources for a module
collect_sources() {
    local module_name="$1"
    local src_dirs="$2"
    local exclude_dirs="$3"
    
    local find_cmd=(find)
    for d in ${src_dirs//:/ }; do
        find_cmd+=("$d")
    done
    find_cmd+=(-name '*.cpp')
    for e in ${exclude_dirs//:/ }; do
        find_cmd+=(-not -path "*/$e/*")
    done
    
    mapfile -t SRCS < <("${find_cmd[@]}" | sort)
}

# Build single object
build_object() {
    local src="$1"
    local obj="$2"
    local deps="$3"
    
    if [[ $REBUILD -eq 1 || ! -f "$obj" || "$src" -nt "$obj" ]]; then
        # Check header dependencies
        if [[ -f "$deps" ]]; then
            while IFS= read -r dep; do
                [[ "$dep" -nt "$obj" ]] && { needs_rebuild=1; break; }
            done < "$deps"
        fi
        
        local cmd=($CXX $CXXFLAGS $DEFS -MMD -MP -MF "$deps" -c "$src" -o "$obj")
        if [[ $VERBOSE -eq 1 ]]; then
            echo "  ${cmd[*]}"
        fi
        if ! "${cmd[@]}"; then
            err "Failed to compile: $src"
            return 1
        fi
        return 0
    fi
    return 2  # up to date
}

# Build module
build_module() {
    local module_name="$1"
    local src_dirs="$2"
    local exclude_dirs="$3"
    
    log "Building module: $module_name"
    
    collect_sources "$module_name" "$src_dirs" "$exclude_dirs"
    [[ ${#SRCS[@]} -eq 0 ]] && { warn "No sources for $module_name"; return 0; }
    
    local objdir="$OUT_DIR/obj/$module_name"
    mkdir -p "$objdir"
    
    local objs=()
    local deps_files=()
    local changed=0
    local up_to_date=0
    
    # Compile in parallel using xargs
    printf '%s\n' "${SRCS[@]}" | xargs -P "$JOBS" -I {} bash -c '
        src="{}"
        obj="'$objdir'/$(echo "$src" | tr "/" "_").o"
        deps="'$objdir'/$(echo "$src" | tr "/" "_").d"
        build_object "$src" "$obj" "$deps"
        ret=$?
        echo "$ret:$obj" >> "'$objdir'/build_status"
    '
    
    # Collect results
    if [[ -f "$objdir/build_status" ]]; then
        while IFS=: read -r ret obj; do
            case $ret in
                0) ((changed++)); objs+=("$obj"); deps_files+=("${obj%.o}.d") ;;
                2) ((up_to_date++)); objs+=("$obj"); deps_files+=("${obj%.o}.d") ;;
                *) err "Build failed"; return 1 ;;
            esac
        done < "$objdir/build_status"
        rm -f "$objdir/build_status"
    fi
    
    log "$module_name: $changed compiled, $up_to_date up to date"
    echo "${objs[@]}"
}

# Link executable
link_executable() {
    local name="$1"
    shift
    local objs=("$@")
    local output="$OUT_DIR/$name"
    
    log "Linking: $name"
    local cmd=($CXX $LDFLAGS "${objs[@]}" -o "$output")
    if [[ $VERBOSE -eq 1 ]]; then
        echo "  ${cmd[*]}"
    fi
    if ! "${cmd[@]}"; then
        err "Link failed: $name"
        return 1
    fi
    success "Created: $output"
}

# Build all modules
build_all() {
    local all_objs=()
    
    for module_def in "${MODULES[@]}"; do
        IFS=: read -r name dirs excludes <<< "$module_def"
        local objs_str
        objs_str=$(build_module "$name" "$dirs" "$excludes") || return 1
        read -ra objs <<< "$objs_str"
        all_objs+=("${objs[@]}")
    done
    
    # Link main compiler
    link_executable "engc" "${all_objs[@]}" || return 1
    
    # Build tools if requested
    if [[ "$TARGET" == "all" || "$TARGET" == "tools" ]]; then
        build_tools
    fi
}

# Build tools
build_tools() {
    log "Building auxiliary tools..."
    # Add tool-specific builds here
}

# Run tests
run_tests() {
    log "Running tests..."
    if [[ ! -f "$OUT_DIR/engc" ]]; then
        err "Compiler not built. Run build first."
        return 1
    fi
    "$OUT_DIR/engc" test
}

# Clean
clean() {
    log "Cleaning build artifacts..."
    rm -rf "$OUT_DIR"
    success "Clean complete"
}

# Install
install() {
    log "Installing to $INSTALL_PREFIX..."
    [[ $EUID -ne 0 ]] && { err "Install requires sudo"; return 1; }
    install -m 755 "$OUT_DIR/engc" "$INSTALL_PREFIX/bin/engc"
    # Install stdlib
    mkdir -p "$INSTALL_PREFIX/share/enginotech/std"
    cp -r std/* "$INSTALL_PREFIX/share/enginotech/std/"
    success "Installed to $INSTALL_PREFIX"
}

# Main
main() {
    case "$TARGET" in
        clean) clean; return ;;
        test) build_all && run_tests; return ;;
        install) build_all && install; return ;;
    esac
    
    [[ $REBUILD -eq 1 ]] && clean
    
    build_all
    success "Build complete: $OUT_DIR/engc (v$VERSION)"
}

main "$@"