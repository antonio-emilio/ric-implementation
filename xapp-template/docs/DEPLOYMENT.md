# Smart Monitor xApp - Detailed Deployment Guide

This document provides comprehensive deployment instructions for the Smart Monitor xApp in various environments and scenarios.

## 📋 Table of Contents

1. [Prerequisites](#prerequisites)
2. [Environment Setup](#environment-setup)
3. [Deployment Scenarios](#deployment-scenarios)
4. [Configuration](#configuration)
5. [Integration](#integration)
6. [Monitoring](#monitoring)
7. [Troubleshooting](#troubleshooting)
8. [Performance Tuning](#performance-tuning)
9. [Security Considerations](#security-considerations)

## 🔧 Prerequisites

### System Requirements

- **Operating System**: Ubuntu 22.04 LTS (recommended)
- **CPU**: Minimum 2 cores, 4 cores recommended
- **Memory**: Minimum 4GB RAM, 8GB recommended
- **Storage**: 10GB free space
- **Network**: Internet connectivity for dependencies

### Software Dependencies

```bash
# Core dependencies
sudo apt update
sudo apt install -y \
    build-essential \
    cmake \
    git \
    gcc-13 \
    g++-13 \
    libsqlite3-dev \
    libjson-c-dev \
    libcurl4-openssl-dev \
    libsctp-dev \
    libpcre2-dev \
    python3.10-dev \
    ninja-build

# Set GCC 13 as default
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100 \
  --slave /usr/bin/g++ g++ /usr/bin/g++-13
sudo update-alternatives --set gcc /usr/bin/gcc-13
```

### FlexRIC Installation

The xApp requires FlexRIC to be installed. Follow the main repository instructions:

```bash
# Option 1: Use the setup script
./setup_flexric.sh

# Option 2: Use the full deployment script
./deploy_flexric_oai_e2.sh

# Option 3: Manual installation (see FlexRIC documentation)
```

## 🏗️ Environment Setup

### 1. Clone and Build

```bash
# Clone the repository (if not already done)
git clone <repository-url>
cd ric-implementation/xapp-template

# Build the xApp
chmod +x scripts/build.sh
./scripts/build.sh
```

### 2. Verify Installation

```bash
# Check if executable was created
ls -la build/smart_monitor_xapp

# Run basic tests
./scripts/run_tests.sh
```

## 🚀 Deployment Scenarios

### Scenario 1: Development Environment

For development and testing with emulated E2 nodes:

```bash
# 1. Start FlexRIC nearRT-RIC
cd ~/flexric/build
./examples/ric/nearRT-RIC &

# 2. Start emulated E2 agent
./examples/emulator/agent/emu_agent_gnb &

# 3. Deploy and run xApp
cd /path/to/xapp-template
./scripts/deploy.sh
XAPP_DURATION=60 ./build/smart_monitor_xapp
```

### Scenario 2: Integration with OAI

For integration with OpenAirInterface RAN:

```bash
# 1. Ensure OAI 5G Core is running
# (Follow main repository setup_core.sh instructions)

# 2. Run the integrated deployment
./deploy_flexric_oai_e2.sh

# 3. Add our xApp to the deployment
cd xapp-template
XAPP_DURATION=30 ./build/smart_monitor_xapp &
```

### Scenario 3: Production Environment

For production deployment with real base stations:

```bash
# 1. Configure nearRT-RIC for production
# Edit /usr/local/etc/flexric/flexric.conf

# 2. Start nearRT-RIC
systemctl start flexric-nearrt-ric  # If service is configured
# OR
/usr/local/bin/nearRT-RIC &

# 3. Deploy xApp with monitoring
cd xapp-template
nohup ./build/smart_monitor_xapp > /var/log/smart_monitor_xapp.log 2>&1 &

# 4. Set up log rotation
sudo tee /etc/logrotate.d/smart_monitor_xapp << EOF
/var/log/smart_monitor_xapp.log {
    daily
    rotate 30
    compress
    missingok
    notifempty
    create 644 root root
}
EOF
```

### Scenario 4: Docker Deployment

Create a containerized deployment:

```bash
# Create Dockerfile
cat > Dockerfile << EOF
FROM ubuntu:22.04

RUN apt update && apt install -y \\
    libsqlite3-0 \\
    libjson-c5 \\
    libcurl4

COPY build/smart_monitor_xapp /usr/local/bin/
COPY config/ /etc/smart_monitor_xapp/

CMD ["/usr/local/bin/smart_monitor_xapp"]
EOF

# Build container
docker build -t smart_monitor_xapp .

# Run container
docker run -d --name smart_monitor_xapp \\
    -v /tmp:/tmp \\
    -e XAPP_DURATION=0 \\
    smart_monitor_xapp
```

## ⚙️ Configuration

### Main Configuration File

Edit `config/xapp_config.json`:

```json
{
  "xapp_name": "Smart Monitor xApp",
  "version": "1.0.0",
  "monitoring_interval": 1000,
  "database_path": "/var/lib/smart_monitor_xapp/data.db",
  "log_level": "INFO",
  "ric_ip": "10.0.0.1",
  "ric_port": 36421,
  "metrics": {
    "kmp_enabled": true,
    "rc_enabled": true,
    "mac_enabled": false,
    "rlc_enabled": false,
    "pdcp_enabled": false,
    "gtp_enabled": false
  },
  "analytics": {
    "anomaly_detection": true,
    "trend_analysis": true,
    "recommendations": true,
    "alert_threshold": 0.8
  }
}
```

### Threshold Configuration

Edit `config/thresholds.json` for your environment:

```json
{
  "thresholds": {
    "Throughput": {
      "min": 50,
      "max": 2000,
      "warning": 200,
      "critical": 100,
      "enabled": true
    },
    "Latency": {
      "min": 0,
      "max": 100,
      "warning": 20,
      "critical": 50,
      "enabled": true
    }
  }
}
```

### Environment Variables

Control xApp behavior with environment variables:

```bash
# Runtime duration (0 = infinite)
export XAPP_DURATION=3600

# Debug mode
export XAPP_DEBUG=1

# Custom config path
export XAPP_CONFIG_PATH="/etc/smart_monitor_xapp/config.json"

# Custom database path
export XAPP_DATABASE_PATH="/var/lib/smart_monitor_xapp/metrics.db"
```

## 🔗 Integration

### Integration with Existing Scripts

Add to `setup_flexric.sh`:

```bash
# Add after starting other xApps
echo "📊 Starting Smart Monitor xApp..."
cd /path/to/xapp-template
XAPP_DURATION=20 ./build/smart_monitor_xapp &
```

Add to `deploy_flexric_oai_e2.sh`:

```bash
# Add after starting existing xApps
echo "📊 Starting Smart Monitor xApp..."
cd /path/to/xapp-template
XAPP_DURATION=30 ./build/smart_monitor_xapp &
```

### REST API Integration

The xApp can be extended with REST API endpoints:

```bash
# Query recent metrics
curl http://localhost:8080/api/metrics?type=throughput&limit=10

# Get anomalies
curl http://localhost:8080/api/anomalies?severity=critical

# Get recommendations
curl http://localhost:8080/api/recommendations?node_id=1
```

### Database Integration

Access metrics programmatically:

```bash
# Connect to database
sqlite3 /tmp/xapp_data.db

# Query recent metrics
SELECT * FROM metrics ORDER BY timestamp DESC LIMIT 10;

# Get anomalies by severity
SELECT * FROM anomalies WHERE severity = 2 ORDER BY detected_at DESC;

# Get recommendation statistics
SELECT type, COUNT(*) as count FROM recommendations GROUP BY type;
```

## 📊 Monitoring

### Real-time Monitoring

```bash
# Monitor xApp process
watch -n 1 'ps aux | grep smart_monitor_xapp'

# Monitor log output
tail -f /tmp/smart_monitor_xapp.log

# Monitor database growth
watch -n 5 'ls -lh /tmp/xapp_data.db'

# Monitor system resources
htop -p $(pgrep smart_monitor_xapp)
```

### Performance Metrics

```bash
# Check xApp statistics
grep "Statistics" /tmp/smart_monitor_xapp.log | tail -5

# Database performance
sqlite3 /tmp/xapp_data.db "
SELECT 
    'metrics' as table_name, COUNT(*) as count FROM metrics
UNION ALL
SELECT 
    'anomalies', COUNT(*) FROM anomalies
UNION ALL
SELECT 
    'recommendations', COUNT(*) FROM recommendations;
"
```

### Health Checks

Create a health check script:

```bash
#!/bin/bash
# health_check.sh

XAPP_PID=$(pgrep smart_monitor_xapp)
if [ -z "$XAPP_PID" ]; then
    echo "ERROR: xApp not running"
    exit 1
fi

# Check if database is accessible
if ! sqlite3 /tmp/xapp_data.db "SELECT 1;" >/dev/null 2>&1; then
    echo "ERROR: Database not accessible"
    exit 1
fi

# Check log for recent activity
LAST_LOG=$(tail -1 /tmp/smart_monitor_xapp.log | cut -d' ' -f1-2)
CURRENT_TIME=$(date "+%Y-%m-%d %H:%M")

echo "OK: xApp running (PID: $XAPP_PID)"
echo "Last activity: $LAST_LOG"
exit 0
```

## 🐛 Troubleshooting

### Common Issues

#### 1. Build Failures

```bash
# Check dependencies
pkg-config --exists sqlite3 json-c libcurl

# Verify GCC version
gcc --version  # Should be 13.x

# Clean and rebuild
rm -rf build
./scripts/build.sh
```

#### 2. Runtime Errors

```bash
# Check nearRT-RIC status
pgrep -f nearRT-RIC

# Check network connectivity
netstat -an | grep 36421

# Check permissions
ls -la /tmp/xapp_data.db
```

#### 3. Performance Issues

```bash
# Check system resources
free -h
df -h
iostat 1 5

# Monitor database locks
sqlite3 /tmp/xapp_data.db "PRAGMA busy_timeout;"

# Check log for errors
grep ERROR /tmp/smart_monitor_xapp.log
```

### Debug Mode

Enable detailed debugging:

```bash
# Compile with debug symbols
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# Run with debugging
export XAPP_DEBUG=1
gdb ./smart_monitor_xapp
```

### Log Analysis

```bash
# Error analysis
grep -E "(ERROR|CRITICAL)" /tmp/smart_monitor_xapp.log

# Performance analysis
grep "Statistics" /tmp/smart_monitor_xapp.log | \
  awk '{print $3, $NF}' | \
  tail -20

# Anomaly trends
grep "Anomaly detected" /tmp/smart_monitor_xapp.log | \
  cut -d' ' -f1-2 | \
  uniq -c
```

## 🚀 Performance Tuning

### Database Optimization

```sql
-- Run in SQLite
PRAGMA journal_mode=WAL;
PRAGMA synchronous=NORMAL;
PRAGMA cache_size=10000;
PRAGMA temp_store=memory;
```

### System Tuning

```bash
# Increase file descriptors
ulimit -n 4096

# Set CPU affinity
taskset -c 0,1 ./build/smart_monitor_xapp

# Adjust nice priority
nice -n -10 ./build/smart_monitor_xapp
```

### Memory Optimization

```bash
# Monitor memory usage
valgrind --tool=massif ./build/smart_monitor_xapp

# Reduce database retention
sqlite3 /tmp/xapp_data.db "
DELETE FROM metrics WHERE timestamp < strftime('%s', 'now', '-7 days');
VACUUM;
"
```

## 🔒 Security Considerations

### File Permissions

```bash
# Secure configuration files
chmod 600 config/*.json
chown app:app config/*.json

# Secure database
chmod 600 /tmp/xapp_data.db
chown app:app /tmp/xapp_data.db

# Secure logs
chmod 640 /tmp/smart_monitor_xapp.log
chown app:adm /tmp/smart_monitor_xapp.log
```

### Network Security

```bash
# Firewall rules
ufw allow from 10.0.0.0/8 to any port 36421
ufw deny 36421

# SSL/TLS configuration (if applicable)
# Configure in FlexRIC nearRT-RIC
```

### Process Security

```bash
# Run as non-root user
useradd -r -s /bin/false xapp
sudo -u xapp ./build/smart_monitor_xapp

# Use systemd service
sudo tee /etc/systemd/system/smart_monitor_xapp.service << EOF
[Unit]
Description=Smart Monitor xApp
After=network.target

[Service]
Type=simple
User=xapp
ExecStart=/opt/smart_monitor_xapp/build/smart_monitor_xapp
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
EOF

sudo systemctl enable smart_monitor_xapp
sudo systemctl start smart_monitor_xapp
```

## 📞 Support

For additional support:

1. **Check logs**: `/tmp/smart_monitor_xapp.log`
2. **Review documentation**: `docs/` folder
3. **Run diagnostics**: `./scripts/run_tests.sh`
4. **Database inspection**: Use SQLite tools
5. **Community support**: Create repository issues

## 📚 Additional Resources

- [FlexRIC Documentation](https://gitlab.eurecom.fr/mosaic5g/flexric)
- [O-RAN Specifications](https://www.o-ran.org/specifications)
- [SQLite Documentation](https://www.sqlite.org/docs.html)
- [JSON-C Documentation](https://json-c.github.io/json-c/)