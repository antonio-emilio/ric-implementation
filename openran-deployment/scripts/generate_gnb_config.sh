#!/bin/bash

# Generate gNB configuration files
# This script generates configuration files for different gNB implementations

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIGS_DIR="$SCRIPT_DIR/../configs"

# Source deployment configuration
source "$CONFIGS_DIR/deployment_config.env"

# Set AMF_IP and AMF_PORT for srsRAN gNB
AMF_IP="${AMF_IP:-127.0.0.1}"
# srsRAN expects AMF_PORT in the range [20000-40000], so use a valid default (e.g., 38412)
AMF_PORT="${AMF_PORT:-38412}"

# Optionally force GNB_IP to 127.0.0.1 for local deployments
GNB_IP="127.0.0.1"

# Generate srsRAN gNB configuration for ORAN SC
generate_srsran_gnb_config() {
    cat > "$CONFIGS_DIR/gnb_config.yaml" << EOF
# srsRAN gNB configuration for ORAN SC RIC (modern YAML schema)

cu_cp:
  amf:
    addr: $AMF_IP
    port: $AMF_PORT
    bind_addr: $GNB_IP
    supported_tracking_areas:
      - tac: 7
        plmn_list:
          - plmn: "00101"
            tai_slice_support_list:
              - sst: 1

ru_sdr:
  device_driver: zmq
  device_args: fail_on_disconnect=true,id=gnb,base_srate=23.04e6,tx_port0=tcp://*:2000,rx_port0=tcp://localhost:2001,tx_port1=tcp://*:2002,rx_port1=tcp://localhost:2003
  srate: 23.04
  tx_gain: 80
  rx_gain: 40

cell_cfg:
  dl_arfcn: 632628
  band: 78
  channel_bandwidth_MHz: 20
  common_scs: 30
  plmn: "00101"
  tac: 7
  pci: 1
  prach:
    prach_config_index: 159
  pdsch:
    mcs_table: qam64
  pusch:
    mcs_table: qam64

log:
  filename: /tmp/gnb.log
  all_level: $( [[ "$DEBUG_LOGGING" == "y" ]] && echo "debug" || echo "info" )

pcap:
  mac_enable: enable
  mac_filename: /tmp/gnb_mac.pcap
  ngap_enable: enable
  ngap_filename: /tmp/gnb_ngap.pcap
EOF
}

# Generate OAI gNB configuration
generate_oai_gnb_config() {
    cat > "$CONFIGS_DIR/gnb_oai_config.conf" << EOF
# OAI gNB configuration for FlexRIC
# Generated automatically by OpenRAN deployment script

Active_gNBs = ( "gNB-OAI");

gNBs = ({
    gNB_ID = 0xe00;
    gNB_name = "gNB-OAI";
    
    # Tracking area code
    tracking_area_code = 1;
    plmn_list = ({ mcc = 001; mnc = 01; mnc_length = 2; snssaiList = ({ sst = 1, sd = 0x010203 }) });
    
    # Radio parameters
    nr_cellid = 12345678L;
    
    # Physical cell identity
    physCellId = 0;
    
    # Downlink frequency and bandwidth
    dl_carrierfreq = 3619200000;
    dl_bandwidth = 106;
    
    # Uplink frequency and bandwidth  
    ul_carrierfreq = 3619200000;
    ul_bandwidth = 106;
    
    # Numerology
    ssb_SubcarrierOffset = 0;
    pdsch_AntennaPorts = 1;
    pusch_AntennaPorts = 1;
    
    # AMF configuration
    amf_ip_address = ({ ipv4 = "127.0.0.1"; ipv6 = "192:168:30::17"; port = 38412; });
    
    # gNB interface configuration
    NETWORK_INTERFACES = {
        GNB_INTERFACE_NAME_FOR_NG_AMF = "lo";
        GNB_IPV4_ADDRESS_FOR_NG_AMF = "$GNB_IP";
        GNB_INTERFACE_NAME_FOR_NGU = "lo";
        GNB_IPV4_ADDRESS_FOR_NGU = "$GNB_IP";
        GNB_PORT_FOR_S1U = 2152;
    };
});

# E2 Agent configuration
e2_agent = {
    near_ric_ip_addr = "$RIC_IP";
    sm_dir = "/usr/local/lib/flexric/";
};

# RU configuration for RF simulator
RUs = ({
    local_rf = "yes";
    nb_tx = 1;
    nb_rx = 1;
    att_tx = 0;
    att_rx = 0;
    bands = [78];
    max_pdschReferenceSignalPower = -27;
    max_rxgain = 114;
    eNB_instances = [0];
    rfsimulator = {
        serveraddr = "server";
        serverport = "4043";
        options = (); 
    };
});

# Security configuration
security = {
    # Ciphering algorithms
    ciphering_algorithms = ["nea0", "nea1", "nea2"];
    
    # Integrity algorithms  
    integrity_algorithms = ["nia0", "nia1", "nia2"];
    
    # DRB configuration
    drb_ciphering = "nea0";
    drb_integrity = "nia0";
};

# Log configuration
log_config = {
    global_log_level = "$( [[ "$DEBUG_LOGGING" == "y" ]] && echo "debug" || echo "info" )";
    hw_log_level = "info";
    phy_log_level = "info";
    mac_log_level = "info";
    rlc_log_level = "info";
    pdcp_log_level = "info";
    rrc_log_level = "info";
    ngap_log_level = "info";
    gtpu_log_level = "info";
};
EOF
}

# Generate RIC configuration
generate_ric_config() {
    cat > "$CONFIGS_DIR/ric_config.conf" << EOF
# RIC configuration
# Generated automatically by OpenRAN deployment script

[GENERAL]
# RIC IP and port configuration
ric_ip = $RIC_IP
e2_port = $RIC_E2_PORT

# Service models directory
sm_dir = /usr/local/lib/flexric/

# Logging configuration
log_level = $( [[ "$DEBUG_LOGGING" == "y" ]] && echo "DEBUG" || echo "INFO" )
log_file = /tmp/ric.log

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
EOF
}

# Main function
main() {
    echo "📄 Generating gNB configuration files..."
    
    # Generate configurations based on deployment type
    case $DEPLOYMENT_TYPE in
        "oran-sc")
            generate_srsran_gnb_config
            ;;
        "flexric-oai")
            generate_oai_gnb_config
            ;;
        "both")
            generate_srsran_gnb_config
            generate_oai_gnb_config
            ;;
    esac
    
    generate_ric_config
    
    echo "✅ gNB configuration files generated successfully"
}

# Execute main function
main "$@"
