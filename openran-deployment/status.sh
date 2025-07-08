#!/bin/bash

# OpenRAN Deployment Status Script
# This script provides a comprehensive status overview of the deployment

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIGS_DIR="$SCRIPT_DIR/configs"
LOGS_DIR="$SCRIPT_DIR/logs"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Check if deployment config exists
if [[ -f "$CONFIGS_DIR/deployment_config.env" ]]; then
    source "$CONFIGS_DIR/deployment_config.env"
else
    echo -e "${RED}❌ No deployment configuration found${NC}"
    echo "Run ./deploy.sh to create a deployment"
    exit 1
fi

# Function to check service status
check_service() {
    local service_name="$1"
    local check_command="$2"
    
    if eval "$check_command" &>/dev/null; then
        echo -e "${GREEN}✅ $service_name${NC}"
        return 0
    else
        echo -e "${RED}❌ $service_name${NC}"
        return 1
    fi
}

# Function to get service info
get_service_info() {
    local service_name="$1"
    local info_command="$2"
    
    local info=$(eval "$info_command" 2>/dev/null || echo "N/A")
    echo -e "${BLUE}   $service_name:${NC} $info"
}

# Display header
echo -e "${GREEN}🚀 OpenRAN Deployment Status${NC}"
echo -e "${GREEN}=============================${NC}"
echo -e "${BLUE}Deployment Type:${NC} $DEPLOYMENT_TYPE"
echo -e "${BLUE}Deployed on:${NC} $DEPLOYMENT_DATE"
echo ""

# Check RIC status
echo -e "${YELLOW}📡 RIC Status${NC}"
case $DEPLOYMENT_TYPE in
    "oran-sc"|"both")
        check_service "O-RAN SC RIC E2 Manager" "docker ps | grep -q ric-e2mgr"
        check_service "O-RAN SC RIC Database" "docker ps | grep -q ric-db"
        check_service "O-RAN SC RIC Redis" "docker ps | grep -q ric-redis"
        get_service_info "E2 Port" "nc -z $RIC_IP $RIC_E2_PORT && echo 'Open' || echo 'Closed'"
        ;;
    "flexric-oai"|"both")
        check_service "FlexRIC nearRT-RIC" "pgrep -f nearRT-RIC"
        check_service "FlexRIC Service Models" "ls /usr/local/lib/flexric/ | wc -l | xargs echo 'files'"
        get_service_info "E2 Port" "nc -z $RIC_IP $RIC_E2_PORT && echo 'Open' || echo 'Closed'"
        ;;
esac
echo ""

# Check gNB status
echo -e "${YELLOW}📡 gNB Status${NC}"
case $DEPLOYMENT_TYPE in
    "oran-sc"|"both")
        check_service "srsRAN gNB" "pgrep -f gnb"
        get_service_info "gNB PID" "pgrep -f gnb || echo 'Not running'"
        get_service_info "Config File" "ls -la $CONFIGS_DIR/gnb_config.yaml | awk '{print \$5 \" bytes\"}' || echo 'Not found'"
        ;;
    "flexric-oai"|"both")
        check_service "OAI gNB" "pgrep -f nr-softmodem"
        get_service_info "gNB PID" "pgrep -f nr-softmodem || echo 'Not running'"
        get_service_info "Config File" "ls -la $CONFIGS_DIR/gnb_oai_config.conf | awk '{print \$5 \" bytes\"}' || echo 'Not found'"
        ;;
esac
echo ""

# Check Core Network status
echo -e "${YELLOW}🏗️ Core Network Status${NC}"
case $DEPLOYMENT_TYPE in
    "oran-sc"|"both")
        check_service "Open5GS Core" "docker ps | grep -q 5gc"
        get_service_info "AMF" "nc -z 127.0.0.1 38412 && echo 'Running' || echo 'Not accessible'"
        ;;
    "flexric-oai"|"both")
        check_service "Core Network" "ping -c 1 -W 1 10.45.1.1"
        get_service_info "Core IP" "echo '10.45.1.1'"
        ;;
esac
echo ""

# Check UE status
echo -e "${YELLOW}📱 UE Status${NC}"
for i in $(seq 1 $NUM_UES); do
    check_service "UE $i namespace" "ip netns list | grep -q ue$i"
    
    if ip netns list | grep -q "ue$i"; then
        get_service_info "UE $i IP" "sudo ip netns exec ue$i ip addr show | grep 'inet ' | grep tun | awk '{print \$2}' | head -1 || echo 'Not assigned'"
        get_service_info "UE $i connectivity" "sudo ip netns exec ue$i ping -c 1 -W 1 10.45.1.1 >/dev/null 2>&1 && echo 'Connected' || echo 'Disconnected'"
    fi
done
echo ""

# Check xApps status
if [[ "$ENABLE_XAPPS" == "y" ]]; then
    echo -e "${YELLOW}🎯 xApps Status${NC}"
    case $DEPLOYMENT_TYPE in
        "oran-sc"|"both")
            check_service "xApp Onboarder" "docker ps | grep -q xapp-onboarder"
            get_service_info "xApp Registry" "curl -s http://localhost:8888/health | jq -r '.status' 2>/dev/null || echo 'Not accessible'"
            ;;
        "flexric-oai"|"both")
            check_service "KPM xApp" "pgrep -f xapp_kmp_moni"
            check_service "RC xApp" "pgrep -f xapp_rc_moni"
            check_service "GTP xApp" "pgrep -f xapp_gtp_mac_rlc_pdcp_moni"
            ;;
    esac
    echo ""
fi

# Check monitoring status
if [[ "$ENABLE_MONITORING" == "y" ]]; then
    echo -e "${YELLOW}📊 Monitoring Status${NC}"
    check_service "Prometheus" "docker ps | grep -q prometheus"
    check_service "Grafana" "docker ps | grep -q grafana"
    check_service "Node Exporter" "docker ps | grep -q node-exporter"
    get_service_info "Prometheus UI" "curl -s http://localhost:9090/api/v1/status/config >/dev/null 2>&1 && echo 'http://localhost:9090' || echo 'Not accessible'"
    get_service_info "Grafana UI" "curl -s http://localhost:3000/api/health >/dev/null 2>&1 && echo 'http://localhost:3000' || echo 'Not accessible'"
    echo ""
fi

# Check system resources
echo -e "${YELLOW}💻 System Resources${NC}"
get_service_info "CPU Usage" "top -bn1 | grep 'Cpu(s)' | awk '{print \$2}' | sed 's/%us,//'"
get_service_info "Memory Usage" "free -h | awk 'NR==2{printf \"%.1f%%\", \$3/\$2*100}'"
get_service_info "Disk Usage" "df -h . | awk 'NR==2{print \$5}'"
get_service_info "Network Interfaces" "ip addr show | grep -E '^[0-9]+:' | wc -l"
echo ""

# Check log files
echo -e "${YELLOW}📋 Log Files${NC}"
if [[ -d "$LOGS_DIR" ]]; then
    get_service_info "Log Directory" "ls -la $LOGS_DIR | wc -l | xargs echo 'files'"
    get_service_info "Latest Log" "ls -t $LOGS_DIR/*.log 2>/dev/null | head -1 | xargs basename || echo 'No logs'"
    get_service_info "Log Size" "du -sh $LOGS_DIR 2>/dev/null | awk '{print \$1}' || echo 'N/A'"
else
    echo -e "${RED}❌ Log directory not found${NC}"
fi
echo ""

# E2 Interface status
echo -e "${YELLOW}🔗 E2 Interface Status${NC}"
get_service_info "E2 Port Status" "netstat -tlnp | grep :$RIC_E2_PORT | awk '{print \"Listening on \" \$4}' || echo 'Not listening'"
get_service_info "E2 Connections" "netstat -tnp | grep :$RIC_E2_PORT | wc -l | xargs echo 'active connections'"

# Check for E2 setup messages in logs
case $DEPLOYMENT_TYPE in
    "oran-sc"|"both")
        get_service_info "E2 Setup (O-RAN SC)" "docker logs ric-e2mgr 2>/dev/null | grep -i 'e2 setup' | tail -1 | cut -c1-50 || echo 'No E2 setup found'"
        ;;
    "flexric-oai"|"both")
        get_service_info "E2 Setup (FlexRIC)" "grep -i 'e2 setup' /tmp/flexric.log 2>/dev/null | tail -1 | cut -c1-50 || echo 'No E2 setup found'"
        ;;
esac
echo ""

# Quick actions
echo -e "${YELLOW}🛠️ Quick Actions${NC}"
echo "View logs:           tail -f $LOGS_DIR/deployment.log"
echo "Run tests:           ./scripts/run_tests.sh"
echo "Monitor UEs:         ./configs/monitor_ues.sh"
echo "Stop deployment:     ./stop.sh"
echo "Restart deployment:  ./restart.sh"
echo "Clean deployment:    ./clean.sh"

if [[ "$ENABLE_MONITORING" == "y" ]]; then
    echo ""
    echo -e "${YELLOW}📊 Monitoring URLs${NC}"
    echo "Prometheus:          http://localhost:9090"
    echo "Grafana:             http://localhost:3000 (admin/admin)"
    echo "Node Exporter:       http://localhost:9100"
fi

echo ""
echo -e "${GREEN}Status check completed at $(date)${NC}"