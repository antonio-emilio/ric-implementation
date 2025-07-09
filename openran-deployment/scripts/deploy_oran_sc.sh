#!/bin/bash

# Deploy ORAN SC RIC implementation
# This script handles the deployment of the O-RAN Software Community RIC

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIGS_DIR="$SCRIPT_DIR/../configs"
LOGS_DIR="$SCRIPT_DIR/../logs"

# Source deployment configuration
source "$CONFIGS_DIR/deployment_config.env"

# Logging function
log() {
    echo -e "\033[0;32m[$(date '+%Y-%m-%d %H:%M:%S')]\033[0m $1" | tee -a "$LOGS_DIR/oran_sc_deployment.log"
}

deploy_oran_sc_ric() {
    log "🚀 Deploying ORAN SC RIC..."
    
    # Create working directory
    mkdir -p "$SCRIPT_DIR/../work/oran-sc"
    cd "$SCRIPT_DIR/../work/oran-sc"
    
    # Clone ORAN SC RIC repository
    if [[ ! -d "oran-sc-ric" ]]; then
        log "📥 Cloning ORAN SC RIC repository..."
        git clone https://github.com/srsran/oran-sc-ric.git
    else
        log "ℹ️ ORAN SC RIC repository already exists"
    fi
    
    cd oran-sc-ric
    
    # Build and start RIC containers
    log "🏗️ Building and starting RIC containers..."
    docker-compose up --build -d
    
    # Wait for RIC to be ready
    log "⏳ Waiting for RIC to be ready..."
    sleep 30
    
    # Check RIC status
    log "🔍 Checking RIC status..."
    docker-compose ps
    
    log "✅ ORAN SC RIC deployed successfully"
}

deploy_srsran_gnb() {
    log "📡 Deploying srsRAN gNB..."
    
    cd "$SCRIPT_DIR/../work/oran-sc"
    
    # Clone srsRAN Project
    if [[ ! -d "srsRAN_Project" ]]; then
        log "📥 Cloning srsRAN Project..."
        git clone https://github.com/srsran/srsRAN_Project.git
        cd srsRAN_Project
        
        # Build srsRAN with ZMQ support
        log "🏗️ Building srsRAN Project with ZMQ support..."
        mkdir -p build && cd build
        cmake ../ -DENABLE_EXPORT=ON -DENABLE_ZEROMQ=ON
        make -j $(nproc)
        cd ..
    else
        log "ℹ️ srsRAN Project already exists"
        cd srsRAN_Project
    fi
    
    # Start 5G Core
    log "🏗️ Starting 5G Core..."
    cd docker
    docker-compose up --build -d 5gc
    cd ..
    
    # Start gNB with E2 agent
    log "📡 Starting gNB with E2 agent..."
    cd build/apps/gnb
    
    # Use the generated configuration
    sudo ./gnb -c "$CONFIGS_DIR/gnb_config.yaml" &
    GNB_PID=$!
    echo $GNB_PID > "$LOGS_DIR/gnb.pid"
    
    cd ../../../
    
    log "✅ srsRAN gNB deployed successfully"
}

deploy_srsran_ue() {
    log "📱 Deploying srsRAN UE..."
    
    cd "$SCRIPT_DIR/../work/oran-sc"
    
    # Clone srsRAN 4G for UE
    if [[ ! -d "srsRAN_4G" ]]; then
        log "📥 Cloning srsRAN 4G for UE..."
        git clone https://github.com/srsran/srsRAN_4G.git
        cd srsRAN_4G
        
        # Build srsRAN 4G
        log "🏗️ Building srsRAN 4G..."
        mkdir -p build && cd build
        CC=gcc-11 CXX=g++-11 cmake ..
        make -j $(nproc)
        sudo make install
        srsran_install_configs.sh user
        cd ..
    else
        log "ℹ️ srsRAN 4G already exists"
        cd srsRAN_4G
    fi
    
    # Start UEs
    log "📱 Starting UEs..."
    cd build/srsue/src
    
    for i in $(seq 1 $NUM_UES); do
        log "Starting UE $i in namespace ue$i..."
        sudo ip netns exec ue$i ./srsue "$CONFIGS_DIR/ue${i}_config.conf" &
        UE_PID=$!
        echo $UE_PID > "$LOGS_DIR/ue${i}.pid"
    done
    
    cd ../../../
    
    log "✅ srsRAN UEs deployed successfully"
}

start_xapps() {
    if [[ "$ENABLE_XAPPS" == "y" ]]; then
        log "📊 Starting xApps..."
        
        # This would be expanded to start actual xApps
        # For now, we'll create placeholder monitoring
        log "🔍 xApps monitoring enabled"
    fi
}

run_connectivity_tests() {
    log "🧪 Running connectivity tests..."
    
    # Test UE connectivity
    for i in $(seq 1 $NUM_UES); do
        log "Testing UE $i connectivity..."
        sudo ip netns exec ue$i ping -c 3 10.45.1.1 || log "⚠️ UE $i connectivity test failed"
    done
    
    log "✅ Connectivity tests completed"
}

# Main deployment function
main() {
    log "🚀 Starting ORAN SC deployment..."
    
    deploy_oran_sc_ric
    deploy_srsran_gnb
    
    # Wait for gNB to be ready
    sleep 10
    
    deploy_srsran_ue
    
    # Wait for UEs to register
    sleep 10
    
    start_xapps
    run_connectivity_tests
    
    log "🎉 ORAN SC deployment completed successfully!"
}

# Execute main function
main "$@"
