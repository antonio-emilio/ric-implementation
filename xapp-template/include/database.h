#ifndef DATABASE_H
#define DATABASE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sqlite3.h>
#include <stdbool.h>

#include "analytics.h"

// Database configuration
typedef struct {
    char database_path[512];
    int connection_timeout;
    int busy_timeout;
    bool enable_wal;
    bool enable_foreign_keys;
    int cache_size;
} database_config_t;

// Database context
typedef struct {
    sqlite3* db;
    database_config_t config;
    bool initialized;
    
    // Prepared statements for performance
    sqlite3_stmt* insert_metric_stmt;
    sqlite3_stmt* insert_anomaly_stmt;
    sqlite3_stmt* insert_recommendation_stmt;
    sqlite3_stmt* insert_event_stmt;
    sqlite3_stmt* select_recent_metrics_stmt;
    sqlite3_stmt* select_node_metrics_stmt;
    sqlite3_stmt* select_anomalies_stmt;
    sqlite3_stmt* select_recommendations_stmt;
    
    // Statistics
    uint64_t total_inserts;
    uint64_t total_queries;
    uint64_t total_errors;
    
} database_context_t;

// Event types for logging
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

// Event structure
typedef struct {
    event_type_t type;
    uint32_t node_id;
    uint32_t subscription_id;
    time_t timestamp;
    char message[512];
    char details[1024];
} event_data_t;

// Query result structures
typedef struct {
    metric_data_t* metrics;
    int count;
    int capacity;
} metric_query_result_t;

typedef struct {
    anomaly_result_t* anomalies;
    int count;
    int capacity;
} anomaly_query_result_t;

typedef struct {
    recommendation_result_t* recommendations;
    int count;
    int capacity;
} recommendation_query_result_t;

typedef struct {
    event_data_t* events;
    int count;
    int capacity;
} event_query_result_t;

// Function prototypes

// Context management
database_context_t* database_init(const char* database_path);
void database_cleanup(database_context_t* ctx);
int database_connect(database_context_t* ctx);
void database_disconnect(database_context_t* ctx);

// Schema management
int database_create_schema(database_context_t* ctx);
int database_upgrade_schema(database_context_t* ctx, int current_version, int target_version);
int database_check_schema_version(database_context_t* ctx);

// Prepared statements
int database_prepare_statements(database_context_t* ctx);
void database_finalize_statements(database_context_t* ctx);

// Metric operations
int database_insert_metric(database_context_t* ctx, const metric_data_t* metric);
int database_insert_metrics_batch(database_context_t* ctx, const metric_data_t* metrics, int count);
metric_query_result_t* database_query_metrics(database_context_t* ctx, metric_type_t type, uint32_t node_id, time_t start_time, time_t end_time);
metric_query_result_t* database_query_recent_metrics(database_context_t* ctx, metric_type_t type, int limit);

// Anomaly operations
int database_insert_anomaly(database_context_t* ctx, const anomaly_result_t* anomaly);
anomaly_query_result_t* database_query_anomalies(database_context_t* ctx, anomaly_severity_t severity, time_t start_time, time_t end_time);
anomaly_query_result_t* database_query_recent_anomalies(database_context_t* ctx, int limit);

// Recommendation operations
int database_insert_recommendation(database_context_t* ctx, const recommendation_result_t* recommendation);
recommendation_query_result_t* database_query_recommendations(database_context_t* ctx, recommendation_type_t type, time_t start_time, time_t end_time);
recommendation_query_result_t* database_query_recent_recommendations(database_context_t* ctx, int limit);

// Event operations
int database_insert_event(database_context_t* ctx, const event_data_t* event);
int database_log_event(database_context_t* ctx, event_type_t type, uint32_t node_id, uint32_t subscription_id, const char* message, const char* details);
event_query_result_t* database_query_events(database_context_t* ctx, event_type_t type, time_t start_time, time_t end_time);
event_query_result_t* database_query_recent_events(database_context_t* ctx, int limit);

// Statistics operations
int database_get_metric_stats(database_context_t* ctx, metric_type_t type, uint32_t node_id, time_t start_time, time_t end_time, stats_result_t* stats);
int database_get_node_stats(database_context_t* ctx, uint32_t node_id, time_t start_time, time_t end_time, char* stats_json, int json_size);
int database_get_overall_stats(database_context_t* ctx, time_t start_time, time_t end_time, char* stats_json, int json_size);

// Maintenance operations
int database_vacuum(database_context_t* ctx);
int database_analyze(database_context_t* ctx);
int database_cleanup_old_data(database_context_t* ctx, int retention_days);
int database_backup(database_context_t* ctx, const char* backup_path);
int database_restore(database_context_t* ctx, const char* backup_path);

// Result management
void database_free_metric_result(metric_query_result_t* result);
void database_free_anomaly_result(anomaly_query_result_t* result);
void database_free_recommendation_result(recommendation_query_result_t* result);
void database_free_event_result(event_query_result_t* result);

// Transaction support
int database_begin_transaction(database_context_t* ctx);
int database_commit_transaction(database_context_t* ctx);
int database_rollback_transaction(database_context_t* ctx);

// Utility functions
const char* database_event_type_to_string(event_type_t type);
const char* database_get_error_message(database_context_t* ctx);
int database_get_last_error_code(database_context_t* ctx);

// Performance monitoring
void database_print_performance(const database_context_t* ctx);
int database_get_table_sizes(database_context_t* ctx, char* sizes_json, int json_size);

// Configuration
int database_load_config(database_context_t* ctx, const char* config_file);
void database_print_config(const database_config_t* config);

// SQL query builder helpers
char* database_build_metric_query(metric_type_t type, uint32_t node_id, time_t start_time, time_t end_time, int limit);
char* database_build_anomaly_query(anomaly_severity_t severity, time_t start_time, time_t end_time, int limit);
char* database_build_recommendation_query(recommendation_type_t type, time_t start_time, time_t end_time, int limit);
char* database_build_event_query(event_type_t type, time_t start_time, time_t end_time, int limit);

// Database schema SQL
extern const char* DATABASE_SCHEMA_SQL;
extern const char* DATABASE_INDEXES_SQL;
extern const char* DATABASE_TRIGGERS_SQL;

#endif // DATABASE_H