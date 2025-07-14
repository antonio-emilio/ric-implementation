# Smart Monitor xApp - API Documentation

This document describes the APIs and interfaces provided by the Smart Monitor xApp.

## 📋 Table of Contents

1. [Overview](#overview)
2. [Database API](#database-api)
3. [Analytics API](#analytics-api)
4. [Configuration API](#configuration-api)
5. [Monitoring API](#monitoring-api)
6. [REST API Extensions](#rest-api-extensions)
7. [Event System](#event-system)
8. [Data Structures](#data-structures)

## 🌟 Overview

The Smart Monitor xApp provides multiple APIs for:

- **Database Operations**: Store and retrieve metrics, anomalies, and recommendations
- **Analytics**: Real-time analysis and anomaly detection
- **Configuration**: Runtime configuration management
- **Monitoring**: Performance and health monitoring
- **Events**: Event logging and notification system

## 💾 Database API

### Connection Management

```c
// Initialize database context
database_context_t* database_init(const char* database_path);

// Cleanup database context
void database_cleanup(database_context_t* ctx);

// Connect to database
int database_connect(database_context_t* ctx);

// Disconnect from database
void database_disconnect(database_context_t* ctx);
```

### Schema Management

```c
// Create database schema
int database_create_schema(database_context_t* ctx);

// Check schema version
int database_check_schema_version(database_context_t* ctx);

// Upgrade schema
int database_upgrade_schema(database_context_t* ctx, int current_version, int target_version);
```

### Metric Operations

```c
// Insert single metric
int database_insert_metric(database_context_t* ctx, const metric_data_t* metric);

// Insert multiple metrics in batch
int database_insert_metrics_batch(database_context_t* ctx, const metric_data_t* metrics, int count);

// Query metrics by type and node
metric_query_result_t* database_query_metrics(
    database_context_t* ctx, 
    metric_type_t type, 
    uint32_t node_id, 
    time_t start_time, 
    time_t end_time
);

// Query recent metrics
metric_query_result_t* database_query_recent_metrics(
    database_context_t* ctx, 
    metric_type_t type, 
    int limit
);
```

### Anomaly Operations

```c
// Insert anomaly
int database_insert_anomaly(database_context_t* ctx, const anomaly_result_t* anomaly);

// Query anomalies by severity
anomaly_query_result_t* database_query_anomalies(
    database_context_t* ctx, 
    anomaly_severity_t severity, 
    time_t start_time, 
    time_t end_time
);

// Query recent anomalies
anomaly_query_result_t* database_query_recent_anomalies(database_context_t* ctx, int limit);
```

### Event Logging

```c
// Log event
int database_log_event(
    database_context_t* ctx, 
    event_type_t type, 
    uint32_t node_id, 
    uint32_t subscription_id, 
    const char* message, 
    const char* details
);

// Query events by type
event_query_result_t* database_query_events(
    database_context_t* ctx, 
    event_type_t type, 
    time_t start_time, 
    time_t end_time
);
```

### Usage Example

```c
// Initialize database
database_context_t* db = database_init("/tmp/xapp_data.db");

// Create and insert metric
metric_data_t metric = {
    .type = METRIC_THROUGHPUT,
    .value = 150.5,
    .node_id = 1,
    .cell_id = 1,
    .timestamp = time(NULL)
};
database_insert_metric(db, &metric);

// Query recent metrics
metric_query_result_t* results = database_query_recent_metrics(db, METRIC_THROUGHPUT, 10);
for (int i = 0; i < results->count; i++) {
    printf("Metric: %.2f at %ld\n", results->metrics[i].value, results->metrics[i].timestamp);
}

// Cleanup
database_free_metric_result(results);
database_cleanup(db);
```

## 📊 Analytics API

### Context Management

```c
// Initialize analytics context
analytics_context_t* analytics_init(const char* config_file);

// Cleanup analytics context
void analytics_cleanup(analytics_context_t* ctx);

// Load configuration
int analytics_load_config(analytics_context_t* ctx, const char* config_file);
```

### Metric Processing

```c
// Process single metric
int analytics_process_metric(analytics_context_t* ctx, const metric_data_t* metric);

// Add metric (convenience function)
int analytics_add_metric(
    analytics_context_t* ctx, 
    metric_type_t type, 
    double value, 
    uint32_t node_id, 
    uint32_t cell_id
);
```

### Statistical Analysis

```c
// Calculate statistics
stats_result_t analytics_calculate_stats(const metric_data_t* data, int count);

// Calculate trend
trend_result_t analytics_calculate_trend(const metric_data_t* data, int count);

// Calculate Z-score
double analytics_calculate_z_score(double value, double mean, double std_dev);

// Check if outlier
bool analytics_is_outlier(double z_score, double threshold);
```

### Anomaly Detection

```c
// Detect anomaly using multiple algorithms
anomaly_result_t analytics_detect_anomaly(analytics_context_t* ctx, const metric_data_t* metric);

// Threshold-based detection
anomaly_result_t analytics_threshold_detection(analytics_context_t* ctx, const metric_data_t* metric);

// Statistical detection
anomaly_result_t analytics_statistical_detection(analytics_context_t* ctx, const metric_data_t* metric);

// ML-based detection
anomaly_result_t analytics_ml_detection(analytics_context_t* ctx, const metric_data_t* metric);
```

### Recommendation Generation

```c
// Generate recommendation based on anomaly
recommendation_result_t analytics_generate_recommendation(
    analytics_context_t* ctx, 
    const metric_data_t* metric, 
    const anomaly_result_t* anomaly
);

// Performance-based recommendations
recommendation_result_t analytics_performance_recommendation(
    analytics_context_t* ctx, 
    const metric_data_t* metric
);
```

### Data Access

```c
// Get metric history
metric_history_t* analytics_get_history(analytics_context_t* ctx, metric_type_t type);

// Get recent anomalies
anomaly_result_t* analytics_get_recent_anomalies(analytics_context_t* ctx, int* count);

// Get recent recommendations
recommendation_result_t* analytics_get_recent_recommendations(analytics_context_t* ctx, int* count);
```

### Usage Example

```c
// Initialize analytics
analytics_context_t* analytics = analytics_init("config/thresholds.json");

// Process metrics
for (int i = 0; i < 100; i++) {
    analytics_add_metric(analytics, METRIC_LATENCY, 10.0 + i * 0.5, 1, 1);
}

// Check for anomalies
int anomaly_count;
anomaly_result_t* anomalies = analytics_get_recent_anomalies(analytics, &anomaly_count);

for (int i = 0; i < anomaly_count; i++) {
    if (anomalies[i].severity >= ANOMALY_WARNING) {
        printf("Anomaly: %s\n", anomalies[i].description);
    }
}

// Cleanup
analytics_cleanup(analytics);
```

## ⚙️ Configuration API

### Configuration Loading

```c
// Load main configuration
int load_config(xapp_context_t* ctx);

// Load threshold configuration
int load_thresholds(xapp_context_t* ctx);

// Print configuration
void print_config(const xapp_config_t* config);
```

### JSON Configuration Access

```c
// JSON utilities
bool utils_json_get_string(json_object* obj, const char* key, char* value, size_t value_size);
bool utils_json_get_int(json_object* obj, const char* key, int* value);
bool utils_json_get_double(json_object* obj, const char* key, double* value);
bool utils_json_get_bool(json_object* obj, const char* key, bool* value);

// Load/save JSON files
json_object* utils_json_load_file(const char* filepath);
int utils_json_save_file(json_object* obj, const char* filepath);
```

### Configuration Structure

```c
typedef struct {
    char xapp_name[256];
    char version[32];
    int monitoring_interval;
    char database_path[512];
    char log_level[16];
    char ric_ip[64];
    int ric_port;
    
    // Metrics configuration
    bool kmp_enabled;
    bool rc_enabled;
    bool mac_enabled;
    bool rlc_enabled;
    bool pdcp_enabled;
    bool gtp_enabled;
    
    // Analytics configuration
    bool anomaly_detection;
    bool trend_analysis;
    bool recommendations;
    double alert_threshold;
} xapp_config_t;
```

## 📈 Monitoring API

### Performance Monitoring

```c
// Print statistics
void print_statistics(const xapp_context_t* ctx);

// Generate report
void generate_report(const xapp_context_t* ctx);

// Database performance
void database_print_performance(const database_context_t* ctx);

// Analytics performance
void analytics_print_performance(const analytics_context_t* ctx);
```

### System Utilities

```c
// System information
int utils_get_cpu_count(void);
double utils_get_cpu_usage(void);
size_t utils_get_memory_usage(void);
char* utils_get_hostname(char* buffer, size_t buffer_size);

// Performance timing
void utils_timer_start(performance_timer_t* timer);
double utils_timer_stop(performance_timer_t* timer);
double utils_timer_elapsed(const performance_timer_t* timer);
```

## 🌐 REST API Extensions

The xApp can be extended with REST API endpoints. Here's how to add them:

### HTTP Server Setup

```c
// Add to main application
#include <microhttpd.h>

// HTTP request handler
int handle_http_request(void* cls, struct MHD_Connection* connection,
                       const char* url, const char* method,
                       const char* version, const char* upload_data,
                       size_t* upload_data_size, void** con_cls);

// Start HTTP server
int start_http_server(xapp_context_t* ctx, int port);
```

### API Endpoints

#### GET /api/metrics

```bash
# Get recent metrics
curl "http://localhost:8080/api/metrics?type=throughput&limit=10"

# Response
{
  "status": "success",
  "data": [
    {
      "type": "throughput",
      "value": 150.5,
      "node_id": 1,
      "cell_id": 1,
      "timestamp": 1640995200
    }
  ]
}
```

#### GET /api/anomalies

```bash
# Get recent anomalies
curl "http://localhost:8080/api/anomalies?severity=critical&limit=5"

# Response
{
  "status": "success",
  "data": [
    {
      "metric_type": "latency",
      "severity": "critical",
      "threshold_value": 100.0,
      "actual_value": 150.0,
      "confidence": 0.95,
      "detected_at": 1640995200,
      "description": "Latency threshold exceeded"
    }
  ]
}
```

#### GET /api/recommendations

```bash
# Get recommendations
curl "http://localhost:8080/api/recommendations?node_id=1"

# Response
{
  "status": "success",
  "data": [
    {
      "type": "increase_power",
      "node_id": 1,
      "cell_id": 1,
      "confidence": 0.8,
      "expected_improvement": 15.0,
      "description": "Increase transmission power to improve throughput",
      "parameters": "power_increase=5dB"
    }
  ]
}
```

#### GET /api/status

```bash
# Get xApp status
curl "http://localhost:8080/api/status"

# Response
{
  "status": "success",
  "data": {
    "state": "running",
    "uptime": 3600,
    "connected_nodes": 2,
    "active_subscriptions": 6,
    "total_indications": 1000,
    "total_anomalies": 5,
    "total_recommendations": 3
  }
}
```

## 📡 Event System

### Event Types

```c
typedef enum {
    EVENT_XAPP_START,
    EVENT_XAPP_STOP,
    EVENT_NODE_CONNECT,
    EVENT_NODE_DISCONNECT,
    EVENT_SUBSCRIPTION_CREATE,
    EVENT_SUBSCRIPTION_DELETE,
    EVENT_INDICATION_RECEIVED,
    EVENT_CONTROL_SENT,
    EVENT_ANOMALY_DETECTED,
    EVENT_RECOMMENDATION_GENERATED,
    EVENT_ERROR
} event_type_t;
```

### Event Handling

```c
// Event structure
typedef struct {
    event_type_t type;
    uint32_t node_id;
    uint32_t subscription_id;
    time_t timestamp;
    char message[512];
    char details[1024];
} event_data_t;

// Event callbacks
typedef void (*event_callback_t)(const event_data_t* event, void* user_data);

// Register event callback
int register_event_callback(event_type_t type, event_callback_t callback, void* user_data);

// Fire event
void fire_event(event_type_t type, uint32_t node_id, uint32_t subscription_id, 
               const char* message, const char* details);
```

## 📋 Data Structures

### Core Structures

```c
// Metric data point
typedef struct {
    metric_type_t type;
    double value;
    uint32_t node_id;
    uint32_t cell_id;
    time_t timestamp;
} metric_data_t;

// Statistical analysis result
typedef struct {
    double mean;
    double variance;
    double std_dev;
    double min;
    double max;
    double median;
    double z_score;
    bool is_outlier;
} stats_result_t;

// Anomaly detection result
typedef struct {
    metric_type_t metric_type;
    anomaly_severity_t severity;
    double threshold_value;
    double actual_value;
    double confidence;
    time_t detected_at;
    char description[256];
} anomaly_result_t;

// Recommendation result
typedef struct {
    recommendation_type_t type;
    uint32_t node_id;
    uint32_t cell_id;
    double confidence;
    double expected_improvement;
    time_t generated_at;
    char description[512];
    char parameters[256];
} recommendation_result_t;
```

### Enumerations

```c
// Metric types
typedef enum {
    METRIC_THROUGHPUT,
    METRIC_LATENCY,
    METRIC_PACKET_LOSS,
    METRIC_CPU_UTILIZATION,
    METRIC_MEMORY_USAGE,
    METRIC_RSRP,
    METRIC_RSRQ,
    METRIC_SINR,
    METRIC_PRB_USAGE,
    METRIC_COUNT
} metric_type_t;

// Anomaly severity levels
typedef enum {
    ANOMALY_NONE,
    ANOMALY_WARNING,
    ANOMALY_CRITICAL
} anomaly_severity_t;

// Recommendation types
typedef enum {
    RECOMMENDATION_NONE,
    RECOMMENDATION_INCREASE_POWER,
    RECOMMENDATION_DECREASE_POWER,
    RECOMMENDATION_HANDOVER,
    RECOMMENDATION_LOAD_BALANCE,
    RECOMMENDATION_RESOURCE_ALLOCATION,
    RECOMMENDATION_PARAMETER_ADJUSTMENT
} recommendation_type_t;
```

## 📚 Usage Examples

### Complete Integration Example

```c
#include "smart_monitor_xapp.h"

int main() {
    // Initialize components
    database_context_t* db = database_init("/tmp/xapp_data.db");
    analytics_context_t* analytics = analytics_init("config/thresholds.json");
    
    // Simulate metric processing
    for (int i = 0; i < 100; i++) {
        // Create metric
        metric_data_t metric = {
            .type = METRIC_THROUGHPUT,
            .value = 100.0 + (rand() % 100),
            .node_id = 1,
            .cell_id = 1,
            .timestamp = time(NULL)
        };
        
        // Store in database
        database_insert_metric(db, &metric);
        
        // Process with analytics
        analytics_process_metric(analytics, &metric);
    }
    
    // Check for anomalies
    int anomaly_count;
    anomaly_result_t* anomalies = analytics_get_recent_anomalies(analytics, &anomaly_count);
    
    for (int i = 0; i < anomaly_count; i++) {
        // Store anomaly
        database_insert_anomaly(db, &anomalies[i]);
        
        // Log event
        database_log_event(db, EVENT_ANOMALY_DETECTED, 0, 0, 
                          "Anomaly detected", anomalies[i].description);
    }
    
    // Generate report
    printf("Processed %d metrics, detected %d anomalies\n", 100, anomaly_count);
    
    // Cleanup
    analytics_cleanup(analytics);
    database_cleanup(db);
    
    return 0;
}
```

This API documentation provides comprehensive coverage of all interfaces available in the Smart Monitor xApp. Use these APIs to integrate the xApp with your RIC platform or extend its functionality.