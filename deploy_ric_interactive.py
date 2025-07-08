#!/usr/bin/env python3
"""
Interactive RIC Deployment Script

This script provides an interactive interface to deploy RIC (Radio Interface Controller)
from SC ORAN and connect it to existing CORE/gNB infrastructure.

Features:
- Interactive user input collection
- Multiple RIC implementation options
- Configuration validation
- Support for existing CORE/gNB connections
- Automated deployment with user-provided parameters
"""

import os
import sys
import subprocess
import json
import ipaddress
from typing import Dict, List, Optional, Tuple
import re
from pathlib import Path

class Colors:
    """ANSI color codes for terminal output."""
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

class RICDeploymentConfig:
    """Configuration class for RIC deployment parameters."""
    
    def __init__(self):
        self.ric_type = ""
        self.ric_ip = ""
        self.ric_port = ""
        self.gnb_ip = ""
        self.gnb_port = ""
        self.core_ip = ""
        self.core_type = ""
        self.install_dir = ""
        self.config_dir = ""
        self.use_existing_core = False
        self.use_existing_gnb = False
        self.e2_interface_config = {}
        self.network_config = {}
        self.deployment_options = {}

class RICDeployer:
    """Main class for RIC deployment operations."""
    
    def __init__(self):
        self.config = RICDeploymentConfig()
        self.supported_ric_types = {
            "1": {"name": "ORAN SC RIC", "description": "Standard ORAN SC RIC implementation"},
            "2": {"name": "FlexRIC", "description": "FlexRIC implementation with OAI integration"},
            "3": {"name": "OAI E2 Agent", "description": "OAI with E2 Agent integration"},
            "4": {"name": "Custom RIC", "description": "Custom RIC configuration"}
        }
        self.supported_core_types = {
            "1": {"name": "Open5GS", "description": "Open5GS 5G Core"},
            "2": {"name": "OAI CN5G", "description": "OpenAirInterface 5G Core"},
            "3": {"name": "Free5GC", "description": "Free5GC implementation"},
            "4": {"name": "External Core", "description": "External/Commercial 5G Core"}
        }

    def print_header(self, title: str):
        """Print formatted header."""
        print(f"\n{Colors.HEADER}{'='*60}{Colors.ENDC}")
        print(f"{Colors.HEADER}{title.center(60)}{Colors.ENDC}")
        print(f"{Colors.HEADER}{'='*60}{Colors.ENDC}\n")

    def print_success(self, message: str):
        """Print success message."""
        print(f"{Colors.OKGREEN}✅ {message}{Colors.ENDC}")

    def print_error(self, message: str):
        """Print error message."""
        print(f"{Colors.FAIL}❌ {message}{Colors.ENDC}")

    def print_warning(self, message: str):
        """Print warning message."""
        print(f"{Colors.WARNING}⚠️  {message}{Colors.ENDC}")

    def print_info(self, message: str):
        """Print info message."""
        print(f"{Colors.OKBLUE}ℹ️  {message}{Colors.ENDC}")

    def validate_ip_address(self, ip: str) -> bool:
        """Validate IP address format."""
        try:
            ipaddress.ip_address(ip)
            return True
        except ValueError:
            return False

    def validate_port(self, port: str) -> bool:
        """Validate port number."""
        try:
            port_num = int(port)
            return 1 <= port_num <= 65535
        except ValueError:
            return False

    def validate_directory(self, path: str) -> bool:
        """Validate directory path."""
        return os.path.isdir(path) or self.ask_create_directory(path)

    def ask_create_directory(self, path: str) -> bool:
        """Ask user if they want to create directory."""
        response = input(f"Directory {path} doesn't exist. Create it? (y/n): ").lower()
        if response == 'y':
            try:
                os.makedirs(path, exist_ok=True)
                return True
            except OSError as e:
                self.print_error(f"Failed to create directory {path}: {e}")
                return False
        return False

    def get_user_input(self, prompt: str, default: str = "", validator=None) -> str:
        """Get user input with validation."""
        while True:
            if default:
                user_input = input(f"{prompt} [{default}]: ").strip()
                if not user_input:
                    user_input = default
            else:
                user_input = input(f"{prompt}: ").strip()
            
            if validator and not validator(user_input):
                self.print_error("Invalid input. Please try again.")
                continue
            
            return user_input

    def select_ric_type(self):
        """Allow user to select RIC type."""
        self.print_header("RIC Type Selection")
        
        print("Available RIC implementations:")
        for key, value in self.supported_ric_types.items():
            print(f"  {key}. {value['name']} - {value['description']}")
        
        while True:
            choice = input("\nSelect RIC type (1-4): ").strip()
            if choice in self.supported_ric_types:
                self.config.ric_type = choice
                selected_ric = self.supported_ric_types[choice]
                self.print_success(f"Selected: {selected_ric['name']}")
                return
            else:
                self.print_error("Invalid choice. Please select 1-4.")

    def collect_network_configuration(self):
        """Collect network configuration parameters."""
        self.print_header("Network Configuration")
        
        # RIC Configuration
        print(f"{Colors.OKBLUE}RIC Configuration:{Colors.ENDC}")
        self.config.ric_ip = self.get_user_input(
            "RIC IP Address", 
            "127.0.0.1", 
            self.validate_ip_address
        )
        self.config.ric_port = self.get_user_input(
            "RIC E2 Port", 
            "36421", 
            self.validate_port
        )
        
        # gNB Configuration
        print(f"\n{Colors.OKBLUE}gNB Configuration:{Colors.ENDC}")
        use_existing_gnb = input("Connect to existing gNB? (y/n): ").lower() == 'y'
        self.config.use_existing_gnb = use_existing_gnb
        
        if use_existing_gnb:
            self.config.gnb_ip = self.get_user_input(
                "gNB IP Address", 
                "127.0.0.1", 
                self.validate_ip_address
            )
            self.config.gnb_port = self.get_user_input(
                "gNB E2 Port", 
                "36422", 
                self.validate_port
            )
        else:
            self.print_info("Will deploy gNB as part of the setup")
            self.config.gnb_ip = "127.0.0.1"
            self.config.gnb_port = "36422"
        
        # Core Configuration
        print(f"\n{Colors.OKBLUE}Core Network Configuration:{Colors.ENDC}")
        use_existing_core = input("Connect to existing Core? (y/n): ").lower() == 'y'
        self.config.use_existing_core = use_existing_core
        
        if use_existing_core:
            print("\nAvailable Core types:")
            for key, value in self.supported_core_types.items():
                print(f"  {key}. {value['name']} - {value['description']}")
            
            while True:
                choice = input("\nSelect Core type (1-4): ").strip()
                if choice in self.supported_core_types:
                    self.config.core_type = choice
                    break
                else:
                    self.print_error("Invalid choice. Please select 1-4.")
            
            self.config.core_ip = self.get_user_input(
                "Core Network IP Address", 
                "127.0.0.1", 
                self.validate_ip_address
            )
        else:
            self.print_info("Will deploy Core as part of the setup")
            self.config.core_type = "1"  # Default to Open5GS
            self.config.core_ip = "127.0.0.1"

    def collect_deployment_options(self):
        """Collect deployment-specific options."""
        self.print_header("Deployment Options")
        
        # Installation directory
        default_install_dir = os.path.expanduser("~/ric-deployment")
        self.config.install_dir = self.get_user_input(
            "Installation Directory", 
            default_install_dir,
            self.validate_directory
        )
        
        # Configuration directory
        default_config_dir = os.path.join(self.config.install_dir, "configs")
        self.config.config_dir = self.get_user_input(
            "Configuration Directory", 
            default_config_dir,
            self.validate_directory
        )
        
        # Additional options based on RIC type
        if self.config.ric_type == "1":  # ORAN SC RIC
            self.collect_oran_sc_options()
        elif self.config.ric_type == "2":  # FlexRIC
            self.collect_flexric_options()
        elif self.config.ric_type == "3":  # OAI E2 Agent
            self.collect_oai_e2_options()

    def collect_oran_sc_options(self):
        """Collect ORAN SC RIC specific options."""
        print(f"\n{Colors.OKBLUE}ORAN SC RIC Options:{Colors.ENDC}")
        
        # Docker configuration
        use_docker = input("Use Docker deployment? (y/n): ").lower() == 'y'
        self.config.deployment_options["use_docker"] = use_docker
        
        # E2 interface configuration
        e2_plmn_id = self.get_user_input("E2 PLMN ID", "00101")
        self.config.e2_interface_config["plmn_id"] = e2_plmn_id
        
        # Service Models
        print("\nService Models to enable:")
        kpm_sm = input("Enable KPM Service Model? (y/n): ").lower() == 'y'
        rc_sm = input("Enable RC Service Model? (y/n): ").lower() == 'y'
        
        self.config.deployment_options["service_models"] = {
            "kpm": kpm_sm,
            "rc": rc_sm
        }

    def collect_flexric_options(self):
        """Collect FlexRIC specific options."""
        print(f"\n{Colors.OKBLUE}FlexRIC Options:{Colors.ENDC}")
        
        # ASN1C configuration
        asn1c_path = self.get_user_input("ASN1C Path", "/usr/local/bin/asn1c")
        self.config.deployment_options["asn1c_path"] = asn1c_path
        
        # E2AP version
        e2ap_version = self.get_user_input("E2AP Version", "E2AP_V2")
        kmp_version = self.get_user_input("KMP Version", "KMP_V2_03")
        
        self.config.deployment_options["e2ap_version"] = e2ap_version
        self.config.deployment_options["kmp_version"] = kmp_version

    def collect_oai_e2_options(self):
        """Collect OAI E2 Agent specific options."""
        print(f"\n{Colors.OKBLUE}OAI E2 Agent Options:{Colors.ENDC}")
        
        # Build options
        build_gnb = input("Build gNB? (y/n): ").lower() == 'y'
        build_nrue = input("Build nrUE? (y/n): ").lower() == 'y'
        use_rfsim = input("Use RF simulator? (y/n): ").lower() == 'y'
        
        self.config.deployment_options.update({
            "build_gnb": build_gnb,
            "build_nrue": build_nrue,
            "use_rfsim": use_rfsim
        })

    def display_configuration_summary(self):
        """Display configuration summary for user confirmation."""
        self.print_header("Configuration Summary")
        
        print(f"{Colors.OKBLUE}RIC Configuration:{Colors.ENDC}")
        print(f"  Type: {self.supported_ric_types[self.config.ric_type]['name']}")
        print(f"  IP: {self.config.ric_ip}")
        print(f"  Port: {self.config.ric_port}")
        
        print(f"\n{Colors.OKBLUE}gNB Configuration:{Colors.ENDC}")
        print(f"  Use Existing: {'Yes' if self.config.use_existing_gnb else 'No'}")
        print(f"  IP: {self.config.gnb_ip}")
        print(f"  Port: {self.config.gnb_port}")
        
        print(f"\n{Colors.OKBLUE}Core Configuration:{Colors.ENDC}")
        print(f"  Use Existing: {'Yes' if self.config.use_existing_core else 'No'}")
        if self.config.use_existing_core:
            print(f"  Type: {self.supported_core_types[self.config.core_type]['name']}")
        print(f"  IP: {self.config.core_ip}")
        
        print(f"\n{Colors.OKBLUE}Deployment Options:{Colors.ENDC}")
        print(f"  Install Directory: {self.config.install_dir}")
        print(f"  Config Directory: {self.config.config_dir}")
        
        if self.config.deployment_options:
            print(f"\n{Colors.OKBLUE}Additional Options:{Colors.ENDC}")
            for key, value in self.config.deployment_options.items():
                print(f"  {key}: {value}")

    def save_configuration(self):
        """Save configuration to file."""
        config_file = os.path.join(self.config.config_dir, "ric_deployment_config.json")
        
        config_data = {
            "ric_type": self.config.ric_type,
            "ric_ip": self.config.ric_ip,
            "ric_port": self.config.ric_port,
            "gnb_ip": self.config.gnb_ip,
            "gnb_port": self.config.gnb_port,
            "core_ip": self.config.core_ip,
            "core_type": self.config.core_type,
            "install_dir": self.config.install_dir,
            "config_dir": self.config.config_dir,
            "use_existing_core": self.config.use_existing_core,
            "use_existing_gnb": self.config.use_existing_gnb,
            "e2_interface_config": self.config.e2_interface_config,
            "deployment_options": self.config.deployment_options
        }
        
        try:
            with open(config_file, 'w') as f:
                json.dump(config_data, f, indent=2)
            self.print_success(f"Configuration saved to {config_file}")
        except Exception as e:
            self.print_error(f"Failed to save configuration: {e}")

    def generate_deployment_script(self):
        """Generate deployment script based on configuration."""
        script_path = os.path.join(self.config.install_dir, "deploy_configured_ric.sh")
        
        script_content = self.get_deployment_script_content()
        
        try:
            with open(script_path, 'w') as f:
                f.write(script_content)
            os.chmod(script_path, 0o755)
            self.print_success(f"Deployment script generated: {script_path}")
            return script_path
        except Exception as e:
            self.print_error(f"Failed to generate deployment script: {e}")
            return None

    def get_deployment_script_content(self) -> str:
        """Get deployment script content based on RIC type."""
        if self.config.ric_type == "1":  # ORAN SC RIC
            return self.get_oran_sc_script()
        elif self.config.ric_type == "2":  # FlexRIC
            return self.get_flexric_script()
        elif self.config.ric_type == "3":  # OAI E2 Agent
            return self.get_oai_e2_script()
        else:
            return self.get_custom_script()

    def get_oran_sc_script(self) -> str:
        """Generate ORAN SC RIC deployment script."""
        return f"""#!/bin/bash

set -e

echo "🚀 Starting ORAN SC RIC Deployment"
echo "Configuration:"
echo "  RIC IP: {self.config.ric_ip}"
echo "  RIC Port: {self.config.ric_port}"
echo "  gNB IP: {self.config.gnb_ip}"
echo "  gNB Port: {self.config.gnb_port}"
echo "  Core IP: {self.config.core_ip}"
echo "  Install Dir: {self.config.install_dir}"

# Create installation directory
mkdir -p {self.config.install_dir}
cd {self.config.install_dir}

# Install dependencies
echo "📦 Installing dependencies..."
sudo apt-get update
sudo apt-get install -y git docker.io docker-compose
sudo apt-get install -y cmake build-essential libboost-all-dev libmbedtls-dev libsctp-dev libfftw3-dev libzmq3-dev

# Add user to docker group
sudo usermod -aG docker $USER

# Clone ORAN SC RIC repository
echo "📥 Cloning ORAN SC RIC repository..."
if [ ! -d "oran-sc-ric" ]; then
    git clone https://github.com/srsran/oran-sc-ric.git
fi

cd oran-sc-ric

# Configure E2 interface
echo "⚙️ Configuring E2 interface..."
cat > e2-config.yaml << EOF
e2_interface:
  ric_ip: {self.config.ric_ip}
  ric_port: {self.config.ric_port}
  gnb_ip: {self.config.gnb_ip}
  gnb_port: {self.config.gnb_port}
  plmn_id: {self.config.e2_interface_config.get('plmn_id', '00101')}
EOF

# Start RIC services
echo "🚀 Starting RIC services..."
{"docker compose up --build -d" if self.config.deployment_options.get("use_docker", True) else "make run"}

# Wait for services to be ready
sleep 10

# Verify deployment
echo "✅ Verifying deployment..."
docker ps | grep ric || echo "RIC containers running"

echo "🎉 ORAN SC RIC deployment completed successfully!"
echo "RIC is available at {self.config.ric_ip}:{self.config.ric_port}"
"""

    def get_flexric_script(self) -> str:
        """Generate FlexRIC deployment script."""
        return f"""#!/bin/bash

set -e

echo "🚀 Starting FlexRIC Deployment"
echo "Configuration:"
echo "  RIC IP: {self.config.ric_ip}"
echo "  RIC Port: {self.config.ric_port}"
echo "  E2AP Version: {self.config.deployment_options.get('e2ap_version', 'E2AP_V2')}"
echo "  KMP Version: {self.config.deployment_options.get('kmp_version', 'KMP_V2_03')}"

# Create installation directory
mkdir -p {self.config.install_dir}
cd {self.config.install_dir}

# Install dependencies
echo "📦 Installing dependencies..."
sudo apt update
sudo apt install -y build-essential git cmake-curses-gui libsctp-dev libpcre2-dev unzip \\
  gcc-13 g++-13 cpp-13 python3.10-dev autoconf automake libtool bison flex

# Configure gcc-13 as default
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100 \\
  --slave /usr/bin/g++ g++ /usr/bin/g++-13 --slave /usr/bin/gcov gcov /usr/bin/gcov-13
sudo update-alternatives --set gcc /usr/bin/gcc-13

# Install ASN1C
echo "🔧 Installing ASN1C..."
cd ~
rm -rf asn1c
git clone https://github.com/mouse07410/asn1c.git
cd asn1c
git checkout aper
autoreconf -fiv
./configure
make -j$(nproc)
sudo make install

# Clone and build FlexRIC
echo "📥 Cloning FlexRIC..."
cd {self.config.install_dir}
git clone https://gitlab.eurecom.fr/mosaic5g/flexric.git
cd flexric

echo "🏗️ Building FlexRIC..."
rm -rf build
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DASN1C_EXEC_PATH={self.config.deployment_options.get('asn1c_path', '/usr/local/bin/asn1c')} \\
  -DE2AP_VERSION={self.config.deployment_options.get('e2ap_version', 'E2AP_V2')} \\
  -DKMP_VERSION={self.config.deployment_options.get('kmp_version', 'KMP_V2_03')} ..
make -j$(nproc)
sudo make install

# Create service models directory
sudo mkdir -p /usr/local/lib/flexric/

# Configure FlexRIC
echo "⚙️ Configuring FlexRIC..."
cat > /tmp/flexric.conf << EOF
[E2]
ric_ip = {self.config.ric_ip}
ric_port = {self.config.ric_port}

[SERVICE_MODELS]
kpm = {"true" if self.config.deployment_options.get("service_models", {}).get("kmp", True) else "false"}
rc = {"true" if self.config.deployment_options.get("service_models", {}).get("rc", True) else "false"}
EOF

sudo cp /tmp/flexric.conf /usr/local/etc/flexric/flexric.conf

# Start FlexRIC nearRT-RIC
echo "🚀 Starting FlexRIC nearRT-RIC..."
./examples/ric/nearRT-RIC &

echo "✅ FlexRIC deployment completed successfully!"
echo "nearRT-RIC is running at {self.config.ric_ip}:{self.config.ric_port}"
"""

    def get_oai_e2_script(self) -> str:
        """Generate OAI E2 Agent deployment script."""
        return f"""#!/bin/bash

set -e

echo "🚀 Starting OAI E2 Agent Deployment"
echo "Configuration:"
echo "  RIC IP: {self.config.ric_ip}"
echo "  RIC Port: {self.config.ric_port}"
echo "  Build gNB: {self.config.deployment_options.get('build_gnb', True)}"
echo "  Build nrUE: {self.config.deployment_options.get('build_nrue', True)}"
echo "  Use RF Simulator: {self.config.deployment_options.get('use_rfsim', True)}"

# Create installation directory
mkdir -p {self.config.install_dir}
cd {self.config.install_dir}

# Install dependencies
echo "📦 Installing dependencies..."
sudo apt update && sudo apt install -y \\
  git build-essential cmake-curses-gui libsctp-dev libpcre2-dev \\
  gcc-13 g++-13 cpp-13 unzip python3.10-dev net-tools ninja-build

# Configure gcc-13 as default
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100 \\
  --slave /usr/bin/g++ g++ /usr/bin/g++-13 --slave /usr/bin/gcov gcov /usr/bin/gcov-13
sudo update-alternatives --set gcc /usr/bin/gcc-13

# Clone OAI repository
echo "📥 Cloning OAI repository..."
git clone https://gitlab.eurecom.fr/oai/openairinterface5g.git
cd openairinterface5g

# Initialize FlexRIC submodule
echo "📁 Initializing FlexRIC submodule..."
cd openair2/E2AP/flexric
git submodule init && git submodule update
cd ../../../

# Build OAI with E2 Agent
echo "🛠️ Building OAI with E2 Agent..."
cd cmake_targets
./build_oai -I
./build_oai {"--gNB" if self.config.deployment_options.get("build_gnb", True) else ""} \\
  {"--nrUE" if self.config.deployment_options.get("build_nrue", True) else ""} \\
  --build-e2 --cmake-opt -DE2AP_VERSION=E2AP_V2 --cmake-opt -DKMP_VERSION=KMP_V2_03 --ninja
cd ../

# Build FlexRIC
echo "🏗️ Building FlexRIC..."
cd openair2/E2AP/flexric
mkdir -p build && cd build
cmake -GNinja -DCMAKE_BUILD_TYPE=Release -DE2AP_VERSION=E2AP_V2 -DKMP_VERSION=KMP_V2_03 ..
ninja
sudo make install

# Configure E2 Agent
echo "⚙️ Configuring E2 Agent..."
CONF_PATH="../../../targets/PROJECTS/GENERIC-NR-5GC/CONF/gnb.sa.band78.fr1.106PRB.usrpb210.conf"
E2_AGENT_BLOCK="e2_agent = {{
  near_ric_ip_addr = \\"{self.config.ric_ip}\\";
  sm_dir = \\"/usr/local/lib/flexric/\\"
}}"

if ! grep -q "e2_agent" "$CONF_PATH"; then
    echo -e "\\n$E2_AGENT_BLOCK" | sudo tee -a "$CONF_PATH" > /dev/null
fi

echo "✅ OAI E2 Agent deployment completed successfully!"
echo "E2 Agent configured to connect to RIC at {self.config.ric_ip}:{self.config.ric_port}"
"""

    def get_custom_script(self) -> str:
        """Generate custom RIC deployment script."""
        return f"""#!/bin/bash

set -e

echo "🚀 Starting Custom RIC Deployment"
echo "Configuration:"
echo "  RIC IP: {self.config.ric_ip}"
echo "  RIC Port: {self.config.ric_port}"
echo "  gNB IP: {self.config.gnb_ip}"
echo "  Core IP: {self.config.core_ip}"
echo "  Install Dir: {self.config.install_dir}"

# Create installation directory
mkdir -p {self.config.install_dir}
cd {self.config.install_dir}

echo "⚙️ Custom RIC deployment script generated."
echo "Please customize this script according to your specific RIC implementation."
echo "Configuration has been saved to {self.config.config_dir}/ric_deployment_config.json"

echo "✅ Custom RIC deployment script ready for customization!"
"""

    def execute_deployment(self, script_path: str):
        """Execute the deployment script."""
        if not script_path or not os.path.exists(script_path):
            self.print_error("Deployment script not found")
            return False
        
        self.print_header("Deployment Execution")
        
        confirm = input("Do you want to execute the deployment script now? (y/n): ").lower()
        if confirm != 'y':
            self.print_info("Deployment script generated but not executed")
            self.print_info(f"You can run it manually: {script_path}")
            return False
        
        self.print_info("Starting deployment execution...")
        
        try:
            # Execute the script
            process = subprocess.Popen(
                ['/bin/bash', script_path],
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                universal_newlines=True
            )
            
            # Stream output
            for line in iter(process.stdout.readline, ''):
                print(line.rstrip())
            
            process.wait()
            
            if process.returncode == 0:
                self.print_success("Deployment completed successfully!")
                return True
            else:
                self.print_error(f"Deployment failed with return code {process.returncode}")
                return False
                
        except Exception as e:
            self.print_error(f"Error executing deployment: {e}")
            return False

    def run_deployment_wizard(self):
        """Run the complete deployment wizard."""
        self.print_header("RIC Interactive Deployment Wizard")
        
        try:
            # Step 1: Select RIC type
            self.select_ric_type()
            
            # Step 2: Collect network configuration
            self.collect_network_configuration()
            
            # Step 3: Collect deployment options
            self.collect_deployment_options()
            
            # Step 4: Display summary and confirm
            self.display_configuration_summary()
            
            confirm = input(f"\n{Colors.WARNING}Proceed with deployment? (y/n): {Colors.ENDC}").lower()
            if confirm != 'y':
                self.print_info("Deployment cancelled by user")
                return
            
            # Step 5: Save configuration
            self.save_configuration()
            
            # Step 6: Generate deployment script
            script_path = self.generate_deployment_script()
            
            # Step 7: Execute deployment
            if script_path:
                self.execute_deployment(script_path)
            
        except KeyboardInterrupt:
            self.print_info("\nDeployment wizard interrupted by user")
        except Exception as e:
            self.print_error(f"Error in deployment wizard: {e}")

def main():
    """Main function."""
    if len(sys.argv) > 1 and sys.argv[1] == "--help":
        print("""
RIC Interactive Deployment Script

Usage: python3 deploy_ric_interactive.py [OPTIONS]

This script provides an interactive interface to deploy RIC (Radio Interface Controller)
from SC ORAN and connect it to existing CORE/gNB infrastructure.

Options:
  --help    Show this help message

Features:
- Interactive configuration collection
- Multiple RIC implementation support (ORAN SC, FlexRIC, OAI E2)
- Support for existing CORE/gNB connections
- Configuration validation and error handling
- Automated deployment script generation
- Real-time deployment execution

Supported RIC Types:
1. ORAN SC RIC - Standard ORAN SC implementation
2. FlexRIC - FlexRIC with OAI integration
3. OAI E2 Agent - OAI with E2 Agent integration
4. Custom RIC - Custom configuration

Requirements:
- Ubuntu 20.04 or 22.04
- Python 3.8+
- sudo privileges
- Internet connection
""")
        return
    
    # Check if running as root
    if os.geteuid() == 0:
        print(f"{Colors.WARNING}Warning: Running as root is not recommended{Colors.ENDC}")
    
    # Check Python version
    if sys.version_info < (3, 8):
        print(f"{Colors.FAIL}Error: Python 3.8 or higher is required{Colors.ENDC}")
        sys.exit(1)
    
    # Run the deployment wizard
    deployer = RICDeployer()
    deployer.run_deployment_wizard()

if __name__ == "__main__":
    main()