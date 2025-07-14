# xApp Template for FlexRIC

This template provides a complete implementation of an intelligent monitoring xApp for FlexRIC with advanced features including anomaly detection, resource optimization recommendations, and real-time analytics.

## 📋 Overview

The **Smart Monitor xApp** is a sophisticated monitoring application that:

- **📊 Real-time Monitoring**: Continuously monitors KPM (Key Performance Metrics) and RC (RAN Control) data
- **🔍 Anomaly Detection**: Implements threshold-based anomaly detection for various network metrics
- **🎯 Intelligent Recommendations**: Provides optimization suggestions based on observed patterns
- **📈 Performance Analytics**: Calculates trends and provides statistical analysis
- **🚨 Alert System**: Generates alerts for critical network conditions
- **💾 Data Persistence**: Stores metrics and events for historical analysis

## 🏗️ Architecture

```
xapp-template/
├── src/
│   ├── smart_monitor_xapp.c    # Main xApp implementation
│   ├── analytics.c             # Analytics and anomaly detection
│   ├── database.c              # Data persistence layer
│   └── utils.c                 # Utility functions
├── include/
│   ├── smart_monitor_xapp.h    # Main header file
│   ├── analytics.h             # Analytics definitions
│   ├── database.h              # Database interface
│   └── utils.h                 # Utility definitions
├── config/
│   ├── xapp_config.json        # xApp configuration
│   └── thresholds.json         # Anomaly detection thresholds
├── scripts/
│   ├── build.sh                # Build script
│   ├── deploy.sh               # Deployment script
│   └── run_tests.sh            # Test execution script
├── tests/
│   ├── test_analytics.c        # Analytics tests
│   └── test_database.c         # Database tests
├── docs/
│   ├── API.md                  # API documentation
│   └── DEPLOYMENT.md           # Detailed deployment guide
├── CMakeLists.txt              # CMake build configuration
└── README.md                   # This file
```

## 🚀 Quick Start

### Prerequisites

1. **FlexRIC installed** - Follow the main repository setup:
   ```bash
   ./setup_flexric.sh
   ```

2. **System dependencies**:
   ```bash
   sudo apt update
   sudo apt install -y libsqlite3-dev libjson-c-dev libcurl4-openssl-dev
   ```

### Build and Deploy

1. **Build the xApp**:
   ```bash
   cd xapp-template
   chmod +x scripts/build.sh
   ./scripts/build.sh
   ```

2. **Deploy the xApp**:
   ```bash
   chmod +x scripts/deploy.sh
   ./scripts/deploy.sh
   ```

3. **Run with custom duration**:
   ```bash
   # Run for 60 seconds
   XAPP_DURATION=60 ./build/smart_monitor_xapp
   
   # Run indefinitely
   ./build/smart_monitor_xapp
   ```

## ⚙️ Configuration

### Main Configuration (`config/xapp_config.json`)

```json
{
  "xapp_name": "Smart Monitor xApp",
  "version": "1.0.0",
  "monitoring_interval": 1000,
  "database_path": "/tmp/xapp_data.db",
  "log_level": "INFO",
  "metrics": {
    "kmp_enabled": true,
    "rc_enabled": true,
    "mac_enabled": true,
    "rlc_enabled": true,
    "pdcp_enabled": true,
    "gtp_enabled": true
  },
  "analytics": {
    "anomaly_detection": true,
    "trend_analysis": true,
    "recommendations": true,
    "alert_threshold": 0.8
  }
}
```

### Threshold Configuration (`config/thresholds.json`)

```json
{
  "thresholds": {
    "throughput_mbps": {
      "min": 10,
      "max": 1000,
      "critical": 5
    },
    "latency_ms": {
      "warning": 50,
      "critical": 100
    },
    "packet_loss_percent": {
      "warning": 1.0,
      "critical": 5.0
    },
    "cpu_utilization_percent": {
      "warning": 80,
      "critical": 95
    }
  }
}
```

## 🔧 Advanced Features

### Anomaly Detection

The xApp implements sophisticated anomaly detection algorithms:

- **Statistical Analysis**: Z-score based outlier detection
- **Trend Analysis**: Moving average and trend detection
- **Threshold Monitoring**: Configurable warning and critical thresholds
- **Pattern Recognition**: Identifies recurring patterns and anomalies

### Resource Optimization

Provides intelligent recommendations for:

- **Network Optimization**: Suggests parameter adjustments based on performance
- **Resource Allocation**: Recommends optimal resource distribution
- **Load Balancing**: Identifies load balancing opportunities
- **Capacity Planning**: Predicts future capacity needs

### Real-time Analytics Dashboard

Access real-time metrics through:

- **REST API**: HTTP endpoints for metric queries
- **WebSocket**: Real-time data streaming
- **Database Queries**: SQL interface for historical data
- **Log Analysis**: Structured logging for troubleshooting

## 🔌 Integration with Existing RIC

### FlexRIC Integration

The xApp integrates seamlessly with the existing FlexRIC infrastructure:

```bash
# After running setup_flexric.sh, deploy this xApp
cd /path/to/ric-implementation/xapp-template
./scripts/deploy.sh

# The xApp will automatically connect to the nearRT-RIC
```

### OAI Integration

Works with the OAI setup from the main repository:

```bash
# After running deploy_flexric_oai_e2.sh, you can add this xApp
XAPP_DURATION=30 /path/to/xapp-template/build/smart_monitor_xapp &
```

## 📊 Monitoring and Debugging

### Logs

The xApp provides comprehensive logging:

```bash
# View real-time logs
tail -f /tmp/smart_monitor_xapp.log

# View specific log levels
grep "ERROR\|CRITICAL" /tmp/smart_monitor_xapp.log
```

### Metrics Database

Access stored metrics:

```bash
# Connect to metrics database
sqlite3 /tmp/xapp_data.db

# Query recent metrics
SELECT * FROM metrics ORDER BY timestamp DESC LIMIT 10;

# View anomalies
SELECT * FROM anomalies WHERE severity = 'CRITICAL';
```

### Performance Monitoring

Monitor xApp performance:

```bash
# Check xApp process
ps aux | grep smart_monitor_xapp

# Monitor resource usage
top -p $(pgrep smart_monitor_xapp)

# Check network connections
netstat -an | grep :8080
```

## 🧪 Testing

### Unit Tests

```bash
# Run all tests
./scripts/run_tests.sh

# Run specific test
./build/tests/test_analytics

# Run with verbose output
./build/tests/test_analytics -v
```

### Integration Tests

```bash
# Test with FlexRIC
cd ~/flexric
./build/examples/ric/nearRT-RIC &
sleep 2
cd /path/to/xapp-template
XAPP_DURATION=10 ./build/smart_monitor_xapp
```

## 📈 Performance Optimization

### Tuning Parameters

Optimize performance by adjusting:

- **Monitoring Interval**: Balance between responsiveness and resource usage
- **Buffer Sizes**: Optimize memory usage for high-throughput scenarios
- **Database Settings**: Configure SQLite for optimal performance
- **Threading**: Adjust thread pool sizes for concurrent processing

### Scaling Considerations

For production deployment:

- **Horizontal Scaling**: Deploy multiple instances for load distribution
- **Vertical Scaling**: Increase resources for single instance
- **Database Optimization**: Use dedicated database for high-volume scenarios
- **Caching**: Implement Redis or similar for frequently accessed data

## 🛠️ Development Guide

### Adding New Metrics

1. **Define metric structure** in `include/analytics.h`
2. **Implement collection logic** in `src/analytics.c`
3. **Add database schema** in `src/database.c`
4. **Update configuration** in `config/xapp_config.json`
5. **Add tests** in `tests/test_analytics.c`

### Custom Anomaly Detection

1. **Implement algorithm** in `src/analytics.c`
2. **Add configuration parameters** in `config/thresholds.json`
3. **Update documentation** in `docs/API.md`
4. **Add unit tests** for new algorithms

### API Extensions

1. **Define endpoints** in `src/smart_monitor_xapp.c`
2. **Implement handlers** for new functionality
3. **Update API documentation** in `docs/API.md`
4. **Add integration tests** for new endpoints

## 🔧 Troubleshooting

### Common Issues

1. **Build Failures**:
   - Check FlexRIC installation
   - Verify system dependencies
   - Check compiler version (GCC 13 required)

2. **Runtime Errors**:
   - Verify nearRT-RIC is running
   - Check network connectivity
   - Ensure proper permissions for log/database files

3. **Performance Issues**:
   - Monitor resource usage
   - Adjust monitoring intervals
   - Optimize database queries

### Debug Mode

Enable debug mode for detailed logging:

```bash
export XAPP_DEBUG=1
./build/smart_monitor_xapp
```

## 📚 Additional Resources

- **FlexRIC Documentation**: [https://gitlab.eurecom.fr/mosaic5g/flexric](https://gitlab.eurecom.fr/mosaic5g/flexric)
- **OAI Documentation**: [https://gitlab.eurecom.fr/oai/openairinterface5g](https://gitlab.eurecom.fr/oai/openairinterface5g)
- **E2AP Specification**: [O-RAN E2AP v2.0](https://www.o-ran.org/specifications)
- **Service Models**: [O-RAN Service Models](https://www.o-ran.org/specifications)

## 🤝 Contributing

1. **Fork the repository**
2. **Create a feature branch**
3. **Implement your changes**
4. **Add tests** for new functionality
5. **Update documentation**
6. **Submit a pull request**

## 📄 License

This xApp template is provided under the same license as the main repository.

## 📞 Support

For support and questions:

- **Issues**: Create an issue in the main repository
- **Documentation**: Check the `docs/` folder for detailed guides
- **Examples**: Review the test cases for usage examples