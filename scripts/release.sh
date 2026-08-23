#!/usr/bin/env bash
# EnginotechC++ — Release script
# Usage: ./scripts/release.sh [version] [dist_dir]
# Example: ./scripts/release.sh 0.2.2 /tmp/releases
set -euo pipefail

VERSION="${1:-$(cat VERSION)}"
DIST_DIR="${2:-/tmp/releases}"
REPO="enginotech/enginotech-cpp"

echo "=== Release v${VERSION} ==="
echo "Repo: ${REPO}"
echo "Dist: ${DIST_DIR}"

# Create dist dirs
mkdir -p "${DIST_DIR}/linux" "${DIST_DIR}/windows" "${DIST_DIR}/macos"

# ---- Linux (native build) ----
echo "[1/3] Building Linux binary..."
ARCH=$(uname -m)
if [[ "${ARCH}" == "x86_64" ]]; then
    ARCH_TAG="x86_64"
elif [[ "${ARCH}" == "aarch64" || "${ARCH}" == "arm64" ]]; then
    ARCH_TAG="arm64"
else
    ARCH_TAG="${ARCH}"
fi

./scripts/build.sh "${DIST_DIR}/linux/engc-linux-${ARCH_TAG}" 2>&1 | tail -1
chmod +x "${DIST_DIR}/linux/engc-linux-${ARCH_TAG}"
echo "  → linux/${ARCH_TAG}: ${DIST_DIR}/linux/engc-linux-${ARCH_TAG}"

# ---- macOS (native build) ----
if [[ "$(uname)" == "Darwin" ]]; then
    echo "[2/3] Building macOS binary..."
    ./scripts/build.sh "${DIST_DIR}/macos/engc-macos-$(uname -m)" 2>&1 | tail -1
    chmod +x "${DIST_DIR}/macos/engc-macos-$(uname -m)"
    echo "  → macos/$(uname -m): ${DIST_DIR}/macos/engc-macos-$(uname -m)"
else
    echo "[2/3] Skip macOS (not on Darwin) — cross-compile requires osxcross"
fi

# ---- Windows (requires WSL2 or cross-compile) ----
if command -v x86_64-w64-mingw32-g++ &>/dev/null; then
    echo "[3/3] Building Windows binary..."
    CXX=x86_64-w64-mingw32-g++ ./scripts/build.sh "${DIST_DIR}/windows/engc-win.exe" 2>&1 | tail -1
    echo "  → windows/engc-win.exe"
else
    echo "[3/3] Skip Windows (no mingw-w64 toolchain found)"
fi

# ---- Create archive ----
echo ""
echo "=== Creating archives ==="
cd "${DIST_DIR}"
TIMESTAMP=$(date +%Y%m%d)

for platform in linux macos windows; do
    if ls "${platform}"/* &>/dev/null 2>&1; then
        if [[ "${platform}" == "windows" ]]; then
            zip -rq "engc-${VERSION}-${platform}.zip" "${platform}/" 2>/dev/null || true
        else
            tar czf "engc-${VERSION}-${platform}.tar.gz" "${platform}/"
        fi
        echo "  → engc-${VERSION}-${platform}.tar.gz"
    fi
done

# ---
echo ""
echo "=== Release v${VERSION} ready at ${DIST_DIR} ==="
ls -la "${DIST_DIR}"/*.tar.gz "${DIST_DIR}"/*.zip 2>/dev/null || true
echo ""
echo "Next steps:"
echo "  1. git tag v${VERSION}"
echo "  2. git push origin v${VERSION}"
echo "  3. gh release create v${VERSION} ${DIST_DIR}/*.tar.gz ${DIST_DIR}/*.zip --title 'v${VERSION}' --generate-notes"
