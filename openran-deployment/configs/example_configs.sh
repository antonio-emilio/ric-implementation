#!/bin/bash

# Example configuration for different deployment scenarios
# Source this file to set up specific configurations

# Development Environment
setup_development() {
    export DEPLOYMENT_TYPE="flexric-oai"
    export CORE_SUBNET="10.45.0.0/16"
    export GNB_IP="10.45.1.100"
    export UE_IP_START="10.45.1.200"
    export RIC_IP="127.0.0.1"
    export RIC_E2_PORT="36421"
    export NUM_UES="2"
    export IMSI_BASE="001010000000001"
    export DEBUG_LOGGING="y"
    export ENABLE_XAPPS="y"
    export ENABLE_MONITORING="y"
    
    echo "✅ Development environment configuration loaded"
}

# Production Environment  
setup_production() {
    export DEPLOYMENT_TYPE="oran-sc"
    export CORE_SUBNET="10.45.0.0/16"
    export GNB_IP="10.45.1.100"
    export UE_IP_START="10.45.1.200"
    export RIC_IP="127.0.0.1"
    export RIC_E2_PORT="36421"
    export NUM_UES="10"
    export IMSI_BASE="001010000000001"
    export DEBUG_LOGGING="n"
    export ENABLE_XAPPS="y"
    export ENABLE_MONITORING="y"
    
    echo "✅ Production environment configuration loaded"
}

# Testing Environment
setup_testing() {
    export DEPLOYMENT_TYPE="both"
    export CORE_SUBNET="10.45.0.0/16"
    export GNB_IP="10.45.1.100"
    export UE_IP_START="10.45.1.200"
    export RIC_IP="127.0.0.1"
    export RIC_E2_PORT="36421"
    export NUM_UES="5"
    export IMSI_BASE="001010000000001"
    export DEBUG_LOGGING="y"
    export ENABLE_XAPPS="y"
    export ENABLE_MONITORING="y"
    
    echo "✅ Testing environment configuration loaded"
}

# Demo Environment
setup_demo() {
    export DEPLOYMENT_TYPE="oran-sc"
    export CORE_SUBNET="10.45.0.0/16"
    export GNB_IP="10.45.1.100"
    export UE_IP_START="10.45.1.200"
    export RIC_IP="127.0.0.1"
    export RIC_E2_PORT="36421"
    export NUM_UES="3"
    export IMSI_BASE="001010000000001"
    export DEBUG_LOGGING="n"
    export ENABLE_XAPPS="y"
    export ENABLE_MONITORING="y"
    
    echo "✅ Demo environment configuration loaded"
}

# Usage information
usage() {
    echo "OpenRAN Deployment Configuration"
    echo "Usage: source $0 [environment]"
    echo ""
    echo "Available environments:"
    echo "  development  - Development setup with FlexRIC/OAI"
    echo "  production   - Production setup with O-RAN SC"
    echo "  testing      - Testing setup with both implementations"
    echo "  demo         - Demo setup for presentations"
    echo ""
    echo "Example:"
    echo "  source $0 development"
    echo "  ./deploy.sh --skip-config"
}

# Main logic
case "$1" in
    "development")
        setup_development
        ;;
    "production")
        setup_production
        ;;
    "testing")
        setup_testing
        ;;
    "demo")
        setup_demo
        ;;
    *)
        usage
        ;;
esac