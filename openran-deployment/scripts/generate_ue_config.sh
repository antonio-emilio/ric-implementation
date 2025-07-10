#!/bin/bash

# Generate UE configuration files
# This script generates configuration files for different UE implementations

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIGS_DIR="$SCRIPT_DIR/../configs"

# Source deployment configuration
source "$CONFIGS_DIR/deployment_config.env"

# Generate srsRAN UE configuration
generate_srsran_ue_config() {
    for i in $(seq 1 $NUM_UES); do
        # Calculate IMSI for this UE
        local imsi=$((IMSI_BASE + i - 1))
        
        cat > "$CONFIGS_DIR/ue${i}_config.conf" << EOF
# srsRAN UE configuration for UE $i
# Generated automatically by OpenRAN deployment script

[rf]
# RF configuration for ZMQ
device_name = zmq
device_args = tx_port=tcp://*:$((2000 + i)),rx_port=tcp://localhost:2000,id=ue,base_srate=23.04e6

[rat.eutra]
# LTE parameters
dl_earfcn = 3350
nof_carriers = 1

[rat.nr]
# 5G NR parameters
bands = 78
nof_carriers = 1

# [pcap]
# Packet capture configuration
# enable = false
# mac_filename = /tmp/ue${i}.pcap
# nas_enable = false
# nas_filename = /tmp/ue${i}_nas.pcap

[log]
# Logging configuration
all_level = $( [[ "$DEBUG_LOGGING" == "y" ]] && echo "debug" || echo "info" )
filename = /tmp/ue${i}.log
file_max_size = 1000000

[usim]
# USIM configuration
mode = soft
algo = xor
opc = 63BFA50EE6523365FF14C1F45F88737D
k = 00112233445566778899aabbccddeeff
imsi = $imsi
imei = 35361400000000$i
pin = 1234

[rrc]
# RRC configuration
ue_category = 4
release = 15
mbms_service_id = -1
mbms_service_port = 4321

[nas]
# NAS configuration
apn = srsapn
apn_protocol = ipv4
user = srsuser
pass = srspass
force_imsi_attach = false

[gw]
# Gateway configuration
netns = ue$i
ip_devname = tun_srsue$i
ip_netmask = 255.255.255.0

[gui]
# GUI configuration
enable = false
EOF
    done
}

# Generate OAI UE configuration
generate_oai_ue_config() {
    for i in $(seq 1 $NUM_UES); do
        # Calculate IMSI for this UE
        local imsi=$((IMSI_BASE + i - 1))
        
        cat > "$CONFIGS_DIR/oai_ue${i}_config.conf" << EOF
# OAI UE configuration for UE $i
# Generated automatically by OpenRAN deployment script

# UE identity
uicc0.imsi = $imsi
uicc0.key = fec86ba6eb707ed08905757b1bb44b8f
uicc0.opc = C42449363BBAD02B66D16BC975D77CC1
uicc0.dnn = oai
uicc0.nssai_sst = 1
uicc0.nssai_sd = 0x010203

# RF simulator configuration
rfsimulator.serveraddr = 127.0.0.1
rfsimulator.serverport = 4043

# Radio parameters
radio.dl_earfcn = 632628
radio.band = 78
radio.max_rxgain = 114
radio.numerology = 1
radio.prach_config_index = 159

# Network configuration
network.interface = oai${i}
network.ip = ${UE_IP_START%.*}.$((${UE_IP_START##*.} + i - 1))
network.netmask = 255.255.255.0

# Logging configuration
log.enable = 1
log.level = $( [[ "$DEBUG_LOGGING" == "y" ]] && echo "DEBUG" || echo "INFO" )
log.filename = /tmp/oai_ue${i}.log
EOF
    done
}

# Generate UE test script
generate_ue_test_script() {
    cat > "$CONFIGS_DIR/test_ue_connectivity.sh" << EOF
#!/bin/bash

# Test UE connectivity
# Generated automatically by OpenRAN deployment script

# Source configuration
source "$CONFIGS_DIR/deployment_config.env"

echo "🧪 Testing UE connectivity..."

for i in \$(seq 1 \$NUM_UES); do
    echo "Testing UE \$i..."
    
    # Test ping to core network
    if sudo ip netns exec ue\$i ping -c 3 -W 2 10.45.1.1 &>/dev/null; then
        echo "✅ UE \$i: Core network connectivity OK"
    else
        echo "❌ UE \$i: Core network connectivity FAILED"
    fi
    
    # Test DNS resolution
    if sudo ip netns exec ue\$i nslookup google.com &>/dev/null; then
        echo "✅ UE \$i: DNS resolution OK"
    else
        echo "❌ UE \$i: DNS resolution FAILED"
    fi
    
    # Test internet connectivity
    if sudo ip netns exec ue\$i ping -c 3 -W 2 8.8.8.8 &>/dev/null; then
        echo "✅ UE \$i: Internet connectivity OK"
    else
        echo "❌ UE \$i: Internet connectivity FAILED"
    fi
    
    echo "---"
done

echo "🏁 UE connectivity tests completed"
EOF
    
    chmod +x "$CONFIGS_DIR/test_ue_connectivity.sh"
}

# Generate UE monitoring script
generate_ue_monitoring_script() {
    cat > "$CONFIGS_DIR/monitor_ues.sh" << EOF
#!/bin/bash

# Monitor UE status
# Generated automatically by OpenRAN deployment script

# Source configuration
source "$CONFIGS_DIR/deployment_config.env"

echo "📊 Monitoring UE status..."

while true; do
    clear
    echo "=== UE Status Monitor ==="
    date
    echo "=========================="
    
    for i in \$(seq 1 \$NUM_UES); do
        echo "UE \$i (namespace: ue\$i):"
        
        # Check if UE process is running
        if pgrep -f "ue\$i" &>/dev/null; then
            echo "  Status: 🟢 Running"
        else
            echo "  Status: 🔴 Stopped"
        fi
        
        # Check network interface
        if sudo ip netns exec ue\$i ip addr show tun_srsue\$i &>/dev/null; then
            local ip=\$(sudo ip netns exec ue\$i ip addr show tun_srsue\$i | grep 'inet ' | awk '{print \$2}')
            echo "  Interface: 🟢 tun_srsue\$i (\$ip)"
        else
            echo "  Interface: 🔴 Not configured"
        fi
        
        # Check connectivity
        if sudo ip netns exec ue\$i ping -c 1 -W 1 10.45.1.1 &>/dev/null; then
            echo "  Connectivity: 🟢 OK"
        else
            echo "  Connectivity: 🔴 FAILED"
        fi
        
        echo "  ---"
    done
    
    echo "Press Ctrl+C to exit"
    sleep 5
done
EOF
    
    chmod +x "$CONFIGS_DIR/monitor_ues.sh"
}

# Main function
main() {
    echo "📄 Generating UE configuration files..."
    
    # Generate configurations based on deployment type
    case $DEPLOYMENT_TYPE in
        "oran-sc")
            generate_srsran_ue_config
            ;;
        "flexric-oai")
            generate_oai_ue_config
            ;;
        "both")
            generate_srsran_ue_config
            generate_oai_ue_config
            ;;
    esac
    
    # Generate utility scripts
    generate_ue_test_script
    generate_ue_monitoring_script
    
    echo "✅ UE configuration files generated successfully"
}

# Execute main function
main "$@"
main "$@"
