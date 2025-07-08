/*
 * Smart Monitor xApp - Main Implementation
 * 
 * This is a comprehensive monitoring xApp for FlexRIC that provides:
 * - Real-time monitoring of KPM, RC, MAC, RLC, PDCP, and GTP metrics
 * - Intelligent anomaly detection and alerting
 * - Resource optimization recommendations
 * - Performance analytics and trend analysis
 * - Data persistence and historical analysis
 * 
 * Author: xApp Template Generator
 * Version: 1.0.0
 */

#include "smart_monitor_xapp.h"
#include <math.h>

// Global variables
xapp_context_t g_xapp_ctx;
volatile bool g_running = true;

// Signal handler for graceful shutdown
void signal_handler(int signal) {
    LOG_INFO("Received signal %d, initiating graceful shutdown...", signal);
    g_running = false;
    g_xapp_ctx.running = false;
    
    // Wake up any sleeping threads
    pthread_cond_broadcast(&g_xapp_ctx.state_cond);
}

// Main function
int main(int argc, char* argv[]) {
    int ret = 0;
    const char* duration_env = getenv("XAPP_DURATION");
    
    // Initialize logging
    utils_init_logging(LOG_FILE_PATH, LOG_LEVEL_INFO);
    
    LOG_INFO("=== Starting %s v%s ===", XAPP_NAME, XAPP_VERSION);
    
    // Initialize signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGQUIT, signal_handler);
    
    // Initialize application context
    memset(&g_xapp_ctx, 0, sizeof(xapp_context_t));
    g_xapp_ctx.state = XAPP_STATE_INIT;
    g_xapp_ctx.running = true;
    
    // Parse duration if provided
    if (duration_env) {
        g_xapp_ctx.duration = atoi(duration_env);
        LOG_INFO("xApp will run for %d seconds", g_xapp_ctx.duration);
    } else {
        g_xapp_ctx.duration = 0;  // Run indefinitely
        LOG_INFO("xApp will run indefinitely (use SIGINT/SIGTERM to stop)");
    }
    
    // Initialize application
    ret = xapp_init(&g_xapp_ctx);
    if (ret != 0) {
        LOG_ERROR("Failed to initialize xApp: %d", ret);
        goto cleanup;
    }
    
    // Start application
    ret = xapp_start(&g_xapp_ctx);
    if (ret != 0) {
        LOG_ERROR("Failed to start xApp: %d", ret);
        goto cleanup;
    }
    
    // Main execution loop
    time_t start_time = time(NULL);
    time_t last_stats_time = start_time;
    
    while (g_running && g_xapp_ctx.running) {
        time_t current_time = time(NULL);
        
        // Check duration limit
        if (g_xapp_ctx.duration > 0 && (current_time - start_time) >= g_xapp_ctx.duration) {
            LOG_INFO("Duration limit reached (%d seconds), stopping xApp", g_xapp_ctx.duration);
            break;
        }
        
        // Print statistics every 10 seconds
        if ((current_time - last_stats_time) >= 10) {
            print_statistics(&g_xapp_ctx);
            last_stats_time = current_time;
        }
        
        // Sleep for a short period
        usleep(100000);  // 100ms
    }
    
    LOG_INFO("=== Stopping %s ===", XAPP_NAME);
    
cleanup:
    // Stop application
    xapp_stop(&g_xapp_ctx);
    
    // Cleanup resources
    xapp_cleanup(&g_xapp_ctx);
    
    // Print final statistics
    print_statistics(&g_xapp_ctx);
    
    LOG_INFO("=== %s stopped ===", XAPP_NAME);
    
    // Cleanup logging
    utils_cleanup_logging();
    
    return ret;
}

// Initialize the xApp
int xapp_init(xapp_context_t* ctx) {
    int ret = 0;
    
    LOG_INFO("Initializing xApp...");
    
    // Initialize mutexes and condition variables
    ret = pthread_mutex_init(&ctx->state_mutex, NULL);
    if (ret != 0) {
        LOG_ERROR("Failed to initialize state mutex: %d", ret);
        return ret;
    }
    
    ret = pthread_cond_init(&ctx->state_cond, NULL);
    if (ret != 0) {
        LOG_ERROR("Failed to initialize state condition variable: %d", ret);
        return ret;
    }
    
    // Load configuration
    ret = load_config(ctx);
    if (ret != 0) {
        LOG_ERROR("Failed to load configuration: %d", ret);
        return ret;
    }
    
    // Initialize database
    ctx->db_ctx = database_init(ctx->config.database_path);
    if (!ctx->db_ctx) {
        LOG_ERROR("Failed to initialize database");
        return -1;
    }
    
    // Initialize analytics
    ctx->analytics_ctx = analytics_init(THRESHOLDS_FILE_PATH);
    if (!ctx->analytics_ctx) {
        LOG_ERROR("Failed to initialize analytics");
        return -1;
    }
    
    // Initialize statistics
    ctx->start_time = time(NULL);
    ctx->total_indications = 0;
    ctx->total_errors = 0;
    ctx->total_anomalies = 0;
    ctx->total_recommendations = 0;
    
    // Initialize node and subscription arrays
    ctx->node_count = 0;
    ctx->subscription_count = 0;
    
    ctx->state = XAPP_STATE_CONNECTING;
    
    LOG_INFO("xApp initialized successfully");
    return 0;
}

// Start the xApp
int xapp_start(xapp_context_t* ctx) {
    int ret = 0;
    
    LOG_INFO("Starting xApp...");

#ifndef SIMPLIFIED_BUILD
    // Initialize E2AP connection
    e2ap_init_params_t init_params = {
        .server_ip = ctx->config.ric_ip,
        .server_port = ctx->config.ric_port,
        .connection_callback = e2ap_connection_callback,
        .subscription_callback = e2ap_subscription_callback,
        .indication_callback = e2ap_indication_callback,
        .control_callback = e2ap_control_callback
    };
    
    ret = e2ap_init(&ctx->e2ap_handle, &init_params);
    if (ret != 0) {
        LOG_ERROR("Failed to initialize E2AP: %d", ret);
        return ret;
    }
    
    // Connect to nearRT-RIC
    ret = e2ap_connect(ctx->e2ap_handle);
    if (ret != 0) {
        LOG_ERROR("Failed to connect to nearRT-RIC: %d", ret);
        return ret;
    }
    
    // Wait for connection to be established
    int timeout = 30;  // 30 seconds timeout
    while (ctx->state != XAPP_STATE_CONNECTED && timeout > 0) {
        sleep(1);
        timeout--;
    }
    
    if (ctx->state != XAPP_STATE_CONNECTED) {
        LOG_ERROR("Failed to connect to nearRT-RIC within timeout");
        return -1;
    }
    
    // Create subscriptions
    ret = create_subscriptions(ctx);
    if (ret != 0) {
        LOG_ERROR("Failed to create subscriptions: %d", ret);
        return ret;
    }
#else
    LOG_INFO("Running in simplified mode (without FlexRIC integration)");
    ctx->state = XAPP_STATE_CONNECTED;
    
    // Create fake node for demonstration
    if (ctx->node_count < MAX_NODES) {
        node_info_t* node = &ctx->nodes[ctx->node_count++];
        node->node_id = 1;
        snprintf(node->node_name, sizeof(node->node_name), "Simulated_Node_1");
        node->connected = true;
        node->last_update = time(NULL);
        node->subscription_count = 0;
    }
#endif
    
    // Start monitoring thread
    ret = pthread_create(&ctx->monitor_thread, NULL, monitor_thread_func, ctx);
    if (ret != 0) {
        LOG_ERROR("Failed to create monitor thread: %d", ret);
        return ret;
    }
    
    // Start analytics thread
    ret = pthread_create(&ctx->analytics_thread, NULL, analytics_thread_func, ctx);
    if (ret != 0) {
        LOG_ERROR("Failed to create analytics thread: %d", ret);
        return ret;
    }
    
    ctx->state = XAPP_STATE_RUNNING;
    
    LOG_INFO("xApp started successfully");
    return 0;
}

// Stop the xApp
int xapp_stop(xapp_context_t* ctx) {
    LOG_INFO("Stopping xApp...");
    
    ctx->state = XAPP_STATE_STOPPING;
    ctx->running = false;
    
    // Wake up threads
    pthread_cond_broadcast(&ctx->state_cond);
    
    // Wait for threads to finish
    if (ctx->monitor_thread) {
        pthread_join(ctx->monitor_thread, NULL);
    }
    
    if (ctx->analytics_thread) {
        pthread_join(ctx->analytics_thread, NULL);
    }
    
    // Remove subscriptions
    remove_subscriptions(ctx);
    
    // Disconnect from nearRT-RIC
    if (ctx->e2ap_handle) {
        e2ap_disconnect(ctx->e2ap_handle);
    }
    
    ctx->state = XAPP_STATE_STOPPED;
    
    LOG_INFO("xApp stopped");
    return 0;
}

// Cleanup resources
void xapp_cleanup(xapp_context_t* ctx) {
    LOG_INFO("Cleaning up xApp resources...");
    
    // Cleanup E2AP
    if (ctx->e2ap_handle) {
        e2ap_cleanup(ctx->e2ap_handle);
    }
    
    // Cleanup analytics
    if (ctx->analytics_ctx) {
        analytics_cleanup(ctx->analytics_ctx);
    }
    
    // Cleanup database
    if (ctx->db_ctx) {
        database_cleanup(ctx->db_ctx);
    }
    
    // Cleanup mutexes
    pthread_mutex_destroy(&ctx->state_mutex);
    pthread_cond_destroy(&ctx->state_cond);
    
    LOG_INFO("xApp cleanup completed");
}

// Load configuration from file
int load_config(xapp_context_t* ctx) {
    LOG_INFO("Loading configuration...");
    
    // Set default values
    strcpy(ctx->config.xapp_name, XAPP_NAME);
    strcpy(ctx->config.version, XAPP_VERSION);
    ctx->config.monitoring_interval = DEFAULT_MONITORING_INTERVAL;
    strcpy(ctx->config.database_path, "/tmp/xapp_data.db");
    strcpy(ctx->config.log_level, "INFO");
    strcpy(ctx->config.ric_ip, DEFAULT_RIC_IP);
    ctx->config.ric_port = DEFAULT_RIC_PORT;
    
    // Enable all metrics by default
    ctx->config.kmp_enabled = true;
    ctx->config.rc_enabled = true;
    ctx->config.mac_enabled = true;
    ctx->config.rlc_enabled = true;
    ctx->config.pdcp_enabled = true;
    ctx->config.gtp_enabled = true;
    
    // Enable all analytics by default
    ctx->config.anomaly_detection = true;
    ctx->config.trend_analysis = true;
    ctx->config.recommendations = true;
    ctx->config.alert_threshold = 0.8;
    
    // Try to load configuration file
    json_object* config_obj = utils_json_load_file(CONFIG_FILE_PATH);
    if (config_obj) {
        LOG_INFO("Loading configuration from %s", CONFIG_FILE_PATH);
        
        // Parse configuration values
        utils_json_get_string(config_obj, "xapp_name", ctx->config.xapp_name, sizeof(ctx->config.xapp_name));
        utils_json_get_string(config_obj, "version", ctx->config.version, sizeof(ctx->config.version));
        utils_json_get_int(config_obj, "monitoring_interval", &ctx->config.monitoring_interval);
        utils_json_get_string(config_obj, "database_path", ctx->config.database_path, sizeof(ctx->config.database_path));
        utils_json_get_string(config_obj, "log_level", ctx->config.log_level, sizeof(ctx->config.log_level));
        utils_json_get_string(config_obj, "ric_ip", ctx->config.ric_ip, sizeof(ctx->config.ric_ip));
        utils_json_get_int(config_obj, "ric_port", &ctx->config.ric_port);
        
        // Parse metrics configuration
        json_object* metrics_obj;
        if (json_object_object_get_ex(config_obj, "metrics", &metrics_obj)) {
            utils_json_get_bool(metrics_obj, "kmp_enabled", &ctx->config.kmp_enabled);
            utils_json_get_bool(metrics_obj, "rc_enabled", &ctx->config.rc_enabled);
            utils_json_get_bool(metrics_obj, "mac_enabled", &ctx->config.mac_enabled);
            utils_json_get_bool(metrics_obj, "rlc_enabled", &ctx->config.rlc_enabled);
            utils_json_get_bool(metrics_obj, "pdcp_enabled", &ctx->config.pdcp_enabled);
            utils_json_get_bool(metrics_obj, "gtp_enabled", &ctx->config.gtp_enabled);
        }
        
        // Parse analytics configuration
        json_object* analytics_obj;
        if (json_object_object_get_ex(config_obj, "analytics", &analytics_obj)) {
            utils_json_get_bool(analytics_obj, "anomaly_detection", &ctx->config.anomaly_detection);
            utils_json_get_bool(analytics_obj, "trend_analysis", &ctx->config.trend_analysis);
            utils_json_get_bool(analytics_obj, "recommendations", &ctx->config.recommendations);
            utils_json_get_double(analytics_obj, "alert_threshold", &ctx->config.alert_threshold);
        }
        
        json_object_put(config_obj);
    } else {
        LOG_WARN("Configuration file not found, using default values");
    }
    
    print_config(&ctx->config);
    
    LOG_INFO("Configuration loaded successfully");
    return 0;
}

// Print configuration
void print_config(const xapp_config_t* config) {
    LOG_INFO("=== Configuration ===");
    LOG_INFO("xApp Name: %s", config->xapp_name);
    LOG_INFO("Version: %s", config->version);
    LOG_INFO("Monitoring Interval: %d ms", config->monitoring_interval);
    LOG_INFO("Database Path: %s", config->database_path);
    LOG_INFO("Log Level: %s", config->log_level);
    LOG_INFO("RIC IP: %s", config->ric_ip);
    LOG_INFO("RIC Port: %d", config->ric_port);
    
    LOG_INFO("=== Metrics Configuration ===");
    LOG_INFO("KMP Enabled: %s", config->kmp_enabled ? "Yes" : "No");
    LOG_INFO("RC Enabled: %s", config->rc_enabled ? "Yes" : "No");
    LOG_INFO("MAC Enabled: %s", config->mac_enabled ? "Yes" : "No");
    LOG_INFO("RLC Enabled: %s", config->rlc_enabled ? "Yes" : "No");
    LOG_INFO("PDCP Enabled: %s", config->pdcp_enabled ? "Yes" : "No");
    LOG_INFO("GTP Enabled: %s", config->gtp_enabled ? "Yes" : "No");
    
    LOG_INFO("=== Analytics Configuration ===");
    LOG_INFO("Anomaly Detection: %s", config->anomaly_detection ? "Yes" : "No");
    LOG_INFO("Trend Analysis: %s", config->trend_analysis ? "Yes" : "No");
    LOG_INFO("Recommendations: %s", config->recommendations ? "Yes" : "No");
    LOG_INFO("Alert Threshold: %.2f", config->alert_threshold);
    LOG_INFO("=====================");
}

// Print statistics
void print_statistics(const xapp_context_t* ctx) {
    time_t current_time = time(NULL);
    double uptime = difftime(current_time, ctx->start_time);
    
    LOG_INFO("=== Statistics (Uptime: %.0f seconds) ===", uptime);
    LOG_INFO("State: %s", ctx->state == XAPP_STATE_RUNNING ? "Running" : "Other");
    LOG_INFO("Connected Nodes: %d", ctx->node_count);
    LOG_INFO("Active Subscriptions: %d", ctx->subscription_count);
    LOG_INFO("Total Indications: %llu", (unsigned long long)ctx->total_indications);
    LOG_INFO("Total Errors: %llu", (unsigned long long)ctx->total_errors);
    LOG_INFO("Total Anomalies: %llu", (unsigned long long)ctx->total_anomalies);
    LOG_INFO("Total Recommendations: %llu", (unsigned long long)ctx->total_recommendations);
    
    if (uptime > 0) {
        LOG_INFO("Indications/sec: %.2f", ctx->total_indications / uptime);
    }
    
    // Print database statistics
    if (ctx->db_ctx) {
        database_print_performance(ctx->db_ctx);
    }
    
    // Print analytics statistics
    if (ctx->analytics_ctx) {
        analytics_print_performance(ctx->analytics_ctx);
    }
    
    LOG_INFO("=====================================");
}

// E2AP Connection callback
void e2ap_connection_callback(e2ap_handle_t handle, uint32_t node_id, bool connected) {
    xapp_context_t* ctx = &g_xapp_ctx;
    
    if (connected) {
        LOG_INFO("E2 Node %u connected", node_id);
        
        // Find or create node entry
        node_info_t* node = find_node(ctx, node_id);
        if (!node && ctx->node_count < MAX_NODES) {
            node = &ctx->nodes[ctx->node_count++];
            node->node_id = node_id;
            snprintf(node->node_name, sizeof(node->node_name), "Node_%u", node_id);
            node->subscription_count = 0;
        }
        
        if (node) {
            node->connected = true;
            node->last_update = time(NULL);
        }
        
        // Update state if this is the first connection
        if (ctx->state == XAPP_STATE_CONNECTING) {
            ctx->state = XAPP_STATE_CONNECTED;
            pthread_cond_signal(&ctx->state_cond);
        }
        
        // Log event to database
        if (ctx->db_ctx) {
            database_log_event(ctx->db_ctx, EVENT_NODE_CONNECT, node_id, 0, "Node connected", "");
        }
    } else {
        LOG_INFO("E2 Node %u disconnected", node_id);
        
        // Update node status
        node_info_t* node = find_node(ctx, node_id);
        if (node) {
            node->connected = false;
            node->last_update = time(NULL);
        }
        
        // Log event to database
        if (ctx->db_ctx) {
            database_log_event(ctx->db_ctx, EVENT_NODE_DISCONNECT, node_id, 0, "Node disconnected", "");
        }
    }
}

// E2AP Subscription callback
void e2ap_subscription_callback(e2ap_handle_t handle, uint32_t subscription_id, bool success) {
    xapp_context_t* ctx = &g_xapp_ctx;
    
    if (success) {
        LOG_INFO("Subscription %u created successfully", subscription_id);
        
        // Update subscription status
        subscription_info_t* sub = find_subscription(ctx, subscription_id);
        if (sub) {
            sub->active = true;
            sub->created_at = time(NULL);
        }
        
        // Log event to database
        if (ctx->db_ctx) {
            database_log_event(ctx->db_ctx, EVENT_SUBSCRIPTION_CREATE, 0, subscription_id, "Subscription created", "");
        }
    } else {
        LOG_ERROR("Subscription %u creation failed", subscription_id);
        ctx->total_errors++;
        
        // Log event to database
        if (ctx->db_ctx) {
            database_log_event(ctx->db_ctx, EVENT_ERROR, 0, subscription_id, "Subscription creation failed", "");
        }
    }
}

// E2AP Indication callback
void e2ap_indication_callback(e2ap_handle_t handle, uint32_t subscription_id, const e2ap_indication_t* indication) {
    xapp_context_t* ctx = &g_xapp_ctx;
    
    ctx->total_indications++;
    
    // Update subscription statistics
    subscription_info_t* sub = find_subscription(ctx, subscription_id);
    if (sub) {
        sub->indication_count++;
        
        // Route to appropriate handler based on service model
        if (strcmp(sub->sm_name, "KMP") == 0) {
            handle_kmp_indication(ctx, indication);
        } else if (strcmp(sub->sm_name, "RC") == 0) {
            handle_rc_indication(ctx, indication);
        } else if (strcmp(sub->sm_name, "MAC") == 0) {
            handle_mac_indication(ctx, indication);
        } else if (strcmp(sub->sm_name, "RLC") == 0) {
            handle_rlc_indication(ctx, indication);
        } else if (strcmp(sub->sm_name, "PDCP") == 0) {
            handle_pdcp_indication(ctx, indication);
        } else if (strcmp(sub->sm_name, "GTP") == 0) {
            handle_gtp_indication(ctx, indication);
        }
    }
    
    // Log event to database
    if (ctx->db_ctx) {
        database_log_event(ctx->db_ctx, EVENT_INDICATION_RECEIVED, 0, subscription_id, "Indication received", "");
    }
}

// E2AP Control callback
void e2ap_control_callback(e2ap_handle_t handle, uint32_t request_id, bool success) {
    xapp_context_t* ctx = &g_xapp_ctx;
    
    if (success) {
        LOG_DEBUG("Control request %u successful", request_id);
    } else {
        LOG_ERROR("Control request %u failed", request_id);
        ctx->total_errors++;
    }
    
    // Log event to database
    if (ctx->db_ctx) {
        database_log_event(ctx->db_ctx, EVENT_CONTROL_SENT, 0, request_id, 
                          success ? "Control successful" : "Control failed", "");
    }
}

// Service model indication handlers (simplified implementations)
void handle_kmp_indication(xapp_context_t* ctx, const e2ap_indication_t* indication) {
    // Parse KMP indication and extract metrics
    // This is a simplified implementation - in real scenario, you would parse the actual indication
    
    // Example: Extract throughput metric
    double throughput = 100.0 + (rand() % 900);  // Simulated value
    analytics_add_metric(ctx->analytics_ctx, METRIC_THROUGHPUT, throughput, indication->node_id, 0);
    
    // Example: Extract latency metric
    double latency = 10.0 + (rand() % 50);  // Simulated value
    analytics_add_metric(ctx->analytics_ctx, METRIC_LATENCY, latency, indication->node_id, 0);
}

void handle_rc_indication(xapp_context_t* ctx, const e2ap_indication_t* indication) {
    // Parse RC indication and extract metrics
    // This is a simplified implementation
    
    // Example: Extract RSRP metric
    double rsrp = -100.0 + (rand() % 50);  // Simulated value
    analytics_add_metric(ctx->analytics_ctx, METRIC_RSRP, rsrp, indication->node_id, 0);
}

void handle_mac_indication(xapp_context_t* ctx, const e2ap_indication_t* indication) {
    // Parse MAC indication and extract metrics
    // This is a simplified implementation
    
    // Example: Extract PRB usage
    double prb_usage = rand() % 100;  // Simulated value
    analytics_add_metric(ctx->analytics_ctx, METRIC_PRB_USAGE, prb_usage, indication->node_id, 0);
}

void handle_rlc_indication(xapp_context_t* ctx, const e2ap_indication_t* indication) {
    // Parse RLC indication and extract metrics
    // This is a simplified implementation
    
    // Example: Extract packet loss
    double packet_loss = (rand() % 100) / 100.0;  // Simulated value
    analytics_add_metric(ctx->analytics_ctx, METRIC_PACKET_LOSS, packet_loss, indication->node_id, 0);
}

void handle_pdcp_indication(xapp_context_t* ctx, const e2ap_indication_t* indication) {
    // Parse PDCP indication and extract metrics
    // This is a simplified implementation
    
    // Example: Extract CPU utilization
    double cpu_usage = 20.0 + (rand() % 60);  // Simulated value
    analytics_add_metric(ctx->analytics_ctx, METRIC_CPU_UTILIZATION, cpu_usage, indication->node_id, 0);
}

void handle_gtp_indication(xapp_context_t* ctx, const e2ap_indication_t* indication) {
    // Parse GTP indication and extract metrics
    // This is a simplified implementation
    
    // Example: Extract memory usage
    double memory_usage = 30.0 + (rand() % 50);  // Simulated value
    analytics_add_metric(ctx->analytics_ctx, METRIC_MEMORY_USAGE, memory_usage, indication->node_id, 0);
}

// Create subscriptions for all enabled service models
int create_subscriptions(xapp_context_t* ctx) {
    LOG_INFO("Creating subscriptions...");
    
    // This is a simplified implementation
    // In a real scenario, you would create actual E2AP subscriptions
    
    int subscription_id = 1;
    
    for (int i = 0; i < ctx->node_count; i++) {
        uint32_t node_id = ctx->nodes[i].node_id;
        
        // Create KMP subscription
        if (ctx->config.kmp_enabled && ctx->subscription_count < MAX_NODES * 6) {
            subscription_info_t* sub = &ctx->subscriptions[ctx->subscription_count++];
            sub->subscription_id = subscription_id++;
            sub->node_id = node_id;
            sub->ran_func_id = 2;  // KMP RAN function ID
            strcpy(sub->sm_name, "KMP");
            sub->active = false;
            sub->indication_count = 0;
        }
        
        // Create RC subscription
        if (ctx->config.rc_enabled && ctx->subscription_count < MAX_NODES * 6) {
            subscription_info_t* sub = &ctx->subscriptions[ctx->subscription_count++];
            sub->subscription_id = subscription_id++;
            sub->node_id = node_id;
            sub->ran_func_id = 3;  // RC RAN function ID
            strcpy(sub->sm_name, "RC");
            sub->active = false;
            sub->indication_count = 0;
        }
        
        // Similar for other service models...
    }
    
    LOG_INFO("Created %d subscriptions", ctx->subscription_count);
    return 0;
}

// Remove all subscriptions
int remove_subscriptions(xapp_context_t* ctx) {
    LOG_INFO("Removing subscriptions...");
    
    // This is a simplified implementation
    // In a real scenario, you would remove actual E2AP subscriptions
    
    for (int i = 0; i < ctx->subscription_count; i++) {
        subscription_info_t* sub = &ctx->subscriptions[i];
        sub->active = false;
        
        // Log event to database
        if (ctx->db_ctx) {
            database_log_event(ctx->db_ctx, EVENT_SUBSCRIPTION_DELETE, sub->node_id, sub->subscription_id, 
                              "Subscription removed", "");
        }
    }
    
    ctx->subscription_count = 0;
    
    LOG_INFO("Removed all subscriptions");
    return 0;
}

// Find subscription by ID
subscription_info_t* find_subscription(xapp_context_t* ctx, uint32_t subscription_id) {
    for (int i = 0; i < ctx->subscription_count; i++) {
        if (ctx->subscriptions[i].subscription_id == subscription_id) {
            return &ctx->subscriptions[i];
        }
    }
    return NULL;
}

// Find node by ID
node_info_t* find_node(xapp_context_t* ctx, uint32_t node_id) {
    for (int i = 0; i < ctx->node_count; i++) {
        if (ctx->nodes[i].node_id == node_id) {
            return &ctx->nodes[i];
        }
    }
    return NULL;
}

// Monitor thread function
void* monitor_thread_func(void* arg) {
    xapp_context_t* ctx = (xapp_context_t*)arg;
    
    LOG_INFO("Monitor thread started");
    
    while (ctx->running) {
        time_t current_time = time(NULL);
        
        // Monitor node connections
        for (int i = 0; i < ctx->node_count; i++) {
            node_info_t* node = &ctx->nodes[i];
            
            // Check for stale connections
            if (node->connected && (current_time - node->last_update) > 60) {
                LOG_WARN("Node %u appears to be stale (last update: %ld seconds ago)", 
                        node->node_id, current_time - node->last_update);
            }
        }
        
#ifdef SIMPLIFIED_BUILD
        // Generate simulated metrics in simplified mode
        if (ctx->analytics_ctx && ctx->node_count > 0) {
            // Generate some realistic simulated metrics
            double base_throughput = 150.0;
            double base_latency = 25.0;
            double base_rsrp = -85.0;
            
            // Add some variance and trends
            double time_factor = (double)(current_time % 3600) / 3600.0;  // 0-1 over an hour
            double noise = ((double)rand() / RAND_MAX - 0.5) * 0.2;  // ±10% noise
            
            // Throughput with daily pattern
            double throughput = base_throughput * (0.8 + 0.4 * sin(time_factor * 2 * M_PI)) * (1.0 + noise);
            analytics_add_metric(ctx->analytics_ctx, METRIC_THROUGHPUT, throughput, 1, 1);
            
            // Latency with inverse relationship to throughput
            double latency = base_latency * (1.2 - 0.4 * sin(time_factor * 2 * M_PI)) * (1.0 + noise);
            analytics_add_metric(ctx->analytics_ctx, METRIC_LATENCY, latency, 1, 1);
            
            // RSRP with some random walk
            static double rsrp_drift = 0.0;
            rsrp_drift += ((double)rand() / RAND_MAX - 0.5) * 2.0;  // Random walk
            rsrp_drift = fmax(-10.0, fmin(10.0, rsrp_drift));  // Clamp drift
            double rsrp = base_rsrp + rsrp_drift + noise * 5.0;
            analytics_add_metric(ctx->analytics_ctx, METRIC_RSRP, rsrp, 1, 1);
            
            // CPU utilization based on throughput
            double cpu_util = 30.0 + (throughput / base_throughput) * 40.0 + noise * 10.0;
            analytics_add_metric(ctx->analytics_ctx, METRIC_CPU_UTILIZATION, cpu_util, 1, 1);
            
            // PRB usage
            double prb_usage = 40.0 + (throughput / base_throughput) * 35.0 + noise * 15.0;
            analytics_add_metric(ctx->analytics_ctx, METRIC_PRB_USAGE, prb_usage, 1, 1);
            
            ctx->total_indications += 5;  // Count simulated indications
            
            LOG_DEBUG("Generated simulated metrics: throughput=%.1f, latency=%.1f, rsrp=%.1f, cpu=%.1f, prb=%.1f",
                     throughput, latency, rsrp, cpu_util, prb_usage);
        }
#endif
        
        // Sleep for monitoring interval
        usleep(ctx->config.monitoring_interval * 1000);
    }
    
    LOG_INFO("Monitor thread stopped");
    return NULL;
}

// Analytics thread function
void* analytics_thread_func(void* arg) {
    xapp_context_t* ctx = (xapp_context_t*)arg;
    
    LOG_INFO("Analytics thread started");
    
    while (ctx->running) {
        // Process analytics if enabled
        if (ctx->config.anomaly_detection || ctx->config.trend_analysis || ctx->config.recommendations) {
            
            // Check for anomalies
            if (ctx->analytics_ctx) {
                int anomaly_count = 0;
                anomaly_result_t* anomalies = analytics_get_recent_anomalies(ctx->analytics_ctx, &anomaly_count);
                
                if (anomalies && anomaly_count > 0) {
                    for (int i = 0; i < anomaly_count; i++) {
                        const anomaly_result_t* anomaly = &anomalies[i];
                        
                        if (anomaly->severity >= ANOMALY_WARNING) {
                            LOG_WARN("Anomaly detected: %s", anomaly->description);
                            ctx->total_anomalies++;
                            
                            // Store anomaly in database
                            if (ctx->db_ctx) {
                                database_insert_anomaly(ctx->db_ctx, anomaly);
                                database_log_event(ctx->db_ctx, EVENT_ANOMALY_DETECTED, 0, 0, 
                                                  "Anomaly detected", anomaly->description);
                            }
                        }
                    }
                }
                
                // Check for new recommendations
                int recommendation_count = 0;
                recommendation_result_t* recommendations = analytics_get_recent_recommendations(ctx->analytics_ctx, &recommendation_count);
                
                if (recommendations && recommendation_count > 0) {
                    for (int i = 0; i < recommendation_count; i++) {
                        const recommendation_result_t* rec = &recommendations[i];
                        
                        LOG_INFO("Recommendation: %s", rec->description);
                        ctx->total_recommendations++;
                        
                        // Store recommendation in database
                        if (ctx->db_ctx) {
                            database_insert_recommendation(ctx->db_ctx, rec);
                            database_log_event(ctx->db_ctx, EVENT_RECOMMENDATION_GENERATED, rec->node_id, 0, 
                                              "Recommendation generated", rec->description);
                        }
                    }
                }
            }
        }
        
        // Sleep for analytics interval
        sleep(5);  // Run analytics every 5 seconds
    }
    
    LOG_INFO("Analytics thread stopped");
    return NULL;
}