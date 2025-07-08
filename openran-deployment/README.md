# 🚀 OpenRAN Complete Deployment Solution

A comprehensive, automated deployment solution for OpenRAN stack including RIC, gNodeB, Core Network, and UE simulation.

## ✨ Features

- **🎯 Multiple Deployment Options**: O-RAN SC, FlexRIC/OAI, or both
- **🔄 Full Automation**: One-click deployment with interactive configuration
- **📊 Comprehensive Monitoring**: Prometheus, Grafana, and custom dashboards
- **🧪 Built-in Testing**: Automated test suite for validation
- **📱 Multi-UE Support**: Simulate multiple UEs with individual configurations
- **🛠️ Easy Management**: Start, stop, restart, and clean scripts
- **📖 Extensive Documentation**: Complete guides and API reference

## 📋 Quick Start

```bash
# Clone the repository
git clone https://github.com/antonio-emilio/ric-implementation.git
cd ric-implementation/openran-deployment

# Run the deployment wizard
./deploy.sh
```

The deployment wizard will guide you through:
1. **System Requirements Check** - Verify your system meets the requirements
2. **Configuration Wizard** - Interactive setup of network, RIC, and UE parameters
3. **Dependency Installation** - Automated installation of required packages
4. **Component Deployment** - Deploy RIC, gNodeB, Core, and UE components
5. **Testing & Validation** - Comprehensive testing suite
6. **Monitoring Setup** - Optional monitoring stack deployment

## 🏗️ Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                   OpenRAN Stack                             │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐     │
│  │     RIC     │    │    gNodeB   │    │  5G Core    │     │
│  │             │◄──►│             │◄──►│             │     │
│  │ O-RAN SC /  │    │ srsRAN /    │    │ Open5GS /   │     │
│  │ FlexRIC     │    │ OAI         │    │ OAI Core    │     │
│  └─────────────┘    └─────────────┘    └─────────────┘     │
│         ▲                   ▲                              │
│         │                   │                              │
│  ┌─────────────┐    ┌─────────────┐                       │
│  │    xApps    │    │  UE Sims    │                       │
│  │ KPM, RC,    │    │ Multiple    │                       │
│  │ GTP, etc.   │    │ UEs         │                       │
│  └─────────────┘    └─────────────┘                       │
├─────────────────────────────────────────────────────────────┤
│              Monitoring & Management                        │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐     │
│  │ Prometheus  │    │   Grafana   │    │   Logs      │     │
│  │ Metrics     │    │ Dashboards  │    │ Aggregated  │     │
│  └─────────────┘    └─────────────┘    └─────────────┘     │
└─────────────────────────────────────────────────────────────┘
```

## 📦 Components

### RIC (Radio Intelligent Controller)
- **O-RAN SC**: Standards-compliant O-RAN Software Community implementation
- **FlexRIC**: Lightweight, research-focused RIC implementation
- **xApps**: KPM monitoring, RC control, GTP tracking

### gNodeB (5G Base Station)
- **srsRAN**: Open-source 5G RAN implementation
- **OpenAirInterface**: Research-grade 5G implementation
- **E2 Agent**: Standards-compliant E2 interface

### Core Network
- **Open5GS**: Complete 5G standalone core
- **OAI Core**: Integrated with OAI RAN components

### UE Simulation
- **Multiple UEs**: Support for multiple simulated UEs
- **Network Namespaces**: Isolated network environments
- **Configurable Parameters**: IMSI, IP addresses, capabilities

## 🔧 Configuration Options

### Deployment Types
1. **O-RAN SC + srsRAN**: Production-ready, standards-compliant
2. **FlexRIC + OAI**: Research-focused, highly configurable
3. **Both**: Side-by-side comparison

### Network Configuration
- Custom IP ranges and subnets
- Configurable RIC and gNB addresses
- Dynamic UE IP assignment

### Advanced Features
- Debug logging
- Performance monitoring
- xApp deployment
- Custom metrics collection

## 🛠️ Management Scripts

```bash
# Deploy the stack
./deploy.sh

# Stop all components
./stop.sh

# Restart with existing configuration
./restart.sh

# Clean up deployment
./clean.sh

# Run tests
./scripts/run_tests.sh

# Monitor UEs
./configs/monitor_ues.sh

# Test UE connectivity
./configs/test_ue_connectivity.sh
```

## 📊 Monitoring

### Prometheus Metrics
- System metrics (CPU, memory, disk)
- E2 interface metrics
- RIC and gNB performance
- UE connection statistics

### Grafana Dashboards
- Real-time system overview
- E2 message flow visualization
- Performance trending
- Alert management

### Log Aggregation
- Centralized logging
- Component-specific logs
- Real-time log streaming
- Historical log analysis

## 🧪 Testing

### Automated Test Suite
- **Component Health**: Verify all components are running
- **Network Connectivity**: Test UE to core connectivity
- **E2 Interface**: Validate E2 setup and messages
- **Performance**: System resource utilization
- **Integration**: End-to-end functionality

### Manual Testing
- UE ping tests
- Data plane validation
- xApp functionality
- Monitoring verification

## 🔍 Troubleshooting

### Common Issues
- Docker permission errors
- Port conflicts
- Network namespace issues
- E2 connection problems

### Debug Tools
- Comprehensive logging
- Network packet capture
- System resource monitoring
- Component health checks

## 📖 Documentation

- **[Complete Guide](docs/README.md)**: Detailed deployment and usage guide
- **[API Reference](docs/API_REFERENCE.md)**: REST API and SDK documentation
- **Configuration Examples**: Sample configurations for different scenarios
- **Troubleshooting Guide**: Common issues and solutions

## 🎯 Use Cases

### Development & Testing
- OpenRAN application development
- E2 interface testing
- xApp prototyping
- Performance benchmarking

### Education & Research
- 5G network learning
- RAN intelligence research
- Protocol analysis
- Algorithm development

### Pre-Production Testing
- Deployment validation
- Configuration testing
- Performance analysis
- Integration testing

## 🔧 Requirements

### System Requirements
- Ubuntu 22.04 LTS (recommended)
- 8GB+ RAM (16GB recommended)
- 4+ CPU cores (8+ recommended)
- 20GB+ free disk space

### Software Dependencies
- Docker & Docker Compose
- Git
- Python 3.8+
- Build tools (GCC, CMake)

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Implement your changes
4. Add tests
5. Update documentation
6. Submit a pull request

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 🆘 Support

- **Issues**: Report bugs and feature requests on GitHub
- **Documentation**: Check the comprehensive guides in `docs/`
- **Testing**: Run the diagnostic suite with `./scripts/run_tests.sh`
- **Monitoring**: Check system status with built-in monitoring

---

*🚀 Ready to deploy your OpenRAN stack? Run `./deploy.sh` to get started!*