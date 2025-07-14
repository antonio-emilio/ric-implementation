#!/bin/bash

# Build script for Smart Monitor xApp
# This script builds the xApp with all dependencies

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build"

echo "🏗️ Building Smart Monitor xApp..."

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check dependencies
echo -e "${YELLOW}📦 Checking dependencies...${NC}"

# Check if FlexRIC is installed
if [ ! -d "/usr/local/lib/flexric" ]; then
    echo -e "${RED}❌ FlexRIC not found. Please install FlexRIC first using:${NC}"
    echo "  ./setup_flexric.sh"
    exit 1
fi

# Check required libraries
MISSING_DEPS=()

if ! pkg-config --exists sqlite3; then
    MISSING_DEPS+=("libsqlite3-dev")
fi

if ! pkg-config --exists json-c; then
    MISSING_DEPS+=("libjson-c-dev")
fi

if ! pkg-config --exists libcurl; then
    MISSING_DEPS+=("libcurl4-openssl-dev")
fi

if [ ${#MISSING_DEPS[@]} -gt 0 ]; then
    echo -e "${RED}❌ Missing dependencies:${NC}"
    for dep in "${MISSING_DEPS[@]}"; do
        echo "  - $dep"
    done
    echo -e "${YELLOW}Install them with:${NC}"
    echo "  sudo apt update"
    echo "  sudo apt install -y ${MISSING_DEPS[*]}"
    exit 1
fi

echo -e "${GREEN}✅ All dependencies found${NC}"

# Create build directory
echo -e "${YELLOW}📁 Creating build directory...${NC}"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with CMake
echo -e "${YELLOW}⚙️ Configuring build...${NC}"
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DCMAKE_INSTALL_PREFIX=/usr/local \
         -DFLEXRIC_PATH=$HOME/CERISE/ric/flexric \
         -DBUILD_TESTS=ON

# Build the project
echo -e "${YELLOW}🔨 Building project...${NC}"
make -j$(nproc)

# Build tests
echo -e "${YELLOW}🧪 Building tests...${NC}"
make -j$(nproc) tests

echo -e "${GREEN}✅ Build completed successfully!${NC}"

# Display build results
echo -e "${GREEN}📋 Build Results:${NC}"
echo "  📁 Build directory: $BUILD_DIR"
echo "  🚀 Main executable: $BUILD_DIR/smart_monitor_xapp"
echo "  🧪 Test executables: $BUILD_DIR/tests/"
echo "  📚 Configuration: $PROJECT_DIR/config/"

# Check if executable exists and is runnable
if [ -f "$BUILD_DIR/smart_monitor_xapp" ]; then
    echo -e "${GREEN}✅ Executable created successfully${NC}"
    echo -e "${YELLOW}📋 To run the xApp:${NC}"
    echo "  cd $PROJECT_DIR"
    echo "  ./build/smart_monitor_xapp"
    echo ""
    echo -e "${YELLOW}📋 To run with custom duration:${NC}"
    echo "  XAPP_DURATION=60 ./build/smart_monitor_xapp"
    echo ""
    echo -e "${YELLOW}📋 To run tests:${NC}"
    echo "  ./scripts/run_tests.sh"
else
    echo -e "${RED}❌ Build failed - executable not found${NC}"
    exit 1
fi

echo -e "${GREEN}🎉 Build process completed successfully!${NC}"