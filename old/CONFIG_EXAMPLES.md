# RIC Interactive Deployment Configuration Examples

This file contains example configurations for different RIC deployment scenarios.

## Example 1: ORAN SC RIC with Existing gNB and Core

```json
{
  "ric_type": "1",
  "ric_ip": "192.168.1.100",
  "ric_port": "36421",
  "gnb_ip": "192.168.1.50",
  "gnb_port": "36422",
  "core_ip": "192.168.1.10",
  "core_type": "2",
  "install_dir": "/home/user/ric-deployment",
  "config_dir": "/home/user/ric-deployment/configs",
  "use_existing_core": true,
  "use_existing_gnb": true,
  "e2_interface_config": {
    "plmn_id": "00101"
  },
  "deployment_options": {
    "use_docker": true,
    "service_models": {
      "kpm": true,
      "rc": true
    }
  }
}
```

## Example 2: FlexRIC with Simulated Environment

```json
{
  "ric_type": "2",
  "ric_ip": "127.0.0.1",
  "ric_port": "36421",
  "gnb_ip": "127.0.0.1",
  "gnb_port": "36422",
  "core_ip": "127.0.0.1",
  "core_type": "1",
  "install_dir": "/home/user/flexric-deployment",
  "config_dir": "/home/user/flexric-deployment/configs",
  "use_existing_core": false,
  "use_existing_gnb": false,
  "e2_interface_config": {},
  "deployment_options": {
    "asn1c_path": "/usr/local/bin/asn1c",
    "e2ap_version": "E2AP_V2",
    "kmp_version": "KMP_V2_03",
    "service_models": {
      "kmp": true,
      "rc": true
    }
  }
}
```

## Example 3: OAI E2 Agent with RF Simulator

```json
{
  "ric_type": "3",
  "ric_ip": "127.0.0.1",
  "ric_port": "36421",
  "gnb_ip": "127.0.0.1",
  "gnb_port": "36422",
  "core_ip": "127.0.0.1",
  "core_type": "2",
  "install_dir": "/home/user/oai-e2-deployment",
  "config_dir": "/home/user/oai-e2-deployment/configs",
  "use_existing_core": false,
  "use_existing_gnb": false,
  "e2_interface_config": {},
  "deployment_options": {
    "build_gnb": true,
    "build_nrue": true,
    "use_rfsim": true
  }
}
```

## Configuration Parameters

### Required Parameters

- `ric_type`: RIC implementation type (1=ORAN SC, 2=FlexRIC, 3=OAI E2, 4=Custom)
- `ric_ip`: IP address where RIC will be deployed
- `ric_port`: Port for E2 interface communication
- `gnb_ip`: IP address of the gNB (existing or to be deployed)
- `gnb_port`: Port for gNB E2 interface
- `core_ip`: IP address of the Core network
- `core_type`: Core network type (1=Open5GS, 2=OAI CN5G, 3=Free5GC, 4=External)
- `install_dir`: Directory where RIC will be installed
- `config_dir`: Directory for configuration files

### Optional Parameters

- `use_existing_core`: Whether to connect to existing Core (true/false)
- `use_existing_gnb`: Whether to connect to existing gNB (true/false)
- `e2_interface_config`: E2 interface specific configuration
- `deployment_options`: RIC-specific deployment options

### E2 Interface Configuration

- `plmn_id`: PLMN identifier for the network
- Additional parameters depend on RIC type

### Deployment Options

#### ORAN SC RIC Options
- `use_docker`: Use Docker deployment (true/false)
- `service_models`: Service models to enable (kpm, rc)

#### FlexRIC Options
- `asn1c_path`: Path to ASN1C compiler
- `e2ap_version`: E2AP version to use
- `kmp_version`: KMP version to use

#### OAI E2 Agent Options
- `build_gnb`: Build gNB component (true/false)
- `build_nrue`: Build nrUE component (true/false)
- `use_rfsim`: Use RF simulator (true/false)

## Network Configuration Guidelines

### IP Address Planning
- RIC: Typically uses management network IP
- gNB: Can be same as RIC for local deployment or different for remote gNB
- Core: Usually separate network segment for production

### Port Configuration
- Standard E2 port: 36421
- gNB E2 port: 36422
- Avoid conflicts with existing services

### Security Considerations
- Use appropriate firewall rules
- Consider VPN for remote deployments
- Validate certificate configurations for production