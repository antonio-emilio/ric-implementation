/*
 * Database Module for Smart Monitor xApp
 * 
 * This module provides data persistence capabilities including:
 * - SQLite database management
 * - Metric storage and retrieval
 * - Anomaly and recommendation logging
 * - Event logging and querying
 * - Performance monitoring
 * 
 * Author: xApp Template Generator
 * Version: 1.0.0
 */

#include "database.h"
#include "utils.h"

// Database schema SQL
const char* DATABASE_SCHEMA_SQL = 
    "CREATE TABLE IF NOT EXISTS metrics ("
    "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "  metric_type INTEGER NOT NULL,"
    "  value REAL NOT NULL,"
    "  node_id INTEGER NOT NULL,"
    "  cell_id INTEGER NOT NULL,"
    "  timestamp INTEGER NOT NULL"
    ");"
    
    "CREATE TABLE IF NOT EXISTS anomalies ("
    "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "  metric_type INTEGER NOT NULL,"
    "  severity INTEGER NOT NULL,"
    "  threshold_value REAL NOT NULL,"
    "  actual_value REAL NOT NULL,"
    "  confidence REAL NOT NULL,"
    "  detected_at INTEGER NOT NULL,"
    "  description TEXT NOT NULL"
    ");"
    
    "CREATE TABLE IF NOT EXISTS recommendations ("
    "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "  type INTEGER NOT NULL,"
    "  node_id INTEGER NOT NULL,"
    "  cell_id INTEGER NOT NULL,"
    "  confidence REAL NOT NULL,"
    "  expected_improvement REAL NOT NULL,"
    "  generated_at INTEGER NOT NULL,"
    "  description TEXT NOT NULL,"
    "  parameters TEXT NOT NULL"
    ");"
    
    "CREATE TABLE IF NOT EXISTS events ("
    "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "  event_type INTEGER NOT NULL,"
    "  node_id INTEGER NOT NULL,"
    "  subscription_id INTEGER NOT NULL,"
    "  timestamp INTEGER NOT NULL,"
    "  message TEXT NOT NULL,"
    "  details TEXT NOT NULL"
    ");"
    
    "CREATE TABLE IF NOT EXISTS schema_version ("
    "  version INTEGER PRIMARY KEY"
    ");";

// Database indexes SQL
const char* DATABASE_INDEXES_SQL = 
    "CREATE INDEX IF NOT EXISTS idx_metrics_timestamp ON metrics(timestamp);"
    "CREATE INDEX IF NOT EXISTS idx_metrics_type_node ON metrics(metric_type, node_id);"
    "CREATE INDEX IF NOT EXISTS idx_anomalies_timestamp ON anomalies(detected_at);"
    "CREATE INDEX IF NOT EXISTS idx_anomalies_severity ON anomalies(severity);"
    "CREATE INDEX IF NOT EXISTS idx_recommendations_timestamp ON recommendations(generated_at);"
    "CREATE INDEX IF NOT EXISTS idx_recommendations_type ON recommendations(type);"
    "CREATE INDEX IF NOT EXISTS idx_events_timestamp ON events(timestamp);"
    "CREATE INDEX IF NOT EXISTS idx_events_type ON events(event_type);";

// Database triggers SQL (for cleanup)
const char* DATABASE_TRIGGERS_SQL = 
    "CREATE TRIGGER IF NOT EXISTS cleanup_old_metrics "
    "AFTER INSERT ON metrics "
    "BEGIN "
    "  DELETE FROM metrics WHERE timestamp < (NEW.timestamp - 86400 * 7); "
    "END;"
    
    "CREATE TRIGGER IF NOT EXISTS cleanup_old_events "
    "AFTER INSERT ON events "
    "BEGIN "
    "  DELETE FROM events WHERE timestamp < (NEW.timestamp - 86400 * 30); "
    "END;";

// String conversion functions
const char* database_event_type_to_string(event_type_t type) {
    switch (type) {
        case EVENT_XAPP_START: return "XAPP_START";
        case EVENT_XAPP_STOP: return "XAPP_STOP";
        case EVENT_NODE_CONNECT: return "NODE_CONNECT";
        case EVENT_NODE_DISCONNECT: return "NODE_DISCONNECT";
        case EVENT_SUBSCRIPTION_CREATE: return "SUBSCRIPTION_CREATE";
        case EVENT_SUBSCRIPTION_DELETE: return "SUBSCRIPTION_DELETE";
        case EVENT_INDICATION_RECEIVED: return "INDICATION_RECEIVED";
        case EVENT_CONTROL_SENT: return "CONTROL_SENT";
        case EVENT_ANOMALY_DETECTED: return "ANOMALY_DETECTED";
        case EVENT_RECOMMENDATION_GENERATED: return "RECOMMENDATION_GENERATED";
        case EVENT_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

// Initialize database context
database_context_t* database_init(const char* database_path) {
    database_context_t* ctx = malloc(sizeof(database_context_t));
    if (!ctx) {
        LOG_ERROR("Failed to allocate database context");
        return NULL;
    }
    
    memset(ctx, 0, sizeof(database_context_t));
    
    // Set default configuration
    if (database_path) {
        strncpy(ctx->config.database_path, database_path, sizeof(ctx->config.database_path) - 1);
    } else {
        strncpy(ctx->config.database_path, "/tmp/xapp_data.db", sizeof(ctx->config.database_path) - 1);
    }
    
    ctx->config.connection_timeout = 30;
    ctx->config.busy_timeout = 5000;
    ctx->config.enable_wal = true;
    ctx->config.enable_foreign_keys = true;
    ctx->config.cache_size = 10000;
    
    // Connect to database
    if (database_connect(ctx) != 0) {
        LOG_ERROR("Failed to connect to database");
        free(ctx);
        return NULL;
    }
    
    // Create schema
    if (database_create_schema(ctx) != 0) {
        LOG_ERROR("Failed to create database schema");
        database_cleanup(ctx);
        return NULL;
    }
    
    // Prepare statements
    if (database_prepare_statements(ctx) != 0) {
        LOG_ERROR("Failed to prepare database statements");
        database_cleanup(ctx);
        return NULL;
    }
    
    ctx->initialized = true;
    
    LOG_INFO("Database initialized successfully: %s", ctx->config.database_path);
    return ctx;
}

// Cleanup database context
void database_cleanup(database_context_t* ctx) {
    if (!ctx) return;
    
    LOG_INFO("Cleaning up database context");
    
    // Finalize prepared statements
    database_finalize_statements(ctx);
    
    // Close database connection
    database_disconnect(ctx);
    
    free(ctx);
}

// Connect to database
int database_connect(database_context_t* ctx) {
    if (!ctx) return -1;
    
    int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    int rc = sqlite3_open_v2(ctx->config.database_path, &ctx->db, flags, NULL);
    
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to open database: %s", sqlite3_errmsg(ctx->db));
        return -1;
    }
    
    // Configure database
    sqlite3_busy_timeout(ctx->db, ctx->config.busy_timeout);
    
    if (ctx->config.enable_wal) {
        sqlite3_exec(ctx->db, "PRAGMA journal_mode=WAL;", NULL, NULL, NULL);
    }
    
    if (ctx->config.enable_foreign_keys) {
        sqlite3_exec(ctx->db, "PRAGMA foreign_keys=ON;", NULL, NULL, NULL);
    }
    
    char cache_sql[256];
    snprintf(cache_sql, sizeof(cache_sql), "PRAGMA cache_size=-%d;", ctx->config.cache_size);
    sqlite3_exec(ctx->db, cache_sql, NULL, NULL, NULL);
    
    LOG_DEBUG("Database connected successfully");
    return 0;
}

// Disconnect from database
void database_disconnect(database_context_t* ctx) {
    if (!ctx || !ctx->db) return;
    
    sqlite3_close(ctx->db);
    ctx->db = NULL;
    
    LOG_DEBUG("Database disconnected");
}

// Create database schema
int database_create_schema(database_context_t* ctx) {
    if (!ctx || !ctx->db) return -1;
    
    char* err_msg = NULL;
    
    // Create tables
    int rc = sqlite3_exec(ctx->db, DATABASE_SCHEMA_SQL, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to create schema: %s", err_msg);
        sqlite3_free(err_msg);
        return -1;
    }
    
    // Create indexes
    rc = sqlite3_exec(ctx->db, DATABASE_INDEXES_SQL, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to create indexes: %s", err_msg);
        sqlite3_free(err_msg);
        return -1;
    }
    
    // Create triggers
    rc = sqlite3_exec(ctx->db, DATABASE_TRIGGERS_SQL, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to create triggers: %s", err_msg);
        sqlite3_free(err_msg);
        return -1;
    }
    
    // Insert schema version
    const char* version_sql = "INSERT OR REPLACE INTO schema_version (version) VALUES (1);";
    rc = sqlite3_exec(ctx->db, version_sql, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to set schema version: %s", err_msg);
        sqlite3_free(err_msg);
        return -1;
    }
    
    LOG_DEBUG("Database schema created successfully");
    return 0;
}

// Prepare statements
int database_prepare_statements(database_context_t* ctx) {
    if (!ctx || !ctx->db) return -1;
    
    // Prepare insert metric statement
    const char* insert_metric_sql = 
        "INSERT INTO metrics (metric_type, value, node_id, cell_id, timestamp) "
        "VALUES (?, ?, ?, ?, ?);";
    
    int rc = sqlite3_prepare_v2(ctx->db, insert_metric_sql, -1, &ctx->insert_metric_stmt, NULL);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare insert metric statement: %s", sqlite3_errmsg(ctx->db));
        return -1;
    }
    
    // Prepare insert anomaly statement
    const char* insert_anomaly_sql = 
        "INSERT INTO anomalies (metric_type, severity, threshold_value, actual_value, confidence, detected_at, description) "
        "VALUES (?, ?, ?, ?, ?, ?, ?);";
    
    rc = sqlite3_prepare_v2(ctx->db, insert_anomaly_sql, -1, &ctx->insert_anomaly_stmt, NULL);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare insert anomaly statement: %s", sqlite3_errmsg(ctx->db));
        return -1;
    }
    
    // Prepare insert recommendation statement
    const char* insert_recommendation_sql = 
        "INSERT INTO recommendations (type, node_id, cell_id, confidence, expected_improvement, generated_at, description, parameters) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?);";
    
    rc = sqlite3_prepare_v2(ctx->db, insert_recommendation_sql, -1, &ctx->insert_recommendation_stmt, NULL);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare insert recommendation statement: %s", sqlite3_errmsg(ctx->db));
        return -1;
    }
    
    // Prepare insert event statement
    const char* insert_event_sql = 
        "INSERT INTO events (event_type, node_id, subscription_id, timestamp, message, details) "
        "VALUES (?, ?, ?, ?, ?, ?);";
    
    rc = sqlite3_prepare_v2(ctx->db, insert_event_sql, -1, &ctx->insert_event_stmt, NULL);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare insert event statement: %s", sqlite3_errmsg(ctx->db));
        return -1;
    }
    
    LOG_DEBUG("Database statements prepared successfully");
    return 0;
}

// Finalize statements
void database_finalize_statements(database_context_t* ctx) {
    if (!ctx) return;
    
    if (ctx->insert_metric_stmt) {
        sqlite3_finalize(ctx->insert_metric_stmt);
        ctx->insert_metric_stmt = NULL;
    }
    
    if (ctx->insert_anomaly_stmt) {
        sqlite3_finalize(ctx->insert_anomaly_stmt);
        ctx->insert_anomaly_stmt = NULL;
    }
    
    if (ctx->insert_recommendation_stmt) {
        sqlite3_finalize(ctx->insert_recommendation_stmt);
        ctx->insert_recommendation_stmt = NULL;
    }
    
    if (ctx->insert_event_stmt) {
        sqlite3_finalize(ctx->insert_event_stmt);
        ctx->insert_event_stmt = NULL;
    }
    
    LOG_DEBUG("Database statements finalized");
}

// Insert metric
int database_insert_metric(database_context_t* ctx, const metric_data_t* metric) {
    if (!ctx || !ctx->db || !ctx->insert_metric_stmt || !metric) {
        return -1;
    }
    
    // Bind parameters
    sqlite3_bind_int(ctx->insert_metric_stmt, 1, metric->type);
    sqlite3_bind_double(ctx->insert_metric_stmt, 2, metric->value);
    sqlite3_bind_int(ctx->insert_metric_stmt, 3, metric->node_id);
    sqlite3_bind_int(ctx->insert_metric_stmt, 4, metric->cell_id);
    sqlite3_bind_int64(ctx->insert_metric_stmt, 5, metric->timestamp);
    
    // Execute statement
    int rc = sqlite3_step(ctx->insert_metric_stmt);
    
    // Reset statement
    sqlite3_reset(ctx->insert_metric_stmt);
    
    if (rc != SQLITE_DONE) {
        LOG_ERROR("Failed to insert metric: %s", sqlite3_errmsg(ctx->db));
        ctx->total_errors++;
        return -1;
    }
    
    ctx->total_inserts++;
    return 0;
}

// Insert anomaly
int database_insert_anomaly(database_context_t* ctx, const anomaly_result_t* anomaly) {
    if (!ctx || !ctx->db || !ctx->insert_anomaly_stmt || !anomaly) {
        return -1;
    }
    
    // Bind parameters
    sqlite3_bind_int(ctx->insert_anomaly_stmt, 1, anomaly->metric_type);
    sqlite3_bind_int(ctx->insert_anomaly_stmt, 2, anomaly->severity);
    sqlite3_bind_double(ctx->insert_anomaly_stmt, 3, anomaly->threshold_value);
    sqlite3_bind_double(ctx->insert_anomaly_stmt, 4, anomaly->actual_value);
    sqlite3_bind_double(ctx->insert_anomaly_stmt, 5, anomaly->confidence);
    sqlite3_bind_int64(ctx->insert_anomaly_stmt, 6, anomaly->detected_at);
    sqlite3_bind_text(ctx->insert_anomaly_stmt, 7, anomaly->description, -1, SQLITE_STATIC);
    
    // Execute statement
    int rc = sqlite3_step(ctx->insert_anomaly_stmt);
    
    // Reset statement
    sqlite3_reset(ctx->insert_anomaly_stmt);
    
    if (rc != SQLITE_DONE) {
        LOG_ERROR("Failed to insert anomaly: %s", sqlite3_errmsg(ctx->db));
        ctx->total_errors++;
        return -1;
    }
    
    ctx->total_inserts++;
    return 0;
}

// Insert recommendation
int database_insert_recommendation(database_context_t* ctx, const recommendation_result_t* recommendation) {
    if (!ctx || !ctx->db || !ctx->insert_recommendation_stmt || !recommendation) {
        return -1;
    }
    
    // Bind parameters
    sqlite3_bind_int(ctx->insert_recommendation_stmt, 1, recommendation->type);
    sqlite3_bind_int(ctx->insert_recommendation_stmt, 2, recommendation->node_id);
    sqlite3_bind_int(ctx->insert_recommendation_stmt, 3, recommendation->cell_id);
    sqlite3_bind_double(ctx->insert_recommendation_stmt, 4, recommendation->confidence);
    sqlite3_bind_double(ctx->insert_recommendation_stmt, 5, recommendation->expected_improvement);
    sqlite3_bind_int64(ctx->insert_recommendation_stmt, 6, recommendation->generated_at);
    sqlite3_bind_text(ctx->insert_recommendation_stmt, 7, recommendation->description, -1, SQLITE_STATIC);
    sqlite3_bind_text(ctx->insert_recommendation_stmt, 8, recommendation->parameters, -1, SQLITE_STATIC);
    
    // Execute statement
    int rc = sqlite3_step(ctx->insert_recommendation_stmt);
    
    // Reset statement
    sqlite3_reset(ctx->insert_recommendation_stmt);
    
    if (rc != SQLITE_DONE) {
        LOG_ERROR("Failed to insert recommendation: %s", sqlite3_errmsg(ctx->db));
        ctx->total_errors++;
        return -1;
    }
    
    ctx->total_inserts++;
    return 0;
}

// Log event
int database_log_event(database_context_t* ctx, event_type_t type, uint32_t node_id, uint32_t subscription_id, const char* message, const char* details) {
    if (!ctx || !ctx->db || !ctx->insert_event_stmt) {
        return -1;
    }
    
    event_data_t event = {
        .type = type,
        .node_id = node_id,
        .subscription_id = subscription_id,
        .timestamp = time(NULL)
    };
    
    // Copy message and details
    if (message) {
        strncpy(event.message, message, sizeof(event.message) - 1);
    }
    if (details) {
        strncpy(event.details, details, sizeof(event.details) - 1);
    }
    
    return database_insert_event(ctx, &event);
}

// Insert event
int database_insert_event(database_context_t* ctx, const event_data_t* event) {
    if (!ctx || !ctx->db || !ctx->insert_event_stmt || !event) {
        return -1;
    }
    
    // Bind parameters
    sqlite3_bind_int(ctx->insert_event_stmt, 1, event->type);
    sqlite3_bind_int(ctx->insert_event_stmt, 2, event->node_id);
    sqlite3_bind_int(ctx->insert_event_stmt, 3, event->subscription_id);
    sqlite3_bind_int64(ctx->insert_event_stmt, 4, event->timestamp);
    sqlite3_bind_text(ctx->insert_event_stmt, 5, event->message, -1, SQLITE_STATIC);
    sqlite3_bind_text(ctx->insert_event_stmt, 6, event->details, -1, SQLITE_STATIC);
    
    // Execute statement
    int rc = sqlite3_step(ctx->insert_event_stmt);
    
    // Reset statement
    sqlite3_reset(ctx->insert_event_stmt);
    
    if (rc != SQLITE_DONE) {
        LOG_ERROR("Failed to insert event: %s", sqlite3_errmsg(ctx->db));
        ctx->total_errors++;
        return -1;
    }
    
    ctx->total_inserts++;
    return 0;
}

// Get error message
const char* database_get_error_message(database_context_t* ctx) {
    if (!ctx || !ctx->db) {
        return "Database not initialized";
    }
    return sqlite3_errmsg(ctx->db);
}

// Get last error code
int database_get_last_error_code(database_context_t* ctx) {
    if (!ctx || !ctx->db) {
        return -1;
    }
    return sqlite3_errcode(ctx->db);
}

// Print performance statistics
void database_print_performance(const database_context_t* ctx) {
    if (!ctx) return;
    
    LOG_INFO("Database Performance:");
    LOG_INFO("  Total Inserts: %llu", (unsigned long long)ctx->total_inserts);
    LOG_INFO("  Total Queries: %llu", (unsigned long long)ctx->total_queries);
    LOG_INFO("  Total Errors: %llu", (unsigned long long)ctx->total_errors);
    LOG_INFO("  Database Path: %s", ctx->config.database_path);
    
    // Check database size
    if (ctx->db) {
        sqlite3_stmt* stmt;
        const char* size_sql = "SELECT page_count * page_size as size FROM pragma_page_count(), pragma_page_size();";
        
        if (sqlite3_prepare_v2(ctx->db, size_sql, -1, &stmt, NULL) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                int64_t size = sqlite3_column_int64(stmt, 0);
                LOG_INFO("  Database Size: %lld bytes (%.2f MB)", 
                        (long long)size, (double)size / (1024 * 1024));
            }
            sqlite3_finalize(stmt);
        }
    }
}

// Vacuum database
int database_vacuum(database_context_t* ctx) {
    if (!ctx || !ctx->db) return -1;
    
    LOG_INFO("Vacuuming database...");
    
    char* err_msg = NULL;
    int rc = sqlite3_exec(ctx->db, "VACUUM;", NULL, NULL, &err_msg);
    
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to vacuum database: %s", err_msg);
        sqlite3_free(err_msg);
        return -1;
    }
    
    LOG_INFO("Database vacuumed successfully");
    return 0;
}

// Analyze database
int database_analyze(database_context_t* ctx) {
    if (!ctx || !ctx->db) return -1;
    
    LOG_INFO("Analyzing database...");
    
    char* err_msg = NULL;
    int rc = sqlite3_exec(ctx->db, "ANALYZE;", NULL, NULL, &err_msg);
    
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to analyze database: %s", err_msg);
        sqlite3_free(err_msg);
        return -1;
    }
    
    LOG_INFO("Database analyzed successfully");
    return 0;
}

// Cleanup old data
int database_cleanup_old_data(database_context_t* ctx, int retention_days) {
    if (!ctx || !ctx->db) return -1;
    
    LOG_INFO("Cleaning up data older than %d days", retention_days);
    
    time_t cutoff_time = time(NULL) - (retention_days * 24 * 60 * 60);
    
    char sql[512];
    snprintf(sql, sizeof(sql), 
            "DELETE FROM metrics WHERE timestamp < %ld;"
            "DELETE FROM anomalies WHERE detected_at < %ld;"
            "DELETE FROM recommendations WHERE generated_at < %ld;"
            "DELETE FROM events WHERE timestamp < %ld;",
            cutoff_time, cutoff_time, cutoff_time, cutoff_time);
    
    char* err_msg = NULL;
    int rc = sqlite3_exec(ctx->db, sql, NULL, NULL, &err_msg);
    
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to cleanup old data: %s", err_msg);
        sqlite3_free(err_msg);
        return -1;
    }
    
    LOG_INFO("Old data cleanup completed");
    return 0;
}