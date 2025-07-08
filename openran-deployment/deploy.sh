#!/bin/bash

# OpenRAN Stack Deployment Script
# This script deploys a complete OpenRAN stack locally including:
# - RIC (Radio Intelligent Controller)
# - gNodeB (5G Base Station)
# - 5G Core Network
# - Simulated UE (User Equipment)
# - xApps for monitoring and control

set -e

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Directory paths
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIGS_DIR="$SCRIPT_DIR/configs"
LOGS_DIR="$SCRIPT_DIR/logs"
SCRIPTS_DIR="$SCRIPT_DIR/scripts"

# Create necessary directories
mkdir -p "$LOGS_DIR" "$CONFIGS_DIR" "$SCRIPTS_DIR"

# Logging function
log() {
    echo -e "${GREEN}[$(date '+%Y-%m-%d %H:%M:%S')]${NC} $1" | tee -a "$LOGS_DIR/deployment.log"
}

warn() {
    echo -e "${YELLOW}[$(date '+%Y-%m-%d %H:%M:%S')] WARNING:${NC} $1" | tee -a "$LOGS_DIR/deployment.log"
}

error() {
    echo -e "${RED}[$(date '+%Y-%m-%d %H:%M:%S')] ERROR:${NC} $1" | tee -a "$LOGS_DIR/deployment.log"
}

# Function to check if running as root
check_root() {
    if [[ $EUID -eq 0 ]]; then
        error "This script should not be run as root. Please run as a normal user with sudo privileges."
        exit 1
    fi
}

# Function to check system requirements
check_requirements() {
    log "🔍 Checking system requirements..."
    
    # Check OS
    if [[ ! -f /etc/os-release ]]; then
        error "Cannot determine operating system"
        exit 1
    fi
    
    source /etc/os-release
    if [[ "$ID" != "ubuntu" ]]; then
        warn "This script is designed for Ubuntu. Your OS: $ID"
        read -p "Continue anyway? (y/N): " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            exit 1
        fi
    fi
    
    # Check for required commands
    local required_commands=("git" "docker" "docker-compose" "sudo")
    for cmd in "${required_commands[@]}"; do
        if ! command -v "$cmd" &> /dev/null; then
            error "Required command '$cmd' is not installed"
            exit 1
        fi
    done
    
    # Check available disk space (minimum 20GB)
    local available_space=$(df . | tail -1 | awk '{print $4}')
    if [[ $available_space -lt 20971520 ]]; then # 20GB in KB
        warn "Available disk space is less than 20GB. Deployment may fail."
    fi
    
    # Check available memory (minimum 8GB)
    local available_memory=$(free -m | awk 'NR==2{printf "%d", $2}')
    if [[ $available_memory -lt 8192 ]]; then # 8GB in MB
        warn "Available memory is less than 8GB. Performance may be affected."
    fi
    
    log "✅ System requirements check passed"
}

# Function to collect user configuration
collect_user_config() {
    log "📝 Collecting deployment configuration..."
    
    # Deployment type
    echo -e "${BLUE}Select deployment type:${NC}"
    echo "1) ORAN SC RIC (O-RAN Software Community)"
    echo "2) FlexRIC + OAI (OpenAirInterface)"
    echo "3) Both (for comparison)"
    read -p "Enter your choice (1-3): " DEPLOYMENT_TYPE
    
    case $DEPLOYMENT_TYPE in
        1) DEPLOYMENT_TYPE="oran-sc" ;;
        2) DEPLOYMENT_TYPE="flexric-oai" ;;
        3) DEPLOYMENT_TYPE="both" ;;
        *) error "Invalid choice"; exit 1 ;;
    esac
    
    # Network configuration
    echo -e "${BLUE}Network Configuration:${NC}"
    read -p "Enter 5G Core network subnet (default: 10.45.0.0/16): " CORE_SUBNET
    CORE_SUBNET=${CORE_SUBNET:-"10.45.0.0/16"}
    
    read -p "Enter gNB IP address (default: 10.45.1.100): " GNB_IP
    GNB_IP=${GNB_IP:-"10.45.1.100"}
    
    read -p "Enter UE IP range start (default: 10.45.1.200): " UE_IP_START
    UE_IP_START=${UE_IP_START:-"10.45.1.200"}
    
    # RIC configuration
    echo -e "${BLUE}RIC Configuration:${NC}"
    read -p "Enter RIC IP address (default: 127.0.0.1): " RIC_IP
    RIC_IP=${RIC_IP:-"127.0.0.1"}
    
    read -p "Enter RIC E2 port (default: 36421): " RIC_E2_PORT
    RIC_E2_PORT=${RIC_E2_PORT:-"36421"}
    
    # UE configuration
    echo -e "${BLUE}UE Configuration:${NC}"
    read -p "Enter number of UEs to simulate (default: 1): " NUM_UES
    NUM_UES=${NUM_UES:-"1"}
    
    read -p "Enter IMSI base (default: 001010000000001): " IMSI_BASE
    IMSI_BASE=${IMSI_BASE:-"001010000000001"}
    
    # Advanced options
    echo -e "${BLUE}Advanced Options:${NC}"
    read -p "Enable debug logging? (y/N): " DEBUG_LOGGING
    DEBUG_LOGGING=${DEBUG_LOGGING:-"n"}
    
    read -p "Enable xApps monitoring? (Y/n): " ENABLE_XAPPS
    ENABLE_XAPPS=${ENABLE_XAPPS:-"y"}
    
    read -p "Enable performance monitoring? (Y/n): " ENABLE_MONITORING
    ENABLE_MONITORING=${ENABLE_MONITORING:-"y"}
    
    # Save configuration
    cat > "$CONFIGS_DIR/deployment_config.env" << EOF
# OpenRAN Deployment Configuration
DEPLOYMENT_TYPE=$DEPLOYMENT_TYPE
CORE_SUBNET=$CORE_SUBNET
GNB_IP=$GNB_IP
UE_IP_START=$UE_IP_START
RIC_IP=$RIC_IP
RIC_E2_PORT=$RIC_E2_PORT
NUM_UES=$NUM_UES
IMSI_BASE=$IMSI_BASE
DEBUG_LOGGING=$DEBUG_LOGGING
ENABLE_XAPPS=$ENABLE_XAPPS
ENABLE_MONITORING=$ENABLE_MONITORING
DEPLOYMENT_DATE=$(date '+%Y-%m-%d %H:%M:%S')
EOF
    
    log "✅ Configuration saved to $CONFIGS_DIR/deployment_config.env"
}

# Function to install system dependencies
install_dependencies() {
    log "📦 Installing system dependencies..."
    
    # Update package list
    sudo apt-get update
    
    # Install basic dependencies
    sudo apt-get install -y \
        curl \
        wget \
        git \
        build-essential \
        cmake \
        pkg-config \
        python3 \
        python3-pip \
        python3-dev \
        net-tools \
        htop \
        screen \
        tmux \
        unzip \
        jq \
        vim \
        nano
    
    # Install Docker if not present
    if ! command -v docker &> /dev/null; then
        log "🐳 Installing Docker..."
        curl -fsSL https://get.docker.com -o get-docker.sh
        sudo sh get-docker.sh
        sudo usermod -aG docker $USER
        rm get-docker.sh
        warn "Please log out and log back in for Docker group membership to take effect"
    fi
    
    # Install Docker Compose if not present
    if ! command -v docker-compose &> /dev/null; then
        log "🐳 Installing Docker Compose..."
        sudo curl -L "https://github.com/docker/compose/releases/download/v2.20.2/docker-compose-$(uname -s)-$(uname -m)" -o /usr/local/bin/docker-compose
        sudo chmod +x /usr/local/bin/docker-compose
    fi
    
    # Install specific dependencies based on deployment type
    case $DEPLOYMENT_TYPE in
        "oran-sc"|"both")
            sudo apt-get install -y \
                libmbedtls-dev \
                libsctp-dev \
                libfftw3-dev \
                libzmq3-dev \
                libczmq-dev \
                libconfig++-dev \
                libboost-all-dev \
                doxygen \
                g++-11 \
                gcc-11
            ;;
        "flexric-oai"|"both")
            sudo apt-get install -y \
                cmake-curses-gui \
                libsctp-dev \
                libpcre2-dev \
                gcc-13 \
                g++-13 \
                cpp-13 \
                python3.10-dev \
                ninja-build \
                autoconf \
                automake \
                libtool \
                bison \
                flex
            ;;
    esac
    
    log "✅ Dependencies installed successfully"
}

# Function to setup network namespaces
setup_network() {
    log "🌐 Setting up network namespaces..."
    
    # Create UE namespaces
    for i in $(seq 1 $NUM_UES); do
        sudo ip netns add ue$i 2>/dev/null || true
        log "Created namespace ue$i"
    done
    
    # Setup routing if needed
    # This will be expanded based on the specific RAN implementation
    
    log "✅ Network setup completed"
}

# Function to generate configuration files
generate_configs() {
    log "📄 Generating configuration files..."
    
    # Source the deployment config
    source "$CONFIGS_DIR/deployment_config.env"
    
    # Generate gNB configuration
    if [[ "$DEPLOYMENT_TYPE" == "oran-sc" || "$DEPLOYMENT_TYPE" == "both" ]]; then
        "$SCRIPTS_DIR/generate_gnb_config.sh"
    fi
    
    # Generate UE configuration
    "$SCRIPTS_DIR/generate_ue_config.sh"
    
    # Generate RIC configuration
    "$SCRIPTS_DIR/generate_ric_config.sh"
    
    log "✅ Configuration files generated"
}

# Function to deploy based on selected type
deploy_stack() {
    log "🚀 Deploying OpenRAN stack..."
    
    case $DEPLOYMENT_TYPE in
        "oran-sc")
            "$SCRIPTS_DIR/deploy_oran_sc.sh"
            ;;
        "flexric-oai")
            "$SCRIPTS_DIR/deploy_flexric_oai.sh"
            ;;
        "both")
            "$SCRIPTS_DIR/deploy_oran_sc.sh"
            "$SCRIPTS_DIR/deploy_flexric_oai.sh"
            ;;
    esac
    
    log "✅ OpenRAN stack deployed successfully"
}

# Function to start monitoring
start_monitoring() {
    if [[ "$ENABLE_MONITORING" == "y" ]]; then
        log "📊 Starting monitoring services..."
        "$SCRIPTS_DIR/start_monitoring.sh"
    fi
}

# Function to run post-deployment tests
run_tests() {
    log "🧪 Running post-deployment tests..."
    "$SCRIPTS_DIR/run_tests.sh"
}

# Function to display deployment summary
show_summary() {
    log "📋 Deployment Summary"
    echo -e "${GREEN}=================================${NC}"
    echo -e "${BLUE}Deployment Type:${NC} $DEPLOYMENT_TYPE"
    echo -e "${BLUE}RIC IP:${NC} $RIC_IP:$RIC_E2_PORT"
    echo -e "${BLUE}gNB IP:${NC} $GNB_IP"
    echo -e "${BLUE}Number of UEs:${NC} $NUM_UES"
    echo -e "${BLUE}Core Network:${NC} $CORE_SUBNET"
    echo -e "${GREEN}=================================${NC}"
    
    echo -e "${YELLOW}Useful Commands:${NC}"
    echo "  - View logs: tail -f $LOGS_DIR/deployment.log"
    echo "  - Check containers: docker ps"
    echo "  - Stop deployment: $SCRIPT_DIR/stop.sh"
    echo "  - Restart deployment: $SCRIPT_DIR/restart.sh"
    echo "  - Clean deployment: $SCRIPT_DIR/clean.sh"
    
    if [[ "$ENABLE_MONITORING" == "y" ]]; then
        echo -e "${YELLOW}Monitoring:${NC}"
        echo "  - Open monitoring dashboard: http://localhost:3000"
        echo "  - Check metrics: docker exec -it monitoring-container /bin/bash"
    fi
}

# Function to create management scripts
create_management_scripts() {
    log "🛠️ Creating management scripts..."
    
    # Create stop script
    cat > "$SCRIPT_DIR/stop.sh" << 'EOF'
#!/bin/bash
echo "🛑 Stopping OpenRAN deployment..."
source "$(dirname "$0")/configs/deployment_config.env"
docker-compose -f "$(dirname "$0")/configs/docker-compose.yml" down 2>/dev/null || true
pkill -f "nr-softmodem\|nr-uesoftmodem\|nearRT-RIC\|gnb\|srsue" 2>/dev/null || true
echo "✅ Deployment stopped"
EOF
    
    # Create restart script
    cat > "$SCRIPT_DIR/restart.sh" << 'EOF'
#!/bin/bash
echo "🔄 Restarting OpenRAN deployment..."
"$(dirname "$0")/stop.sh"
sleep 5
"$(dirname "$0")/deploy.sh" --restart
EOF
    
    # Create clean script
    cat > "$SCRIPT_DIR/clean.sh" << 'EOF'
#!/bin/bash
echo "🧹 Cleaning OpenRAN deployment..."
"$(dirname "$0")/stop.sh"
docker system prune -f
for i in {1..10}; do
    sudo ip netns del ue$i 2>/dev/null || true
done
rm -rf "$(dirname "$0")/logs/*"
echo "✅ Deployment cleaned"
EOF
    
    # Make scripts executable
    chmod +x "$SCRIPT_DIR"/*.sh
    
    log "✅ Management scripts created"
}

# Main execution function
main() {
    echo -e "${GREEN}🚀 OpenRAN Stack Deployment Tool${NC}"
    echo -e "${GREEN}=================================${NC}"
    
    # Check if restart flag is provided
    if [[ "$1" == "--restart" ]]; then
        log "🔄 Restarting deployment with existing configuration..."
        source "$CONFIGS_DIR/deployment_config.env"
    else
        check_root
        check_requirements
        collect_user_config
        install_dependencies
        setup_network
        create_management_scripts
    fi
    
    generate_configs
    deploy_stack
    start_monitoring
    run_tests
    show_summary
    
    log "🎉 OpenRAN deployment completed successfully!"
    log "📖 Check the documentation in $SCRIPT_DIR/docs/ for more information"
}

# Trap to handle script interruption
trap 'error "Deployment interrupted"; exit 1' INT TERM

# Execute main function
main "$@"