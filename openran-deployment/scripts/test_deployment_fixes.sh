#!/bin/bash

# Integration test to verify the specific issues from the problem statement are fixed
# This simulates the exact errors that were reported

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIGS_DIR="$SCRIPT_DIR/../configs"

echo "🎯 Testing specific fixes for reported deployment issues..."

# Generate fresh configurations
cd "$SCRIPT_DIR"
./generate_gnb_config.sh
./generate_ue_config.sh

echo ""
echo "🔍 Checking Fix #1: gNB UHD Radio Plugin Error"
echo "   Original error: 'Failed to load RF plugin libsrsran_radio_uhd.so'"
echo "   Expected fix: gNB should use ZMQ instead of UHD"

if grep -q "device_driver: uhd" "$CONFIGS_DIR/gnb_config.yaml"; then
    echo "❌ FAILED: gNB still configured for UHD hardware"
    echo "   This would cause: 'Failed to load RF plugin libsrsran_radio_uhd.so'"
    exit 1
elif grep -q "device_driver: zmq" "$CONFIGS_DIR/gnb_config.yaml"; then
    echo "✅ FIXED: gNB now uses ZMQ (software simulation)"
    echo "   This eliminates the UHD plugin error"
else
    echo "❌ FAILED: gNB device_driver not found"
    exit 1
fi

echo ""
echo "🔍 Checking Fix #2: UE PCAP Configuration Error"
echo "   Original error: 'unrecognised option pcap.filename'"
echo "   Expected fix: UE should use mac_filename instead of filename"

if grep -A5 "\[pcap\]" "$CONFIGS_DIR/ue1_config.conf" | grep -q "^filename = "; then
    echo "❌ FAILED: UE still uses deprecated 'filename' in pcap section"
    echo "   This would cause: 'unrecognised option pcap.filename'"
    exit 1
elif grep -A5 "\[pcap\]" "$CONFIGS_DIR/ue1_config.conf" | grep -q "mac_filename = "; then
    echo "✅ FIXED: UE now uses 'mac_filename' (correct format)"
    echo "   This eliminates the unrecognised option error"
else
    echo "❌ FAILED: UE pcap configuration not found"
    exit 1
fi

echo ""
echo "🔍 Checking Fix #3: ZMQ Port Configuration"
echo "   Ensuring proper gNB-UE communication setup"

# Extract port configurations
gnb_tx=$(grep "tx_port=" "$CONFIGS_DIR/gnb_config.yaml" | sed 's/.*tx_port=tcp:\/\/\*:\([0-9]*\).*/\1/')
gnb_rx=$(grep "rx_port=" "$CONFIGS_DIR/gnb_config.yaml" | sed 's/.*rx_port=tcp:\/\/localhost:\([0-9]*\).*/\1/')
ue1_tx=$(grep "tx_port=" "$CONFIGS_DIR/ue1_config.conf" | sed 's/.*tx_port=tcp:\/\/\*:\([0-9]*\).*/\1/')
ue1_rx=$(grep "rx_port=" "$CONFIGS_DIR/ue1_config.conf" | sed 's/.*rx_port=tcp:\/\/localhost:\([0-9]*\).*/\1/')

echo "   gNB: TX=$gnb_tx, RX=$gnb_rx"
echo "   UE1: TX=$ue1_tx, RX=$ue1_rx"

if [[ "$gnb_tx" == "$ue1_rx" && "$gnb_rx" == "$ue1_tx" ]]; then
    echo "✅ FIXED: ZMQ ports properly configured for gNB-UE communication"
    echo "   gNB TX($gnb_tx) -> UE1 RX($ue1_rx) ✓"
    echo "   UE1 TX($ue1_tx) -> gNB RX($gnb_rx) ✓"
else
    echo "❌ FAILED: ZMQ port configuration mismatch"
    echo "   This could cause communication failures"
    exit 1
fi

echo ""
echo "🎉 All specific deployment issues have been FIXED!"
echo ""
echo "📋 Summary of Fixes:"
echo "   ✅ gNB UHD Error: Resolved by switching to ZMQ simulation"
echo "   ✅ UE PCAP Error: Resolved by using correct configuration format"
echo "   ✅ Port Config: Proper ZMQ port mapping for communication"
echo ""
echo "🚀 The deployment should now work without these errors!"
echo ""
echo "📝 Next Steps:"
echo "   • Run the full deployment: ./deploy.sh"
echo "   • Monitor for new issues in the logs"
echo "   • Verify UE connectivity tests pass"