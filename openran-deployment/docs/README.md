# OpenRAN Stack Deployment Guide

## 🚀 Overview

This comprehensive deployment solution provides a complete OpenRAN stack that includes:

- **RIC (Radio Intelligent Controller)** - Both O-RAN SC and FlexRIC implementations
- **gNodeB (5G Base Station)** - Using srsRAN or OpenAirInterface
- **5G Core Network** - Using Open5GS or OAI Core
- **Simulated UE (User Equipment)** - Multiple UE simulation support
- **xApps** - Monitoring and control applications
- **Monitoring Stack** - Prometheus, Grafana, and custom metrics

## 📋 Table of Contents

1. [Prerequisites](#prerequisites)
2. [Quick Start](#quick-start)
3. [Deployment Options](#deployment-options)
4. [Configuration](#configuration)
5. [Architecture](#architecture)
6. [Troubleshooting](#troubleshooting)
7. [Advanced Usage](#advanced-usage)
8. [Performance Tuning](#performance-tuning)
9. [Security Considerations](#security-considerations)
10. [FAQ](#faq)

## 🔧 Prerequisites

### System Requirements

- **Operating System**: Ubuntu 22.04 LTS (recommended)
- **RAM**: Minimum 8GB, recommended 16GB
- **CPU**: Minimum 4 cores, recommended 8 cores
- **Storage**: Minimum 20GB free space
- **Network**: Internet connectivity for downloading dependencies

### Software Dependencies

- Docker and Docker Compose
- Git
- Python 3.8+
- GCC/G++ compiler
- CMake
- Various networking tools

### Hardware Requirements

- **For simulation**: No special hardware required
- **For real deployment**: USRP B210 or similar SDR hardware

## 🚀 Quick Start

### 1. Clone and Setup

```bash
# Clone the repository
git clone https://github.com/antonio-emilio/ric-implementation.git
cd ric-implementation/openran-deployment

# Make scripts executable
chmod +x deploy.sh
chmod +x scripts/*.sh
```

### 2. Run Deployment

```bash
# Start the deployment wizard
./deploy.sh
```

The script will guide you through:
- Deployment type selection (O-RAN SC, FlexRIC/OAI, or both)
- Network configuration
- UE settings
- Advanced options

### 3. Verify Deployment

```bash
# Check deployment status
docker ps
./scripts/run_tests.sh

# Monitor in real-time
./configs/monitor_ues.sh
```

## 📊 Deployment Options

### Option 1: O-RAN SC RIC with srsRAN

**Best for**: Standards-compliant O-RAN deployment

```bash
# During deployment, select:
# 1) ORAN SC RIC (O-RAN Software Community)
```

**Components**:
- O-RAN SC RIC platform
- srsRAN gNodeB with E2 agent
- srsRAN UE
- Open5GS 5G Core

### Option 2: FlexRIC with OpenAirInterface

**Best for**: Research and development, flexibility

```bash
# During deployment, select:
# 2) FlexRIC + OAI (OpenAirInterface)
```

**Components**:
- FlexRIC nearRT-RIC
- OAI gNodeB with E2 agent
- OAI UE
- Built-in core network

### Option 3: Both Deployments

**Best for**: Comparison and testing

```bash
# During deployment, select:
# 3) Both (for comparison)
```

## ⚙️ Configuration

### Network Configuration

The deployment uses the following default network configuration:

```yaml
Core Network: 10.45.0.0/16
gNB IP: 10.45.1.100
UE IP Range: 10.45.1.200+
RIC IP: 127.0.0.1
E2 Port: 36421
```

### UE Configuration

Each UE is configured with:
- Unique IMSI starting from 001010000000001
- Separate network namespace
- Individual configuration files
- Dedicated log files

### Custom Configuration

You can modify configurations in the `configs/` directory:

```bash
# Edit deployment settings
vim configs/deployment_config.env

# Edit gNB configuration
vim configs/gnb_config.yaml

# Edit UE configuration
vim configs/ue1_config.conf
```

## 🏗️ Architecture

### High-Level Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    OpenRAN Deployment                      │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐    │
│  │     RIC     │    │    gNodeB   │    │  5G Core    │    │
│  │             │◄──►│             │◄──►│             │    │
│  │ O-RAN SC /  │    │ srsRAN /    │    │ Open5GS /   │    │
│  │ FlexRIC     │    │ OAI         │    │ OAI Core    │    │
│  └─────────────┘    └─────────────┘    └─────────────┘    │
│         ▲                   ▲                             │
│         │                   │                             │
│  ┌─────────────┐    ┌─────────────┐                      │
│  │    xApps    │    │  UE Sims    │                      │
│  │             │    │             │                      │
│  │ KPM, RC,    │    │ Multiple    │                      │
│  │ GTP, etc.   │    │ UEs         │                      │
│  └─────────────┘    └─────────────┘                      │
│                                                             │
├─────────────────────────────────────────────────────────────┤
│                 Monitoring & Management                     │
│                                                             │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐    │
│  │ Prometheus  │    │   Grafana   │    │   Logs      │    │
│  │             │    │             │    │             │    │
│  │ Metrics     │    │ Dashboards  │    │ Aggregated  │    │
│  │ Collection  │    │ & Alerts    │    │ Logging     │    │
│  └─────────────┘    └─────────────┘    └─────────────┘    │
└─────────────────────────────────────────────────────────────┘
```

### E2 Interface Architecture

```
┌─────────────────┐         E2 Interface         ┌─────────────────┐
│       RIC       │◄─────────────────────────────►│     gNodeB      │
│                 │                               │                 │
│  ┌───────────┐  │  ┌─────────────────────────┐  │  ┌───────────┐  │
│  │ nearRT-RIC│  │  │    E2 Messages          │  │  │ E2 Agent  │  │
│  │           │  │  │                         │  │  │           │  │
│  │ Service   │  │  │ • E2 Setup             │  │  │ Service   │  │
│  │ Models:   │  │  │ • RIC Subscription     │  │  │ Models:   │  │
│  │ - KPM     │  │  │ • RIC Indication       │  │  │ - KPM     │  │
│  │ - RC      │  │  │ • RIC Control          │  │  │ - RC      │  │
│  │ - GTP     │  │  │                         │  │  │ - GTP     │  │
│  └───────────┘  │  └─────────────────────────┘  │  └───────────┘  │
└─────────────────┘                               └─────────────────┘
```

## 🔍 Troubleshooting

### Common Issues

#### 1. Docker Permission Denied

```bash
# Solution: Add user to docker group
sudo usermod -aG docker $USER
# Log out and back in
```

#### 2. Port Already in Use

```bash
# Check what's using the port
sudo netstat -tlnp | grep :36421

# Kill the process or change port in config
```

#### 3. UE Not Registering

```bash
# Check UE logs
tail -f /tmp/ue1.log

# Check network namespace
sudo ip netns exec ue1 ip addr show

# Test connectivity
sudo ip netns exec ue1 ping 10.45.1.1
```

#### 4. E2 Connection Failed

```bash
# Check RIC logs
docker logs ric-e2mgr

# Check E2 port
nc -z 127.0.0.1 36421

# Restart RIC
docker restart ric-e2mgr
```

### Log Analysis

```bash
# View aggregated logs
tail -f logs/aggregated.log

# View specific component logs
tail -f logs/ric.log
tail -f logs/gnb.log
tail -f logs/ue1.log

# View test results
cat logs/test_report.html
```

## 🎯 Advanced Usage

### Manual Component Control

```bash
# Start only RIC
./scripts/deploy_oran_sc.sh --ric-only

# Start only gNB
./scripts/deploy_oran_sc.sh --gnb-only

# Start specific UE
sudo ip netns exec ue1 ./srsue configs/ue1_config.conf
```

### Custom xApp Development

```bash
# Create custom xApp
mkdir custom_xapp
cd custom_xapp

# Use FlexRIC SDK
cp -r /path/to/flexric/examples/xApp/c/template/* .

# Modify and build
make
./my_custom_xapp
```

### Performance Monitoring

```bash
# Monitor system resources
htop

# Monitor network traffic
sudo tcpdump -i any -w capture.pcap

# Monitor E2 messages
./scripts/monitor_e2.sh
```

## ⚡ Performance Tuning

### System Optimization

```bash
# Increase file descriptor limits
echo "* soft nofile 65536" >> /etc/security/limits.conf
echo "* hard nofile 65536" >> /etc/security/limits.conf

# Optimize network parameters
echo "net.core.rmem_max = 16777216" >> /etc/sysctl.conf
echo "net.core.wmem_max = 16777216" >> /etc/sysctl.conf
sysctl -p
```

### Container Optimization

```bash
# Increase Docker resources
# Edit /etc/docker/daemon.json
{
  "default-ulimits": {
    "nofile": {
      "name": "nofile",
      "hard": 65536,
      "soft": 65536
    }
  }
}

# Restart Docker
sudo systemctl restart docker
```

### gNB Performance

```bash
# Use CPU affinity
taskset -c 0-3 ./gnb -c config.yaml

# Adjust thread priorities
chrt -f 50 ./gnb -c config.yaml

# Monitor CPU usage
top -p $(pgrep gnb)
```

## 🔒 Security Considerations

### Network Security

```bash
# Enable firewall
sudo ufw enable

# Allow only necessary ports
sudo ufw allow 36421/tcp  # E2 interface
sudo ufw allow 38412/tcp  # N2 interface
sudo ufw allow 2152/udp   # GTP-U

# Block unnecessary services
sudo ufw deny 22/tcp      # SSH (if not needed)
```

### Container Security

```bash
# Run containers with limited privileges
docker run --user 1000:1000 --read-only ...

# Use security profiles
docker run --security-opt apparmor:docker-default ...

# Scan for vulnerabilities
docker scan image:tag
```

### Data Protection

```bash
# Encrypt sensitive configuration
gpg --cipher-algo AES256 --compress-algo 1 --symmetric config.yaml

# Use secrets management
docker secret create my_secret secret.txt
```

## ❓ FAQ

### Q: Can I run this on a VM?

A: Yes, but ensure:
- VM has enough resources (8GB+ RAM, 4+ cores)
- Nested virtualization is enabled
- Network performance is adequate

### Q: How do I add more UEs?

A: Modify the configuration:
```bash
# Edit deployment config
vim configs/deployment_config.env
# Change NUM_UES=5

# Regenerate configs
./scripts/generate_ue_config.sh

# Restart deployment
./restart.sh
```

### Q: Can I use real hardware?

A: Yes, for real deployment:
1. Replace ZMQ with actual RF configuration
2. Use USRP B210 or similar SDR
3. Modify RF parameters in gNB config
4. Ensure proper antenna connections

### Q: How do I debug E2 messages?

A: Use the built-in debugging:
```bash
# Enable debug logging
DEBUG_LOGGING=y ./deploy.sh

# Monitor E2 messages
wireshark -i lo -f "port 36421"

# Check E2 logs
grep -i "e2" /tmp/ric.log
```

### Q: What's the difference between O-RAN SC and FlexRIC?

A: 
- **O-RAN SC**: Standards-compliant, production-ready, containerized
- **FlexRIC**: Research-focused, lightweight, flexible development

### Q: How do I update the deployment?

A: 
```bash
# Pull latest changes
git pull origin main

# Clean existing deployment
./clean.sh

# Redeploy with new version
./deploy.sh
```

## 📞 Support

For issues and questions:
- Check the troubleshooting section
- Review logs in `logs/` directory
- Run diagnostic tests with `./scripts/run_tests.sh`
- Check the GitHub issues page

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests
5. Submit a pull request

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

---

*Last updated: $(date '+%Y-%m-%d')*