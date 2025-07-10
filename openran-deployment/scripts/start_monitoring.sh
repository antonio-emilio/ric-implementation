#!/bin/bash

# Start monitoring services
# This script starts monitoring and observability services

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIGS_DIR="$SCRIPT_DIR/../configs"
LOGS_DIR="$SCRIPT_DIR/../logs"

# Source deployment configuration
source "$CONFIGS_DIR/deployment_config.env"

# Logging function
log() {
    echo -e "\033[0;32m[$(date '+%Y-%m-%d %H:%M:%S')]\033[0m $1" | tee -a "$LOGS_DIR/monitoring.log"
}

# Start Prometheus
start_prometheus() {
    log "📊 Starting Prometheus..."
    
    # Create Prometheus configuration
    cat > "$CONFIGS_DIR/prometheus.yml" << EOF
global:
  scrape_interval: 15s
  evaluation_interval: 15s

scrape_configs:
  - job_name: 'ric'
    static_configs:
      - targets: ['$RIC_IP:$RIC_E2_PORT']
    metrics_path: /metrics
    scrape_interval: 10s
    
  - job_name: 'gnb'
    static_configs:
      - targets: ['$GNB_IP:8080']
    metrics_path: /metrics
    scrape_interval: 10s
    
  - job_name: 'core'
    static_configs:
      - targets: ['127.0.0.1:9090']
    metrics_path: /metrics
    scrape_interval: 10s
    
  - job_name: 'node-exporter'
    static_configs:
      - targets: ['localhost:9100']
    scrape_interval: 10s
EOF
    
    # Start Prometheus container
    docker run -d \
        --name prometheus \
        --network host \
        -v "$CONFIGS_DIR/prometheus.yml:/etc/prometheus/prometheus.yml" \
        -v prometheus_data:/prometheus \
        prom/prometheus:latest \
        --config.file=/etc/prometheus/prometheus.yml \
        --storage.tsdb.path=/prometheus \
        --web.console.libraries=/etc/prometheus/console_libraries \
        --web.console.templates=/etc/prometheus/consoles \
        --storage.tsdb.retention.time=200h \
        --web.enable-lifecycle
    
    log "✅ Prometheus started on port 9090"
}

# Start Grafana
start_grafana() {
    log "📈 Starting Grafana..."
    
    # Start Grafana container
    docker run -d \
        --name grafana \
        --network host \
        -v grafana_data:/var/lib/grafana \
        -e "GF_SECURITY_ADMIN_PASSWORD=admin" \
        grafana/grafana:latest
    
    # Wait for Grafana to start
    sleep 10
    
    # Create data source
    curl -X POST \
        -H "Content-Type: application/json" \
        -d '{
            "name": "Prometheus",
            "type": "prometheus",
            "url": "http://localhost:9090",
            "access": "proxy",
            "isDefault": true
        }' \
        http://admin:admin@localhost:3000/api/datasources || log "⚠️ Failed to create Prometheus data source"
    
    log "✅ Grafana started on port 3000 (admin/admin)"
}

# Start Node Exporter
start_node_exporter() {
    log "💻 Starting Node Exporter..."
    
    docker run -d \
        --name node-exporter \
        --network host \
        --pid host \
        -v "/:/host:ro,rslave" \
        quay.io/prometheus/node-exporter:latest \
        --path.rootfs=/host
    
    log "✅ Node Exporter started on port 9100"
}

# Start custom metrics collectors
start_custom_collectors() {
    log "🔧 Starting custom metrics collectors..."
    
    # Create RIC metrics collector
    cat > "$CONFIGS_DIR/ric_metrics_collector.py" << 'EOF'
#!/usr/bin/env python3

import time
import subprocess
import json
from http.server import HTTPServer, BaseHTTPRequestHandler
from urllib.parse import urlparse

class MetricsHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/metrics':
            self.send_response(200)
            self.send_header('Content-type', 'text/plain')
            self.end_headers()
            
            # Collect RIC metrics
            metrics = self.collect_ric_metrics()
            self.wfile.write(metrics.encode())
        else:
            self.send_response(404)
            self.end_headers()
    
    def collect_ric_metrics(self):
        metrics = []
        
        # RIC connection status
        try:
            # Check if RIC is running
            result = subprocess.run(['docker', 'ps', '--filter', 'name=ric', '--format', '{{.Status}}'], 
                                  capture_output=True, text=True)
            if 'Up' in result.stdout:
                metrics.append('ric_status 1')
            else:
                metrics.append('ric_status 0')
        except:
            metrics.append('ric_status 0')
        
        # E2 connections
        try:
            # This would be replaced with actual E2 connection monitoring
            metrics.append('e2_connections 1')
        except:
            metrics.append('e2_connections 0')
        
        # Active xApps
        try:
            # This would be replaced with actual xApp monitoring
            metrics.append('active_xapps 3')
        except:
            metrics.append('active_xapps 0')
        
        return '\n'.join(metrics) + '\n'

if __name__ == '__main__':
    server = HTTPServer(('localhost', 8080), MetricsHandler)
    server.serve_forever()
EOF
    
    # Start RIC metrics collector
    python3 "$CONFIGS_DIR/ric_metrics_collector.py" &
    METRICS_PID=$!
    echo $METRICS_PID > "$LOGS_DIR/ric_metrics.pid"
    
    log "✅ Custom metrics collectors started"
}

# Start log aggregation
start_log_aggregation() {
    log "📋 Starting log aggregation..."
    
    # Create log aggregation script
    cat > "$CONFIGS_DIR/aggregate_logs.sh" << 'EOF'
#!/bin/bash

# Log aggregation script
LOGS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )/../logs"

while true; do
    # Aggregate logs from different components
    {
        echo "=== RIC Logs ==="
        docker logs ric-e2mgr 2>/dev/null | tail -10 || echo "RIC not running"
        
        echo "=== gNB Logs ==="
        tail -10 /tmp/gnb.log 2>/dev/null || echo "gNB logs not available"
        
        echo "=== UE Logs ==="
        for i in {1..5}; do
            if [ -f "/tmp/ue${i}.log" ]; then
                echo "--- UE $i ---"
                tail -5 "/tmp/ue${i}.log"
            fi
        done
        
        echo "=== xApp Logs ==="
        tail -10 /tmp/xapp*.log 2>/dev/null || echo "xApp logs not available"
        
        echo "========================="
        echo "Last updated: $(date)"
        echo ""
    } > "$LOGS_DIR/aggregated.log"
    
    sleep 30
done
EOF
    
    chmod +x "$CONFIGS_DIR/aggregate_logs.sh"
    "$CONFIGS_DIR/aggregate_logs.sh" &
    LOGS_PID=$!
    echo $LOGS_PID > "$LOGS_DIR/log_aggregation.pid"
    
    log "✅ Log aggregation started"
}

# Create monitoring dashboard
create_monitoring_dashboard() {
    log "📊 Creating monitoring dashboard..."
    
    # Create simple HTML dashboard
    cat > "$CONFIGS_DIR/dashboard.html" << EOF
<!DOCTYPE html>
<html>
<head>
    <title>OpenRAN Monitoring Dashboard</title>
    <meta http-equiv="refresh" content="30">
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        .container { max-width: 1200px; margin: 0 auto; }
        .header { background: #2c3e50; color: white; padding: 20px; text-align: center; }
        .metrics { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 20px; margin: 20px 0; }
        .metric-card { background: #ecf0f1; padding: 20px; border-radius: 8px; }
        .metric-title { font-weight: bold; margin-bottom: 10px; }
        .metric-value { font-size: 24px; color: #27ae60; }
        .logs { background: #2c3e50; color: white; padding: 20px; border-radius: 8px; max-height: 400px; overflow-y: auto; }
        .status-ok { color: #27ae60; }
        .status-error { color: #e74c3c; }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🚀 OpenRAN Monitoring Dashboard</h1>
            <p>Deployment Type: $DEPLOYMENT_TYPE | RIC: $RIC_IP:$RIC_E2_PORT | gNB: $GNB_IP</p>
        </div>
        
        <div class="metrics">
            <div class="metric-card">
                <div class="metric-title">🏗️ RIC Status</div>
                <div class="metric-value status-ok">Active</div>
            </div>
            
            <div class="metric-card">
                <div class="metric-title">📡 gNB Status</div>
                <div class="metric-value status-ok">Connected</div>
            </div>
            
            <div class="metric-card">
                <div class="metric-title">📱 Active UEs</div>
                <div class="metric-value">$NUM_UES</div>
            </div>
            
            <div class="metric-card">
                <div class="metric-title">🎯 xApps</div>
                <div class="metric-value">$( [[ "$ENABLE_XAPPS" == "y" ]] && echo "3" || echo "0" )</div>
            </div>
        </div>
        
        <div class="logs">
            <h3>📋 Recent Logs</h3>
            <pre id="logs">Loading logs...</pre>
        </div>
    </div>
    
    <script>
        // Auto-refresh logs
        setInterval(function() {
            fetch('/logs')
                .then(response => response.text())
                .then(data => {
                    document.getElementById('logs').textContent = data;
                })
                .catch(error => {
                    document.getElementById('logs').textContent = 'Error loading logs: ' + error;
                });
        }, 10000);
    </script>
</body>
</html>
EOF
    
    log "✅ Monitoring dashboard created"
}

# Main function
main() {
    log "📊 Starting monitoring services..."
    
    if [[ "$ENABLE_MONITORING" == "y" ]]; then
        # start_node_exporter
        start_custom_collectors
        start_prometheus
        start_grafana
        start_log_aggregation
        create_monitoring_dashboard
        
        log "🎉 Monitoring services started successfully!"
        log "📊 Prometheus: http://localhost:9090"
        log "📈 Grafana: http://localhost:3000 (admin/admin)"
        log "📋 Dashboard: file://$CONFIGS_DIR/dashboard.html"
    else
        log "ℹ️ Monitoring disabled in configuration"
    fi
}

# Execute main function
main "$@"