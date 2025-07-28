#!/usr/bin/env python3
"""
Test script for the interactive RIC deployment script.
This validates core functionality without user interaction.
"""

import sys
import os
import tempfile
import json

# Add the script directory to the path
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import deploy_ric_interactive

def test_basic_functionality():
    """Test basic script functionality."""
    print("🧪 Testing basic functionality...")
    
    deployer = deploy_ric_interactive.RICDeployer()
    
    # Test validation functions
    assert deployer.validate_ip_address("192.168.1.1") == True
    assert deployer.validate_ip_address("invalid") == False
    assert deployer.validate_port("8080") == True
    assert deployer.validate_port("99999") == False
    
    print("✅ Validation functions working correctly")
    
    # Test configuration
    config = deployer.config
    config.ric_type = "1"
    config.ric_ip = "127.0.0.1"
    config.ric_port = "36421"
    config.gnb_ip = "127.0.0.1"
    config.gnb_port = "36422"
    config.core_ip = "127.0.0.1"
    config.core_type = "1"
    config.use_existing_core = False
    config.use_existing_gnb = False
    config.e2_interface_config = {"plmn_id": "00101"}
    config.deployment_options = {"use_docker": True}
    
    # Test script generation
    with tempfile.TemporaryDirectory() as temp_dir:
        config.install_dir = temp_dir
        config.config_dir = os.path.join(temp_dir, "configs")
        os.makedirs(config.config_dir, exist_ok=True)
        
        # Test configuration saving
        deployer.save_configuration()
        config_file = os.path.join(config.config_dir, "ric_deployment_config.json")
        assert os.path.exists(config_file)
        
        # Test script generation
        script_path = deployer.generate_deployment_script()
        assert script_path is not None
        assert os.path.exists(script_path)
        
        # Test script content
        with open(script_path, 'r') as f:
            content = f.read()
            assert "ORAN SC RIC" in content
            assert config.ric_ip in content
            assert config.ric_port in content
    
    print("✅ Script generation working correctly")

def test_all_ric_types():
    """Test script generation for all RIC types."""
    print("🧪 Testing all RIC types...")
    
    deployer = deploy_ric_interactive.RICDeployer()
    config = deployer.config
    
    # Common configuration
    config.ric_ip = "127.0.0.1"
    config.ric_port = "36421"
    config.gnb_ip = "127.0.0.1"
    config.gnb_port = "36422"
    config.core_ip = "127.0.0.1"
    config.core_type = "1"
    config.use_existing_core = False
    config.use_existing_gnb = False
    
    ric_types = ["1", "2", "3", "4"]
    
    for ric_type in ric_types:
        config.ric_type = ric_type
        
        # Set type-specific options
        if ric_type == "1":  # ORAN SC RIC
            config.deployment_options = {"use_docker": True, "service_models": {"kmp": True, "rc": True}}
            config.e2_interface_config = {"plmn_id": "00101"}
        elif ric_type == "2":  # FlexRIC
            config.deployment_options = {"asn1c_path": "/usr/local/bin/asn1c", "e2ap_version": "E2AP_V2", "kmp_version": "KMP_V2_03"}
        elif ric_type == "3":  # OAI E2 Agent
            config.deployment_options = {"build_gnb": True, "build_nrue": True, "use_rfsim": True}
        else:  # Custom
            config.deployment_options = {}
        
        with tempfile.TemporaryDirectory() as temp_dir:
            config.install_dir = temp_dir
            config.config_dir = os.path.join(temp_dir, "configs")
            os.makedirs(config.config_dir, exist_ok=True)
            
            script_path = deployer.generate_deployment_script()
            assert script_path is not None
            assert os.path.exists(script_path)
            
            # Test script content contains expected elements
            with open(script_path, 'r') as f:
                content = f.read()
                assert "#!/bin/bash" in content
                assert config.ric_ip in content
                assert config.ric_port in content
        
        print(f"✅ RIC type {ric_type} script generation working")

def test_configuration_validation():
    """Test configuration validation."""
    print("🧪 Testing configuration validation...")
    
    deployer = deploy_ric_interactive.RICDeployer()
    
    # Test IP validation
    valid_ips = ["127.0.0.1", "192.168.1.1", "10.0.0.1", "172.16.0.1"]
    invalid_ips = ["256.1.1.1", "invalid", "192.168.1", "192.168.1.1.1"]
    
    for ip in valid_ips:
        assert deployer.validate_ip_address(ip) == True, f"IP {ip} should be valid"
    
    for ip in invalid_ips:
        assert deployer.validate_ip_address(ip) == False, f"IP {ip} should be invalid"
    
    # Test port validation
    valid_ports = ["1", "80", "443", "8080", "36421", "65535"]
    invalid_ports = ["0", "65536", "invalid", "-1", "99999"]
    
    for port in valid_ports:
        assert deployer.validate_port(port) == True, f"Port {port} should be valid"
    
    for port in invalid_ports:
        assert deployer.validate_port(port) == False, f"Port {port} should be invalid"
    
    print("✅ Configuration validation working correctly")

def main():
    """Run all tests."""
    print("🚀 Starting RIC Interactive Deployment Script Tests")
    print("=" * 50)
    
    try:
        test_basic_functionality()
        test_all_ric_types()
        test_configuration_validation()
        
        print("\n🎉 All tests passed successfully!")
        print("✅ RIC Interactive Deployment Script is ready for use")
        return 0
        
    except Exception as e:
        print(f"\n❌ Test failed: {e}")
        import traceback
        traceback.print_exc()
        return 1

if __name__ == "__main__":
    sys.exit(main())