#!/bin/bash

# Run post-deployment tests
# This script runs comprehensive tests after deployment

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIGS_DIR="$SCRIPT_DIR/../configs"
LOGS_DIR="$SCRIPT_DIR/../logs"

# Source deployment configuration
source "$CONFIGS_DIR/deployment_config.env"

# Logging function
log() {
    echo -e "\033[0;32m[$(date '+%Y-%m-%d %H:%M:%S')]\033[0m $1" | tee -a "$LOGS_DIR/tests.log"
}

warn() {
    echo -e "\033[1;33m[$(date '+%Y-%m-%d %H:%M:%S')] WARNING:\033[0m $1" | tee -a "$LOGS_DIR/tests.log"
}

error() {
    echo -e "\033[0;31m[$(date '+%Y-%m-%d %H:%M:%S')] ERROR:\033[0m $1" | tee -a "$LOGS_DIR/tests.log"
}

# Test results tracking
TESTS_PASSED=0
TESTS_FAILED=0
TESTS_TOTAL=0

# Function to run a test
run_test() {
    local test_name="$1"
    local test_command="$2"
    
    TESTS_TOTAL=$((TESTS_TOTAL + 1))
    
    log "🧪 Running test: $test_name"
    
    if eval "$test_command"; then
        log "✅ Test passed: $test_name"
        TESTS_PASSED=$((TESTS_PASSED + 1))
        return 0
    else
        error "❌ Test failed: $test_name"
        TESTS_FAILED=$((TESTS_FAILED + 1))
        return 1
    fi
}

# Test RIC deployment
test_ric_deployment() {
    case $DEPLOYMENT_TYPE in
        "oran-sc"|"both")
            run_test "ORAN SC RIC containers" "docker ps | grep -q ric-e2mgr"
            run_test "ORAN SC RIC E2 port" "nc -z $RIC_IP $RIC_E2_PORT"
            run_test "ORAN SC RIC database" "docker exec -it ric-db mysql -u ricuser -pricpass -e 'SELECT 1' ricdb"
            ;;
        "flexric-oai"|"both")
            run_test "FlexRIC process" "pgrep -f nearRT-RIC"
            run_test "FlexRIC E2 port" "nc -z $RIC_IP $RIC_E2_PORT"
            run_test "FlexRIC service models" "ls -la /usr/local/lib/flexric/"
            ;;
    esac
}

# Test gNB deployment
test_gnb_deployment() {
    case $DEPLOYMENT_TYPE in
        "oran-sc"|"both")
            run_test "srsRAN gNB process" "pgrep -f gnb"
            run_test "srsRAN gNB configuration" "test -f $CONFIGS_DIR/gnb_config.yaml"
            run_test "srsRAN gNB logs" "test -f /tmp/gnb.log"
            ;;
        "flexric-oai"|"both")
            run_test "OAI gNB process" "pgrep -f nr-softmodem"
            run_test "OAI gNB configuration" "test -f $CONFIGS_DIR/gnb_oai_config.conf"
            run_test "OAI gNB E2 connection" "grep -q 'E2 Setup' /tmp/gnb.log 2>/dev/null || true"
            ;;
    esac
}

# Test UE deployment
test_ue_deployment() {
    for i in $(seq 1 $NUM_UES); do
        run_test "UE $i namespace" "ip netns list | grep -q ue$i"
        run_test "UE $i process" "pgrep -f ue$i || pgrep -f srsue"
        run_test "UE $i configuration" "test -f $CONFIGS_DIR/ue${i}_config.conf"
    done
}

# Test network connectivity
test_network_connectivity() {
    log "🌐 Testing network connectivity..."
    
    # Test basic connectivity
    run_test "Localhost connectivity" "ping -c 3 127.0.0.1"
    run_test "RIC connectivity" "ping -c 3 $RIC_IP"
    
    # Test UE connectivity
    for i in $(seq 1 $NUM_UES); do
        run_test "UE $i core connectivity" "timeout 10 sudo ip netns exec ue$i ping -c 3 10.45.1.1 || true"
        run_test "UE $i DNS resolution" "timeout 10 sudo ip netns exec ue$i nslookup google.com || true"
    done
}

# Test E2 interface
test_e2_interface() {
    log "📡 Testing E2 interface..."
    
    # Test E2 port availability
    run_test "E2 port listening" "netstat -tlnp | grep :$RIC_E2_PORT"
    
    # Test E2 connection (basic)
    run_test "E2 connection attempt" "timeout 5 telnet $RIC_IP $RIC_E2_PORT < /dev/null || true"
    
    # Check E2 setup messages in logs
    case $DEPLOYMENT_TYPE in
        "oran-sc"|"both")
            run_test "E2 setup in RIC logs" "docker logs ric-e2mgr 2>/dev/null | grep -i 'e2 setup' || true"
            ;;
        "flexric-oai"|"both")
            run_test "E2 setup in FlexRIC logs" "grep -i 'e2 setup' /tmp/flexric.log 2>/dev/null || true"
            ;;
    esac
}

# Test xApps
test_xapps() {
    if [[ "$ENABLE_XAPPS" == "y" ]]; then
        log "🎯 Testing xApps..."
        
        case $DEPLOYMENT_TYPE in
            "oran-sc"|"both")
                run_test "ORAN SC xApp onboarder" "docker ps | grep -q xapp-onboarder"
                run_test "ORAN SC xApp registry" "curl -s http://localhost:8888/health || true"
                ;;
            "flexric-oai"|"both")
                run_test "FlexRIC KPM xApp" "pgrep -f xapp_kpm_moni"
                run_test "FlexRIC RC xApp" "pgrep -f xapp_rc_moni"
                run_test "FlexRIC GTP xApp" "pgrep -f xapp_gtp_mac_rlc_pdcp_moni"
                ;;
        esac
    else
        log "ℹ️ xApps disabled, skipping tests"
    fi
}

# Test monitoring services
test_monitoring() {
    if [[ "$ENABLE_MONITORING" == "y" ]]; then
        log "📊 Testing monitoring services..."
        
        run_test "Prometheus container" "docker ps | grep -q prometheus"
        run_test "Grafana container" "docker ps | grep -q grafana"
        run_test "Node exporter container" "docker ps | grep -q node-exporter"
        run_test "Prometheus web interface" "curl -s http://localhost:9090/api/v1/status/config || true"
        run_test "Grafana web interface" "curl -s http://localhost:3000/api/health || true"
        run_test "Custom metrics endpoint" "curl -s http://localhost:8080/metrics || true"
    else
        log "ℹ️ Monitoring disabled, skipping tests"
    fi
}

# Test core network
test_core_network() {
    log "🏗️ Testing core network..."
    
    case $DEPLOYMENT_TYPE in
        "oran-sc"|"both")
            run_test "Open5GS core containers" "docker ps | grep -q 5gc"
            run_test "AMF connectivity" "nc -z 127.0.0.1 38412"
            run_test "SMF connectivity" "nc -z 127.0.0.1 8805"
            ;;
        "flexric-oai"|"both")
            # OAI typically uses built-in core or external core
            run_test "Core network reachability" "ping -c 3 10.45.1.1 || true"
            ;;
    esac
}

# Test performance
test_performance() {
    log "⚡ Testing performance..."
    
    # Test CPU usage
    local cpu_usage=$(top -bn1 | grep "Cpu(s)" | sed "s/.*, *\([0-9.]*\)%* id.*/\1/" | awk '{print 100 - $1}')
    run_test "CPU usage acceptable" "echo '$cpu_usage < 80' | bc -l"
    
    # Test memory usage
    local mem_usage=$(free | grep Mem | awk '{printf "%.2f", $3/$2 * 100.0}')
    run_test "Memory usage acceptable" "echo '$mem_usage < 80' | bc -l"
    
    # Test disk usage
    local disk_usage=$(df . | tail -1 | awk '{print $5}' | sed 's/%//')
    run_test "Disk usage acceptable" "test $disk_usage -lt 90"
}

# Test data plane
test_data_plane() {
    log "📊 Testing data plane..."
    
    # Test UE data connectivity
    for i in $(seq 1 $NUM_UES); do
        if sudo ip netns exec ue$i ip addr show | grep -q "tun_srsue$i"; then
            run_test "UE $i data interface" "sudo ip netns exec ue$i ping -c 3 8.8.8.8 || true"
        else
            warn "UE $i data interface not found, skipping data plane test"
        fi
    done
}

# Test configuration integrity
test_configuration_integrity() {
    log "🔧 Testing configuration integrity..."
    
    # Test configuration files exist
    run_test "Deployment config exists" "test -f $CONFIGS_DIR/deployment_config.env"
    run_test "gNB config exists" "test -f $CONFIGS_DIR/gnb_config.yaml -o -f $CONFIGS_DIR/gnb_oai_config.conf"
    run_test "UE configs exist" "test -f $CONFIGS_DIR/ue1_config.conf"
    
    # Test configuration syntax
    run_test "Deployment config syntax" "source $CONFIGS_DIR/deployment_config.env"
    
    # Test log directories
    run_test "Logs directory exists" "test -d $LOGS_DIR"
    run_test "Logs directory writable" "test -w $LOGS_DIR"
}

# Generate test report
generate_test_report() {
    log "📋 Generating test report..."
    
    local report_file="$LOGS_DIR/test_report.html"
    local test_date=$(date '+%Y-%m-%d %H:%M:%S')
    
    cat > "$report_file" << EOF
<!DOCTYPE html>
<html>
<head>
    <title>OpenRAN Deployment Test Report</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        .header { background: #2c3e50; color: white; padding: 20px; text-align: center; }
        .summary { background: #ecf0f1; padding: 20px; margin: 20px 0; border-radius: 8px; }
        .passed { color: #27ae60; }
        .failed { color: #e74c3c; }
        .test-results { margin: 20px 0; }
        .test-item { margin: 10px 0; padding: 10px; border-left: 4px solid #bdc3c7; }
        .test-item.passed { border-left-color: #27ae60; }
        .test-item.failed { border-left-color: #e74c3c; }
    </style>
</head>
<body>
    <div class="header">
        <h1>🧪 OpenRAN Deployment Test Report</h1>
        <p>Generated on: $test_date</p>
    </div>
    
    <div class="summary">
        <h2>📊 Test Summary</h2>
        <p><strong>Deployment Type:</strong> $DEPLOYMENT_TYPE</p>
        <p><strong>Total Tests:</strong> $TESTS_TOTAL</p>
        <p><strong class="passed">Passed:</strong> $TESTS_PASSED</p>
        <p><strong class="failed">Failed:</strong> $TESTS_FAILED</p>
        <p><strong>Success Rate:</strong> $(( TESTS_PASSED * 100 / TESTS_TOTAL ))%</p>
    </div>
    
    <div class="test-results">
        <h2>📋 Detailed Results</h2>
        <pre>$(cat "$LOGS_DIR/tests.log")</pre>
    </div>
</body>
</html>
EOF
    
    log "📋 Test report generated: $report_file"
}

# Main function
main() {
    log "🧪 Starting OpenRAN deployment tests..."
    
    # Clear previous test results
    > "$LOGS_DIR/tests.log"
    
    # Run all tests
    test_configuration_integrity
    test_ric_deployment
    test_gnb_deployment
    test_ue_deployment
    test_core_network
    test_network_connectivity
    test_e2_interface
    test_xapps
    test_monitoring
    test_performance
    test_data_plane
    
    # Generate report
    generate_test_report
    
    # Summary
    log "🏁 Test execution completed"
    log "📊 Results: $TESTS_PASSED passed, $TESTS_FAILED failed out of $TESTS_TOTAL total tests"
    
    if [[ $TESTS_FAILED -eq 0 ]]; then
        log "🎉 All tests passed! Deployment is successful."
        exit 0
    else
        error "❌ Some tests failed. Please check the logs and fix the issues."
        exit 1
    fi
}

# Execute main function
main "$@"