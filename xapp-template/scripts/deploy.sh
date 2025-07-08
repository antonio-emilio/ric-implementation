#!/bin/bash

# Deployment script for Smart Monitor xApp
# This script deploys the xApp to work with the existing FlexRIC infrastructure

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build"

echo "🚀 Deploying Smart Monitor xApp..."

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Check if xApp is built
if [ ! -f "$BUILD_DIR/smart_monitor_xapp" ]; then
    echo -e "${RED}❌ xApp not built. Please run build.sh first:${NC}"
    echo "  ./scripts/build.sh"
    exit 1
fi

# Check if FlexRIC is running
echo -e "${YELLOW}🔍 Checking FlexRIC nearRT-RIC status...${NC}"
if ! pgrep -f "nearRT-RIC" > /dev/null; then
    echo -e "${YELLOW}⚠️ nearRT-RIC is not running${NC}"
    echo -e "${BLUE}💡 To start nearRT-RIC, run one of:${NC}"
    echo "  1. From FlexRIC build directory:"
    echo "     ./build/examples/ric/nearRT-RIC &"
    echo "  2. From main repository:"
    echo "     ./setup_flexric.sh"
    echo "  3. From main repository (full setup):"
    echo "     ./deploy_flexric_oai_e2.sh"
    echo ""
    echo -e "${YELLOW}🚀 Starting nearRT-RIC automatically...${NC}"
    
    # Try to find and start nearRT-RIC
    FLEXRIC_PATHS=(
        "$HOME/flexric/build/examples/ric/nearRT-RIC"
        "/usr/local/bin/nearRT-RIC"
        "$(find /home -name "nearRT-RIC" -type f -executable 2>/dev/null | head -1)"
    )
    
    NEARRT_RIC_FOUND=false
    for path in "${FLEXRIC_PATHS[@]}"; do
        if [ -f "$path" ] && [ -x "$path" ]; then
            echo -e "${GREEN}✅ Found nearRT-RIC at: $path${NC}"
            nohup "$path" > /tmp/nearRT-RIC.log 2>&1 &
            NEARRT_RIC_FOUND=true
            break
        fi
    done
    
    if [ "$NEARRT_RIC_FOUND" = false ]; then
        echo -e "${RED}❌ nearRT-RIC not found. Please install FlexRIC first.${NC}"
        echo "Run: ./setup_flexric.sh"
        exit 1
    fi
    
    echo -e "${YELLOW}⏳ Waiting for nearRT-RIC to start...${NC}"
    sleep 5
    
    if ! pgrep -f "nearRT-RIC" > /dev/null; then
        echo -e "${RED}❌ Failed to start nearRT-RIC${NC}"
        echo "Check logs: tail -f /tmp/nearRT-RIC.log"
        exit 1
    fi
    
    echo -e "${GREEN}✅ nearRT-RIC started successfully${NC}"
else
    echo -e "${GREEN}✅ nearRT-RIC is already running${NC}"
fi

# Check network connectivity
echo -e "${YELLOW}🔍 Checking network connectivity...${NC}"
if command -v nc >/dev/null 2>&1; then
    if nc -z 127.0.0.1 36421 2>/dev/null; then
        echo -e "${GREEN}✅ nearRT-RIC is accepting connections on port 36421${NC}"
    else
        echo -e "${YELLOW}⚠️ nearRT-RIC port 36421 not accessible${NC}"
        echo "This may be normal if no E2 nodes are connected yet"
    fi
else
    echo -e "${YELLOW}⚠️ netcat not available for connectivity check${NC}"
fi

# Create log directory
LOG_DIR="/tmp/xapp_logs"
mkdir -p "$LOG_DIR"

# Copy configuration files to a runtime location
RUNTIME_CONFIG_DIR="/tmp/xapp_config"
mkdir -p "$RUNTIME_CONFIG_DIR"
cp -r "$PROJECT_DIR/config/"* "$RUNTIME_CONFIG_DIR/"

echo -e "${GREEN}✅ Configuration copied to: $RUNTIME_CONFIG_DIR${NC}"

# Display deployment information
echo -e "${BLUE}📋 Deployment Information:${NC}"
echo "  🎯 xApp executable: $BUILD_DIR/smart_monitor_xapp"
echo "  📁 Configuration: $RUNTIME_CONFIG_DIR"
echo "  📋 Log directory: $LOG_DIR"
echo "  🔗 RIC endpoint: 127.0.0.1:36421"
echo "  💾 Database: /tmp/xapp_data.db"
echo "  📊 Main log: /tmp/smart_monitor_xapp.log"

echo ""
echo -e "${GREEN}🎉 Deployment completed successfully!${NC}"
echo ""
echo -e "${YELLOW}📋 Usage Examples:${NC}"
echo "  # Run for 60 seconds"
echo "  XAPP_DURATION=60 $BUILD_DIR/smart_monitor_xapp"
echo ""
echo "  # Run indefinitely"
echo "  $BUILD_DIR/smart_monitor_xapp"
echo ""
echo "  # Run with debug logging"
echo "  XAPP_DEBUG=1 $BUILD_DIR/smart_monitor_xapp"
echo ""
echo "  # Run in background"
echo "  nohup $BUILD_DIR/smart_monitor_xapp > $LOG_DIR/xapp.log 2>&1 &"
echo ""
echo -e "${YELLOW}📋 Integration with existing scripts:${NC}"
echo "  # Add to setup_flexric.sh"
echo "  XAPP_DURATION=30 $BUILD_DIR/smart_monitor_xapp &"
echo ""
echo "  # Add to deploy_flexric_oai_e2.sh"
echo "  XAPP_DURATION=30 $BUILD_DIR/smart_monitor_xapp &"
echo ""
echo -e "${YELLOW}📋 Monitoring:${NC}"
echo "  # Check if xApp is running"
echo "  ps aux | grep smart_monitor_xapp"
echo ""
echo "  # View real-time logs"
echo "  tail -f /tmp/smart_monitor_xapp.log"
echo ""
echo "  # View metrics database"
echo "  sqlite3 /tmp/xapp_data.db 'SELECT * FROM metrics LIMIT 10;'"
echo ""
echo -e "${YELLOW}📋 Troubleshooting:${NC}"
echo "  # Check nearRT-RIC logs"
echo "  tail -f /tmp/nearRT-RIC.log"
echo ""
echo "  # Check xApp connectivity"
echo "  netstat -an | grep 36421"
echo ""
echo "  # Stop xApp"
echo "  pkill smart_monitor_xapp"

echo -e "${GREEN}✅ Ready to deploy! Run the xApp using the examples above.${NC}"