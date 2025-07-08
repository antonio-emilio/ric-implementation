# OpenRAN API Reference

## 🔌 REST API Endpoints

### RIC Management API

#### Get RIC Status
```http
GET /api/v1/ric/status
```

**Response:**
```json
{
  "status": "active",
  "e2_connections": 1,
  "active_xapps": 3,
  "uptime": "2h 30m"
}
```

#### List Connected E2 Nodes
```http
GET /api/v1/ric/e2nodes
```

**Response:**
```json
{
  "e2nodes": [
    {
      "node_id": "gnb_001",
      "ip": "10.45.1.100",
      "status": "connected",
      "last_seen": "2024-01-01T10:00:00Z"
    }
  ]
}
```

### gNB Management API

#### Get gNB Status
```http
GET /api/v1/gnb/status
```

**Response:**
```json
{
  "status": "active",
  "connected_ues": 2,
  "cell_id": "12345678",
  "bandwidth": "20MHz"
}
```

#### List Connected UEs
```http
GET /api/v1/gnb/ues
```

**Response:**
```json
{
  "ues": [
    {
      "imsi": "001010000000001",
      "ip": "10.45.1.200",
      "status": "connected",
      "last_activity": "2024-01-01T10:00:00Z"
    }
  ]
}
```

### UE Management API

#### Get UE Status
```http
GET /api/v1/ue/{imsi}/status
```

**Response:**
```json
{
  "imsi": "001010000000001",
  "ip": "10.45.1.200",
  "status": "registered",
  "cell_id": "12345678",
  "signal_strength": -70
}
```

#### Execute UE Command
```http
POST /api/v1/ue/{imsi}/command
```

**Request:**
```json
{
  "command": "ping",
  "target": "8.8.8.8",
  "count": 3
}
```

**Response:**
```json
{
  "result": "success",
  "output": "3 packets transmitted, 3 received, 0% packet loss"
}
```

### xApp Management API

#### List xApps
```http
GET /api/v1/xapps
```

**Response:**
```json
{
  "xapps": [
    {
      "name": "kpm-monitor",
      "status": "running",
      "version": "1.0.0",
      "subscriptions": 5
    },
    {
      "name": "rc-control",
      "status": "running",
      "version": "1.0.0",
      "subscriptions": 2
    }
  ]
}
```

#### Deploy xApp
```http
POST /api/v1/xapps/deploy
```

**Request:**
```json
{
  "name": "my-xapp",
  "image": "my-registry/my-xapp:latest",
  "config": {
    "subscription_interval": 1000
  }
}
```

### Monitoring API

#### Get System Metrics
```http
GET /api/v1/metrics/system
```

**Response:**
```json
{
  "cpu_usage": 45.2,
  "memory_usage": 67.8,
  "disk_usage": 23.1,
  "network_throughput": {
    "rx": 1024000,
    "tx": 512000
  }
}
```

#### Get E2 Metrics
```http
GET /api/v1/metrics/e2
```

**Response:**
```json
{
  "e2_setup_requests": 1,
  "e2_setup_responses": 1,
  "ric_subscriptions": 8,
  "ric_indications": 1500,
  "ric_controls": 25
}
```

## 🔧 Configuration API

### Update Configuration
```http
PUT /api/v1/config
```

**Request:**
```json
{
  "component": "gnb",
  "configuration": {
    "cell_id": "87654321",
    "bandwidth": "40MHz",
    "transmit_power": 23
  }
}
```

### Get Configuration
```http
GET /api/v1/config/{component}
```

**Response:**
```json
{
  "component": "gnb",
  "configuration": {
    "cell_id": "12345678",
    "bandwidth": "20MHz",
    "transmit_power": 20
  }
}
```

## 📊 E2 Interface Messages

### E2 Setup Request
```json
{
  "message_type": "E2SetupRequest",
  "transaction_id": 1,
  "global_e2_node_id": {
    "gnb_id": "001",
    "plmn_id": "00101"
  },
  "ran_functions": [
    {
      "function_id": 1,
      "function_definition": "KPM",
      "function_revision": 1
    }
  ]
}
```

### E2 Setup Response
```json
{
  "message_type": "E2SetupResponse",
  "transaction_id": 1,
  "accepted_functions": [
    {
      "function_id": 1,
      "function_definition": "KPM"
    }
  ]
}
```

### RIC Subscription Request
```json
{
  "message_type": "RICSubscriptionRequest",
  "request_id": 1,
  "ran_function_id": 1,
  "subscription_details": {
    "event_triggers": [
      {
        "trigger_type": "periodic",
        "interval": 1000
      }
    ],
    "actions": [
      {
        "action_id": 1,
        "action_type": "report",
        "action_definition": "cell_metrics"
      }
    ]
  }
}
```

### RIC Indication
```json
{
  "message_type": "RICIndication",
  "request_id": 1,
  "ran_function_id": 1,
  "indication_header": {
    "timestamp": "2024-01-01T10:00:00Z",
    "cell_id": "12345678"
  },
  "indication_message": {
    "metrics": {
      "dl_throughput": 50000000,
      "ul_throughput": 20000000,
      "connected_ues": 2,
      "prb_utilization": 75.5
    }
  }
}
```

## 🎯 xApp Development SDK

### Python SDK Example

```python
import openran_sdk

class MyXApp(openran_sdk.XApp):
    def __init__(self):
        super().__init__("my-xapp")
        
    def on_indication(self, indication):
        # Process RIC indication
        metrics = indication.get_metrics()
        
        if metrics.get('prb_utilization') > 80:
            # Send control message
            control_msg = self.create_control_message(
                action='reduce_power',
                target_cell=indication.cell_id
            )
            self.send_control(control_msg)
    
    def on_subscription_response(self, response):
        if response.is_success():
            print("Subscription successful")
        else:
            print(f"Subscription failed: {response.error}")

# Start xApp
xapp = MyXApp()
xapp.run()
```

### C++ SDK Example

```cpp
#include <openran_sdk.h>

class MyXApp : public openran::XApp {
public:
    MyXApp() : XApp("my-xapp") {}
    
    void onIndication(const openran::Indication& indication) override {
        auto metrics = indication.getMetrics();
        
        if (metrics.getPrbUtilization() > 80) {
            auto control = createControlMessage()
                .setAction("reduce_power")
                .setTargetCell(indication.getCellId());
            
            sendControl(control);
        }
    }
    
    void onSubscriptionResponse(const openran::SubscriptionResponse& response) override {
        if (response.isSuccess()) {
            LOG_INFO("Subscription successful");
        } else {
            LOG_ERROR("Subscription failed: " << response.getError());
        }
    }
};

int main() {
    MyXApp xapp;
    xapp.run();
    return 0;
}
```

## 🔍 Debug and Troubleshooting APIs

### Get Component Logs
```http
GET /api/v1/logs/{component}?lines=100
```

### Execute Debug Command
```http
POST /api/v1/debug/command
```

**Request:**
```json
{
  "component": "gnb",
  "command": "show_ue_stats",
  "parameters": {
    "imsi": "001010000000001"
  }
}
```

### Get Health Status
```http
GET /api/v1/health
```

**Response:**
```json
{
  "status": "healthy",
  "components": {
    "ric": "healthy",
    "gnb": "healthy",
    "core": "healthy",
    "monitoring": "healthy"
  },
  "checks": [
    {
      "name": "e2_connectivity",
      "status": "pass",
      "message": "E2 interface is operational"
    }
  ]
}
```

## 🔐 Authentication

### API Key Authentication
```http
GET /api/v1/ric/status
Authorization: Bearer <api_key>
```

### JWT Authentication
```http
POST /api/v1/auth/login
Content-Type: application/json

{
  "username": "admin",
  "password": "password"
}
```

**Response:**
```json
{
  "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "expires_in": 3600
}
```

## 📡 WebSocket APIs

### Real-time Metrics
```javascript
const ws = new WebSocket('ws://localhost:8080/ws/metrics');

ws.onmessage = function(event) {
    const metrics = JSON.parse(event.data);
    console.log('Real-time metrics:', metrics);
};
```

### E2 Message Stream
```javascript
const ws = new WebSocket('ws://localhost:8080/ws/e2');

ws.onmessage = function(event) {
    const e2_message = JSON.parse(event.data);
    console.log('E2 message:', e2_message);
};
```

## 🛠️ CLI Tools

### openran-cli

```bash
# Get RIC status
openran-cli ric status

# List connected UEs
openran-cli ue list

# Deploy xApp
openran-cli xapp deploy --name my-xapp --image my-registry/my-xapp:latest

# Get metrics
openran-cli metrics --component gnb --interval 5

# Execute UE command
openran-cli ue exec --imsi 001010000000001 --command "ping 8.8.8.8"

# View logs
openran-cli logs --component ric --follow
```

### openran-test

```bash
# Run connectivity tests
openran-test connectivity

# Run performance tests
openran-test performance --duration 300

# Run E2 interface tests
openran-test e2 --scenario subscription

# Generate test report
openran-test report --format html --output test_report.html
```

---

*API Version: 1.0.0*
*Last updated: $(date '+%Y-%m-%d')*