# xApp Template Integration with FlexRIC + OAI

This document shows how to integrate the Smart Monitor xApp template with the existing FlexRIC and OAI scripts in the main repository.

## 🔧 Quick Integration

### 1. Add to setup_flexric.sh

Add this section after the existing xApps are started:

```bash
echo "📊 Starting Smart Monitor xApp..."
cd xapp-template
if [ ! -f "build/smart_monitor_xapp_simple" ]; then
    echo "Building Smart Monitor xApp..."
    ./scripts/build.sh
fi
XAPP_DURATION=20 ./build/smart_monitor_xapp_simple &
cd ..
```

### 2. Add to deploy_flexric_oai_e2.sh

Add this section after the existing xApp monitoring commands:

```bash
echo "📊 Starting Smart Monitor xApp with advanced analytics..."
cd xapp-template
if [ ! -f "build/smart_monitor_xapp_simple" ]; then
    echo "Building Smart Monitor xApp..."
    ./scripts/build.sh
fi
XAPP_DURATION=30 ./build/smart_monitor_xapp_simple &
cd ..
```

### 3. Add to integrar_oai_flexric.sh

Add this section with the other xApps:

```bash
echo "📊 Starting Smart Monitor xApp..."
cd xapp-template
XAPP_DURATION=30 ./build/smart_monitor_xapp_simple &
cd ..
```

## 🚀 Standalone Usage

The xApp can also be used independently:

```bash
cd xapp-template

# Build (first time only)
./scripts/build.sh

# Deploy and run
./scripts/deploy.sh
XAPP_DURATION=60 ./build/smart_monitor_xapp_simple

# Run tests
./scripts/run_tests.sh
```

## 📊 Features Overview

### Real-time Monitoring
- KPM, RC, MAC, RLC, PDCP, GTP metrics
- Simulated realistic data patterns in standalone mode
- Integration with real FlexRIC data when available

### Advanced Analytics
- Statistical analysis (mean, std dev, trends)
- Anomaly detection (threshold, statistical, ML-based)
- Intelligent recommendations for optimization
- Performance pattern recognition

### Data Persistence
- SQLite database for metrics storage
- Event logging and audit trail
- Historical data analysis capabilities
- Automatic data cleanup and optimization

### Deployment Flexibility
- Simplified mode (works without FlexRIC)
- Full FlexRIC integration mode
- Configurable monitoring intervals
- Multiple deployment scenarios

## 📝 Configuration

The xApp can be configured via JSON files:

- `config/xapp_config.json` - Main configuration
- `config/thresholds.json` - Anomaly detection thresholds

Key environment variables:
- `XAPP_DURATION` - Runtime duration in seconds (0 = infinite)
- `XAPP_DEBUG` - Enable debug logging
- `XAPP_CONFIG_PATH` - Custom config file path

## 🔍 Monitoring

### Real-time Monitoring
```bash
# View logs
tail -f /tmp/smart_monitor_xapp.log

# Check database
sqlite3 /tmp/xapp_data.db "SELECT * FROM metrics LIMIT 5;"

# Process monitoring
ps aux | grep smart_monitor_xapp
```

### Performance Metrics
The xApp provides detailed statistics every 10 seconds including:
- Total processed indications
- Detected anomalies count
- Generated recommendations
- Database performance metrics
- System resource usage

## 🧪 Testing

### Unit Tests
```bash
cd xapp-template
./scripts/run_tests.sh
```

### Integration Tests
```bash
# Test with existing FlexRIC setup
./setup_flexric.sh
# xApp will be automatically included

# Test standalone
cd xapp-template
XAPP_DURATION=10 ./build/smart_monitor_xapp_simple
```

## 🎯 Use Cases

### Development Environment
Perfect for testing and development with simulated data patterns that represent realistic 5G network behavior.

### Production Monitoring
Provides comprehensive monitoring and analytics for live 5G networks with intelligent alerting and optimization recommendations.

### Research and Analysis
Historical data storage and advanced analytics enable network performance research and optimization studies.

### Integration Testing
Can be used to validate RIC platform functionality and E2AP interface implementations.

## 📚 Documentation

- `xapp-template/README.md` - Complete feature documentation
- `xapp-template/docs/DEPLOYMENT.md` - Detailed deployment guide
- `xapp-template/docs/API.md` - Programming interface documentation

## 🔧 Customization

The template can be easily extended for specific use cases:

1. **Add new metrics** - Extend the analytics module
2. **Custom algorithms** - Implement new anomaly detection methods
3. **External integrations** - Add REST API endpoints
4. **Custom dashboards** - Integrate with monitoring tools

This Smart Monitor xApp template provides a production-ready foundation for developing sophisticated xApps with advanced monitoring, analytics, and optimization capabilities.