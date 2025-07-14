#!/bin/bash

# Test script for Smart Monitor xApp
# This script runs all unit tests and integration tests

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build"

echo "🧪 Running Smart Monitor xApp Tests..."

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if tests are built
if [ ! -d "$BUILD_DIR" ]; then
    echo -e "${RED}❌ Build directory not found. Please run build.sh first:${NC}"
    echo "  ./scripts/build.sh"
    exit 1
fi

cd "$BUILD_DIR"

# Function to run a test and report results
run_test() {
    local test_name="$1"
    local test_executable="$2"
    
    echo -e "${YELLOW}🔍 Running $test_name...${NC}"
    
    if [ ! -f "$test_executable" ]; then
        echo -e "${RED}❌ Test executable not found: $test_executable${NC}"
        return 1
    fi
    
    if "$test_executable"; then
        echo -e "${GREEN}✅ $test_name passed${NC}"
        return 0
    else
        echo -e "${RED}❌ $test_name failed${NC}"
        return 1
    fi
}

# Test results
TESTS_PASSED=0
TESTS_FAILED=0

echo -e "${YELLOW}📋 Test Summary${NC}"
echo "================================"

# Run analytics tests
if run_test "Analytics Tests" "./tests/test_analytics"; then
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

# Run database tests
if run_test "Database Tests" "./tests/test_database"; then
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

# Run integration test (quick xApp test)
echo -e "${YELLOW}🔍 Running Integration Test...${NC}"

# Start xApp for 5 seconds to test basic functionality
if XAPP_DURATION=5 timeout 10s ./smart_monitor_xapp > /dev/null 2>&1; then
    echo -e "${GREEN}✅ Integration Test passed${NC}"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo -e "${YELLOW}⚠️ Integration Test skipped (nearRT-RIC not available)${NC}"
    echo "  This is normal if FlexRIC is not running"
fi

# Memory leak test (if valgrind is available)
if command -v valgrind >/dev/null 2>&1; then
    echo -e "${YELLOW}🔍 Running Memory Leak Test...${NC}"
    
    if valgrind --tool=memcheck --leak-check=full --error-exitcode=1 --quiet \
       timeout 5s ./smart_monitor_xapp >/dev/null 2>&1; then
        echo -e "${GREEN}✅ Memory Leak Test passed${NC}"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}❌ Memory Leak Test failed${NC}"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
else
    echo -e "${YELLOW}⚠️ Valgrind not available, skipping memory leak test${NC}"
fi

echo ""
echo "================================"
echo -e "${YELLOW}📊 Test Results${NC}"
echo "================================"

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${GREEN}🎉 All tests passed! ($TESTS_PASSED/$((TESTS_PASSED + TESTS_FAILED)))${NC}"
    exit 0
else
    echo -e "${RED}❌ Some tests failed: $TESTS_FAILED failed, $TESTS_PASSED passed${NC}"
    exit 1
fi