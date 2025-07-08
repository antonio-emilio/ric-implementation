#ifndef SMART_MONITOR_XAPP_H
#define SMART_MONITOR_XAPP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <json-c/json.h>
#include <sqlite3.h>

#ifndef SIMPLIFIED_BUILD
// FlexRIC includes
#include "flexric/src/lib/e2ap/e2ap_api.h"
#include "flexric/src/lib/e2ap/e2ap_types.h"
#include "flexric/src/lib/e2ap/e2ap_msg_enc.h"
#include "flexric/src/lib/e2ap/e2ap_msg_dec.h"
#include "flexric/src/lib/e2ap/e2ap_node.h"
#include "flexric/src/lib/e2ap/e2ap_ric.h"
#include "flexric/src/lib/e2ap/e2ap_subscription.h"
#include "flexric/src/lib/e2ap/e2ap_indication.h"
#include "flexric/src/lib/e2ap/e2ap_control.h"

// Service Model includes
#include "flexric/src/lib/sm/kpm_sm/kmp_sm_api.h"
#include "flexric/src/lib/sm/rc_sm/rc_sm_api.h"
#include "flexric/src/lib/sm/mac_sm/mac_sm_api.h"
#include "flexric/src/lib/sm/rlc_sm/rlc_sm_api.h"
#include "flexric/src/lib/sm/pdcp_sm/pdcp_sm_api.h"
#include "flexric/src/lib/sm/gtp_sm/gtp_sm_api.h"
#else
// Simplified build - define necessary types
typedef void* e2ap_handle_t;
typedef struct {
    uint32_t node_id;
    void* data;
    size_t data_size;
} e2ap_indication_t;

typedef struct {
    char* server_ip;
    int server_port;
    void (*connection_callback)(e2ap_handle_t, uint32_t, bool);
    void (*subscription_callback)(e2ap_handle_t, uint32_t, bool);
    void (*indication_callback)(e2ap_handle_t, uint32_t, const e2ap_indication_t*);
    void (*control_callback)(e2ap_handle_t, uint32_t, bool);
} e2ap_init_params_t;

// Simplified E2AP functions
static inline int e2ap_init(e2ap_handle_t* handle, e2ap_init_params_t* params) { return 0; }
static inline int e2ap_connect(e2ap_handle_t handle) { return 0; }
static inline int e2ap_disconnect(e2ap_handle_t handle) { return 0; }
static inline void e2ap_cleanup(e2ap_handle_t handle) { }
#endif

// Application includes
#include "analytics.h"
#include "database.h"
#include "utils.h"

// Constants
#define XAPP_NAME "Smart Monitor xApp"
#define XAPP_VERSION "1.0.0"
#define CONFIG_FILE_PATH "config/xapp_config.json"
#define THRESHOLDS_FILE_PATH "config/thresholds.json"
#define LOG_FILE_PATH "/tmp/smart_monitor_xapp.log"
#define DEFAULT_MONITORING_INTERVAL 1000  // milliseconds
#define DEFAULT_RIC_IP "127.0.0.1"
#define DEFAULT_RIC_PORT 36421
#define MAX_BUFFER_SIZE 4096
#define MAX_NODES 32
#define MAX_METRICS 1000

// Global states
typedef enum {
    XAPP_STATE_INIT,
    XAPP_STATE_CONNECTING,
    XAPP_STATE_CONNECTED,
    XAPP_STATE_RUNNING,
    XAPP_STATE_STOPPING,
    XAPP_STATE_STOPPED
} xapp_state_t;

// Configuration structure
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

// Node information
typedef struct {
    uint32_t node_id;
    char node_name[128];
    bool connected;
    time_t last_update;
    uint32_t subscription_count;
} node_info_t;

// Subscription information
typedef struct {
    uint32_t subscription_id;
    uint32_t node_id;
    uint16_t ran_func_id;
    char sm_name[64];
    bool active;
    time_t created_at;
    uint32_t indication_count;
} subscription_info_t;

// Main application context
typedef struct {
    xapp_state_t state;
    xapp_config_t config;
    
    // E2AP context
    e2ap_handle_t e2ap_handle;
    node_info_t nodes[MAX_NODES];
    uint32_t node_count;
    
    // Subscription management
    subscription_info_t subscriptions[MAX_NODES * 6];  // Max 6 SMs per node
    uint32_t subscription_count;
    
    // Threading
    pthread_t main_thread;
    pthread_t monitor_thread;
    pthread_t analytics_thread;
    pthread_mutex_t state_mutex;
    pthread_cond_t state_cond;
    
    // Database context
    database_context_t* db_ctx;
    
    // Analytics context
    analytics_context_t* analytics_ctx;
    
    // Runtime controls
    bool running;
    int duration;  // seconds, 0 for infinite
    time_t start_time;
    
    // Statistics
    uint64_t total_indications;
    uint64_t total_errors;
    uint64_t total_anomalies;
    uint64_t total_recommendations;
    
} xapp_context_t;

// Function prototypes
int xapp_init(xapp_context_t* ctx);
int xapp_start(xapp_context_t* ctx);
int xapp_stop(xapp_context_t* ctx);
void xapp_cleanup(xapp_context_t* ctx);

// Configuration functions
int load_config(xapp_context_t* ctx);
int load_thresholds(xapp_context_t* ctx);
void print_config(const xapp_config_t* config);

// E2AP callback functions
void e2ap_connection_callback(e2ap_handle_t handle, uint32_t node_id, bool connected);
void e2ap_subscription_callback(e2ap_handle_t handle, uint32_t subscription_id, bool success);
void e2ap_indication_callback(e2ap_handle_t handle, uint32_t subscription_id, const e2ap_indication_t* indication);
void e2ap_control_callback(e2ap_handle_t handle, uint32_t request_id, bool success);

// Service Model handlers
void handle_kmp_indication(xapp_context_t* ctx, const e2ap_indication_t* indication);
void handle_rc_indication(xapp_context_t* ctx, const e2ap_indication_t* indication);
void handle_mac_indication(xapp_context_t* ctx, const e2ap_indication_t* indication);
void handle_rlc_indication(xapp_context_t* ctx, const e2ap_indication_t* indication);
void handle_pdcp_indication(xapp_context_t* ctx, const e2ap_indication_t* indication);
void handle_gtp_indication(xapp_context_t* ctx, const e2ap_indication_t* indication);

// Subscription management
int create_subscriptions(xapp_context_t* ctx);
int remove_subscriptions(xapp_context_t* ctx);
subscription_info_t* find_subscription(xapp_context_t* ctx, uint32_t subscription_id);
node_info_t* find_node(xapp_context_t* ctx, uint32_t node_id);

// Thread functions
void* monitor_thread_func(void* arg);
void* analytics_thread_func(void* arg);

// Control functions
int send_control_message(xapp_context_t* ctx, uint32_t node_id, uint16_t ran_func_id, const void* control_msg);

// Statistics and reporting
void print_statistics(const xapp_context_t* ctx);
void generate_report(const xapp_context_t* ctx);

// Signal handlers
void signal_handler(int signal);

// Global application context
extern xapp_context_t g_xapp_ctx;
extern volatile bool g_running;

#endif // SMART_MONITOR_XAPP_H