#!/bin/bash

# Generate RIC configuration files
# This script generates configuration files for RIC components

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIGS_DIR="$SCRIPT_DIR/../configs"

# Source deployment configuration
source "$CONFIGS_DIR/deployment_config.env"

# Generate FlexRIC configuration
generate_flexric_config() {
    cat > "$CONFIGS_DIR/flexric_config.conf" << EOF
# FlexRIC configuration
# Generated automatically by OpenRAN deployment script

[GENERAL]
# RIC IP and port configuration
ric_ip = $RIC_IP
e2_port = $RIC_E2_PORT

# Service models directory
sm_dir = /usr/local/lib/flexric/

# Logging configuration
log_level = $( [[ "$DEBUG_LOGGING" == "y" ]] && echo "DEBUG" || echo "INFO" )
log_file = /tmp/flexric.log

[E2AP]
# E2AP version
version = E2AP_V2

[KPM]
# KPM service model version
version = KPM_V2_03

[RC]
# RC service model version  
version = RC_V1_03

[SCTP]
# SCTP configuration
rto_initial = 120
rto_min = 120
rto_max = 500
init_max_attempts = 3
max_init_timeo = 500

[XAPPS]
# xApps configuration
enable_kpm = $( [[ "$ENABLE_XAPPS" == "y" ]] && echo "true" || echo "false" )
enable_rc = $( [[ "$ENABLE_XAPPS" == "y" ]] && echo "true" || echo "false" )
enable_gtp = $( [[ "$ENABLE_XAPPS" == "y" ]] && echo "true" || echo "false" )
EOF
}

# Generate ORAN SC RIC docker-compose configuration
generate_oran_sc_docker_compose() {
    cat > "$CONFIGS_DIR/oran_sc_docker_compose.yml" << EOF
# ORAN SC RIC Docker Compose configuration
# Generated automatically by OpenRAN deployment script

version: '3.8'

services:
  db:
    image: mysql:5.7
    container_name: ric-db
    restart: always
    environment:
      MYSQL_ROOT_PASSWORD: password
      MYSQL_DATABASE: ricdb
      MYSQL_USER: ricuser
      MYSQL_PASSWORD: ricpass
    volumes:
      - ric_db_data:/var/lib/mysql
    ports:
      - "3306:3306"
    networks:
      - ric-net

  redis:
    image: redis:alpine
    container_name: ric-redis
    restart: always
    ports:
      - "6379:6379"
    networks:
      - ric-net

  e2mgr:
    image: nexus3.o-ran-sc.org:10002/o-ran-sc/ric-plt-e2mgr:6.0.2
    container_name: ric-e2mgr
    restart: always
    depends_on:
      - db
      - redis
    environment:
      - DB_HOST=db
      - DB_PORT=3306
      - DB_NAME=ricdb
      - DB_USER=ricuser
      - DB_PASSWORD=ricpass
      - REDIS_HOST=redis
      - REDIS_PORT=6379
      - RIC_IP=$RIC_IP
      - E2_PORT=$RIC_E2_PORT
    ports:
      - "3800:3800"
      - "$RIC_E2_PORT:$RIC_E2_PORT"
    networks:
      - ric-net

  e2rtmansim:
    image: nexus3.o-ran-sc.org:10002/o-ran-sc/ric-plt-rtmgr:0.8.3
    container_name: ric-rtmgr
    restart: always
    depends_on:
      - e2mgr
    environment:
      - E2MGR_HOST=e2mgr
      - E2MGR_PORT=3800
      - RIC_IP=$RIC_IP
    ports:
      - "4561:4561"
    networks:
      - ric-net

  a1mediator:
    image: nexus3.o-ran-sc.org:10002/o-ran-sc/ric-plt-a1:2.7.0
    container_name: ric-a1mediator
    restart: always
    depends_on:
      - e2mgr
    environment:
      - A1_HOST=0.0.0.0
      - A1_PORT=10000
      - RIC_IP=$RIC_IP
    ports:
      - "10000:10000"
    networks:
      - ric-net

  xapp-onboarder:
    image: nexus3.o-ran-sc.org:10002/o-ran-sc/ric-plt-xapp-onboarder:1.0.0
    container_name: ric-xapp-onboarder
    restart: always
    depends_on:
      - e2mgr
    environment:
      - CHART_REPO_URL=http://aux-entry/charts
      - HELM_HOST=helm-service
      - HELM_PORT=44134
    ports:
      - "8888:8888"
    networks:
      - ric-net

volumes:
  ric_db_data:

networks:
  ric-net:
    driver: bridge
    ipam:
      config:
        - subnet: 172.20.0.0/16
EOF
}

# Generate xApp configuration
generate_xapp_config() {
    cat > "$CONFIGS_DIR/xapp_config.json" << EOF
{
  "xapp_name": "monitoring-xapp",
  "version": "1.0.0",
  "containers": [
    {
      "name": "monitoring-xapp",
      "image": {
        "registry": "nexus3.o-ran-sc.org:10002",
        "name": "o-ran-sc/ric-app-kpimon",
        "tag": "1.0.0"
      },
      "command": ["/opt/ric/monitoring_xapp"],
      "args": [
        "--config-file", "/opt/ric/config/config-file.json",
        "--ric-ip", "$RIC_IP",
        "--ric-port", "$RIC_E2_PORT"
      ]
    }
  ],
  "messaging": {
    "ports": [
      {
        "name": "http",
        "container": "monitoring-xapp",
        "port": 8080,
        "txMessages": ["RIC_SUB_REQ", "RIC_SUB_DEL_REQ"],
        "rxMessages": ["RIC_SUB_RESP", "RIC_SUB_DEL_RESP", "RIC_INDICATION"],
        "policies": [1],
        "description": "http service"
      }
    ]
  },
  "controls": {
    "logger": {
      "level": "$( [[ "$DEBUG_LOGGING" == "y" ]] && echo "DEBUG" || echo "INFO" )"
    }
  }
}
EOF
}

# Generate monitoring configuration
generate_monitoring_config() {
    cat > "$CONFIGS_DIR/monitoring_config.yml" << EOF
# Monitoring configuration
# Generated automatically by OpenRAN deployment script

# Prometheus configuration
prometheus:
  enabled: $( [[ "$ENABLE_MONITORING" == "y" ]] && echo "true" || echo "false" )
  port: 9090
  scrape_interval: 15s
  targets:
    - ric:$RIC_E2_PORT
    - gnb:$GNB_IP:8080
    - core:10.45.1.1:8080

# Grafana configuration
grafana:
  enabled: $( [[ "$ENABLE_MONITORING" == "y" ]] && echo "true" || echo "false" )
  port: 3000
  admin_user: admin
  admin_password: admin

# Logging configuration
logging:
  level: $( [[ "$DEBUG_LOGGING" == "y" ]] && echo "DEBUG" || echo "INFO" )
  file: /tmp/monitoring.log
  max_size: 100MB
  max_files: 10

# Metrics configuration
metrics:
  ric_metrics:
    - e2_connections
    - active_xapps
    - subscription_count
    - indication_count
  
  gnb_metrics:
    - connected_ues
    - throughput_dl
    - throughput_ul
    - resource_utilization
  
  core_metrics:
    - registered_ues
    - active_sessions
    - network_throughput
EOF
}

# Main function
main() {
    echo "📄 Generating RIC configuration files..."
    
    # Generate configurations based on deployment type
    case $DEPLOYMENT_TYPE in
        "oran-sc"|"both")
            generate_oran_sc_docker_compose
            generate_xapp_config
            ;;
        "flexric-oai"|"both")
            generate_flexric_config
            ;;
    esac
    
    # Generate monitoring configuration
    generate_monitoring_config
    
    echo "✅ RIC configuration files generated successfully"
}

# Execute main function
main "$@"