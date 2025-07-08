#!/bin/bash

# Deploy FlexRIC + OAI implementation
# This script handles the deployment of FlexRIC with OpenAirInterface

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIGS_DIR="$SCRIPT_DIR/../configs"
LOGS_DIR="$SCRIPT_DIR/../logs"

# Source deployment configuration
source "$CONFIGS_DIR/deployment_config.env"

# Logging function
log() {
    echo -e "\033[0;32m[$(date '+%Y-%m-%d %H:%M:%S')]\033[0m $1" | tee -a "$LOGS_DIR/flexric_oai_deployment.log"
}

install_flexric_dependencies() {
    log "📦 Installing FlexRIC dependencies..."
    
    # Set GCC 13 as default
    sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100 \
        --slave /usr/bin/g++ g++ /usr/bin/g++-13 --slave /usr/bin/gcov gcov /usr/bin/gcov-13
    sudo update-alternatives --set gcc /usr/bin/gcc-13
    
    # Install ASN1C with UPER support
    cd /tmp
    if [[ ! -d "asn1c" ]]; then
        log "🔧 Installing ASN1C with UPER support..."
        git clone https://github.com/mouse07410/asn1c.git
        cd asn1c
        git checkout aper
        autoreconf -fiv
        ./configure
        make -j$(nproc)
        sudo make install
        cd ..
    fi
    
    log "✅ FlexRIC dependencies installed"
}

deploy_flexric() {
    log "🚀 Deploying FlexRIC..."
    
    # Create working directory
    mkdir -p "$SCRIPT_DIR/../work/flexric-oai"
    cd "$SCRIPT_DIR/../work/flexric-oai"
    
    # Clone FlexRIC repository
    if [[ ! -d "flexric" ]]; then
        log "📥 Cloning FlexRIC repository..."
        git clone https://gitlab.eurecom.fr/mosaic5g/flexric.git
        cd flexric
        
        # Build FlexRIC
        log "🏗️ Building FlexRIC..."
        mkdir -p build && cd build
        cmake -DCMAKE_BUILD_TYPE=Release -DASN1C_EXEC_PATH=/usr/local/bin/asn1c ..
        make -j$(nproc)
        sudo make install
        
        # Create expected directory
        sudo mkdir -p /usr/local/lib/flexric/
        
        cd ..
    else
        log "ℹ️ FlexRIC already exists"
        cd flexric
    fi
    
    # Start nearRT-RIC
    log "🚀 Starting nearRT-RIC..."
    cd build
    ./examples/ric/nearRT-RIC &
    RIC_PID=$!
    echo $RIC_PID > "$LOGS_DIR/nearrt_ric.pid"
    
    # Wait for RIC to start
    sleep 5
    
    cd ../..
    
    log "✅ FlexRIC deployed successfully"
}

deploy_oai_ran() {
    log "📡 Deploying OAI RAN..."
    
    cd "$SCRIPT_DIR/../work/flexric-oai"
    
    # Clone OAI repository
    if [[ ! -d "openairinterface5g" ]]; then
        log "📥 Cloning OAI repository..."
        git clone https://gitlab.eurecom.fr/oai/openairinterface5g.git
        cd openairinterface5g
        
        # Initialize FlexRIC submodule
        log "📁 Initializing FlexRIC submodule..."
        cd openair2/E2AP/flexric
        git submodule init && git submodule update
        cd ../../../
        
        # Build OAI with E2 support
        log "🏗️ Building OAI with E2 support..."
        cd cmake_targets
        ./build_oai -I
        ./build_oai --gNB --nrUE --build-e2 --cmake-opt -DE2AP_VERSION=E2AP_V2 --cmake-opt -DKMP_VERSION=KMP_V2_03 --ninja
        cd ..
        
        # Build embedded FlexRIC
        log "🏗️ Building embedded FlexRIC..."
        cd openair2/E2AP/flexric
        mkdir -p build && cd build
        cmake -GNinja -DCMAKE_BUILD_TYPE=Release -DE2AP_VERSION=E2AP_V2 -DKMP_VERSION=KMP_V2_03 ..
        ninja
        sudo make install
        cd ../../../../
        
    else
        log "ℹ️ OAI repository already exists"
        cd openairinterface5g
    fi
    
    # Start gNB
    log "📡 Starting gNB with E2 agent..."
    cd build
    sudo ./nr-softmodem -O "$CONFIGS_DIR/gnb_oai_config.conf" --rfsim &
    GNB_PID=$!
    echo $GNB_PID > "$LOGS_DIR/oai_gnb.pid"
    
    # Wait for gNB to start
    sleep 10
    
    # Start UE
    log "📱 Starting UE..."
    sudo ./nr-uesoftmodem -r 106 --numerology 1 --band 78 -C 3619200000 --rfsim --uicc0.imsi $IMSI_BASE --rfsimulator.serveraddr 127.0.0.1 &
    UE_PID=$!
    echo $UE_PID > "$LOGS_DIR/oai_ue.pid"
    
    cd ..
    
    log "✅ OAI RAN deployed successfully"
}

start_xapps() {
    if [[ "$ENABLE_XAPPS" == "y" ]]; then
        log "📊 Starting xApps..."
        
        cd "$SCRIPT_DIR/../work/flexric-oai/openairinterface5g/openair2/E2AP/flexric"
        
        # Start monitoring xApps
        log "📈 Starting KPM monitoring xApp..."
        XAPP_DURATION=60 ./build/examples/xApp/c/monitor/xapp_kpm_moni &
        XAPP_KPM_PID=$!
        echo $XAPP_KPM_PID > "$LOGS_DIR/xapp_kpm.pid"
        
        sleep 2
        
        log "📡 Starting RC monitoring xApp..."
        XAPP_DURATION=60 ./build/examples/xApp/c/monitor/xapp_rc_moni &
        XAPP_RC_PID=$!
        echo $XAPP_RC_PID > "$LOGS_DIR/xapp_rc.pid"
        
        sleep 2
        
        log "📊 Starting GTP+MAC+RLC+PDCP monitoring xApp..."
        XAPP_DURATION=60 ./build/examples/xApp/c/monitor/xapp_gtp_mac_rlc_pdcp_moni &
        XAPP_GTP_PID=$!
        echo $XAPP_GTP_PID > "$LOGS_DIR/xapp_gtp.pid"
        
        log "✅ xApps started successfully"
    fi
}

run_connectivity_tests() {
    log "🧪 Running connectivity tests..."
    
    # Test basic connectivity
    log "Testing basic connectivity..."
    ping -c 3 127.0.0.1 || log "⚠️ Localhost connectivity test failed"
    
    log "✅ Connectivity tests completed"
}

# Main deployment function
main() {
    log "🚀 Starting FlexRIC + OAI deployment..."
    
    install_flexric_dependencies
    deploy_flexric
    
    # Wait for RIC to be ready
    sleep 10
    
    deploy_oai_ran
    
    # Wait for RAN to be ready
    sleep 15
    
    start_xapps
    run_connectivity_tests
    
    log "🎉 FlexRIC + OAI deployment completed successfully!"
}

# Execute main function
main "$@"