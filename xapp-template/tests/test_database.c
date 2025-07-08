/*
 * Database Tests for Smart Monitor xApp
 * 
 * Unit tests for the database module
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include "../include/database.h"
#include "../include/analytics.h"
#include "../include/utils.h"

#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("❌ FAILED: %s\n", message); \
            return 0; \
        } else { \
            printf("✅ PASSED: %s\n", message); \
        } \
    } while(0)

#define TEST_DB_PATH "/tmp/test_xapp.db"

// Test database initialization
int test_database_init() {
    printf("\n🧪 Testing Database Initialization...\n");
    
    // Remove test database if it exists
    unlink(TEST_DB_PATH);
    
    database_context_t* ctx = database_init(TEST_DB_PATH);
    TEST_ASSERT(ctx != NULL, "Database context should be created");
    TEST_ASSERT(ctx->initialized, "Database should be initialized");
    TEST_ASSERT(ctx->db != NULL, "SQLite connection should be established");
    
    database_cleanup(ctx);
    TEST_ASSERT(1, "Database cleanup should not crash");
    
    // Clean up
    unlink(TEST_DB_PATH);
    
    return 1;
}

// Test metric insertion
int test_metric_insertion() {
    printf("\n🧪 Testing Metric Insertion...\n");
    
    unlink(TEST_DB_PATH);
    
    database_context_t* ctx = database_init(TEST_DB_PATH);
    TEST_ASSERT(ctx != NULL, "Database context should be created");
    
    // Create test metric
    metric_data_t metric = {
        .type = METRIC_THROUGHPUT,
        .value = 100.5,
        .node_id = 1,
        .cell_id = 1,
        .timestamp = time(NULL)
    };
    
    int result = database_insert_metric(ctx, &metric);
    TEST_ASSERT(result == 0, "Metric insertion should succeed");
    TEST_ASSERT(ctx->total_inserts > 0, "Insert counter should be incremented");
    
    database_cleanup(ctx);
    unlink(TEST_DB_PATH);
    
    return 1;
}

// Test anomaly insertion
int test_anomaly_insertion() {
    printf("\n🧪 Testing Anomaly Insertion...\n");
    
    unlink(TEST_DB_PATH);
    
    database_context_t* ctx = database_init(TEST_DB_PATH);
    TEST_ASSERT(ctx != NULL, "Database context should be created");
    
    // Create test anomaly
    anomaly_result_t anomaly = {
        .metric_type = METRIC_LATENCY,
        .severity = ANOMALY_WARNING,
        .threshold_value = 50.0,
        .actual_value = 75.0,
        .confidence = 0.8,
        .detected_at = time(NULL)
    };
    strcpy(anomaly.description, "Test anomaly");
    
    int result = database_insert_anomaly(ctx, &anomaly);
    TEST_ASSERT(result == 0, "Anomaly insertion should succeed");
    
    database_cleanup(ctx);
    unlink(TEST_DB_PATH);
    
    return 1;
}

// Test recommendation insertion
int test_recommendation_insertion() {
    printf("\n🧪 Testing Recommendation Insertion...\n");
    
    unlink(TEST_DB_PATH);
    
    database_context_t* ctx = database_init(TEST_DB_PATH);
    TEST_ASSERT(ctx != NULL, "Database context should be created");
    
    // Create test recommendation
    recommendation_result_t recommendation = {
        .type = RECOMMENDATION_INCREASE_POWER,
        .node_id = 1,
        .cell_id = 1,
        .confidence = 0.9,
        .expected_improvement = 15.0,
        .generated_at = time(NULL)
    };
    strcpy(recommendation.description, "Test recommendation");
    strcpy(recommendation.parameters, "power=+3dB");
    
    int result = database_insert_recommendation(ctx, &recommendation);
    TEST_ASSERT(result == 0, "Recommendation insertion should succeed");
    
    database_cleanup(ctx);
    unlink(TEST_DB_PATH);
    
    return 1;
}

// Test event logging
int test_event_logging() {
    printf("\n🧪 Testing Event Logging...\n");
    
    unlink(TEST_DB_PATH);
    
    database_context_t* ctx = database_init(TEST_DB_PATH);
    TEST_ASSERT(ctx != NULL, "Database context should be created");
    
    int result = database_log_event(ctx, EVENT_XAPP_START, 0, 0, "Test message", "Test details");
    TEST_ASSERT(result == 0, "Event logging should succeed");
    
    database_cleanup(ctx);
    unlink(TEST_DB_PATH);
    
    return 1;
}

// Test multiple insertions
int test_multiple_insertions() {
    printf("\n🧪 Testing Multiple Insertions...\n");
    
    unlink(TEST_DB_PATH);
    
    database_context_t* ctx = database_init(TEST_DB_PATH);
    TEST_ASSERT(ctx != NULL, "Database context should be created");
    
    // Insert multiple metrics
    for (int i = 0; i < 100; i++) {
        metric_data_t metric = {
            .type = METRIC_THROUGHPUT,
            .value = 50.0 + i,
            .node_id = 1,
            .cell_id = 1,
            .timestamp = time(NULL) + i
        };
        
        int result = database_insert_metric(ctx, &metric);
        TEST_ASSERT(result == 0, "Multiple metric insertions should succeed");
    }
    
    TEST_ASSERT(ctx->total_inserts >= 100, "Should have inserted at least 100 records");
    
    database_cleanup(ctx);
    unlink(TEST_DB_PATH);
    
    return 1;
}

// Test error handling
int test_error_handling() {
    printf("\n🧪 Testing Error Handling...\n");
    
    // Test with NULL parameters
    int result = database_insert_metric(NULL, NULL);
    TEST_ASSERT(result != 0, "Should fail with NULL parameters");
    
    // Test with invalid database path
    database_context_t* ctx = database_init("/invalid/path/database.db");
    TEST_ASSERT(ctx == NULL, "Should fail with invalid path");
    
    return 1;
}

// Test string conversions
int test_string_conversions() {
    printf("\n🧪 Testing String Conversions...\n");
    
    const char* event_str = database_event_type_to_string(EVENT_XAPP_START);
    TEST_ASSERT(strcmp(event_str, "XAPP_START") == 0, "Should convert event type to string");
    
    return 1;
}

// Test database maintenance
int test_database_maintenance() {
    printf("\n🧪 Testing Database Maintenance...\n");
    
    unlink(TEST_DB_PATH);
    
    database_context_t* ctx = database_init(TEST_DB_PATH);
    TEST_ASSERT(ctx != NULL, "Database context should be created");
    
    // Add some data
    metric_data_t metric = {
        .type = METRIC_THROUGHPUT,
        .value = 100.0,
        .node_id = 1,
        .cell_id = 1,
        .timestamp = time(NULL)
    };
    database_insert_metric(ctx, &metric);
    
    // Test vacuum
    int result = database_vacuum(ctx);
    TEST_ASSERT(result == 0, "Database vacuum should succeed");
    
    // Test analyze
    result = database_analyze(ctx);
    TEST_ASSERT(result == 0, "Database analyze should succeed");
    
    database_cleanup(ctx);
    unlink(TEST_DB_PATH);
    
    return 1;
}

// Main test function
int main() {
    printf("🚀 Starting Database Tests\n");
    printf("===========================\n");
    
    // Initialize logging for tests
    utils_init_logging(NULL, LOG_LEVEL_ERROR);
    
    int tests_passed = 0;
    int total_tests = 0;
    
    total_tests++; if (test_database_init()) tests_passed++;
    total_tests++; if (test_metric_insertion()) tests_passed++;
    total_tests++; if (test_anomaly_insertion()) tests_passed++;
    total_tests++; if (test_recommendation_insertion()) tests_passed++;
    total_tests++; if (test_event_logging()) tests_passed++;
    total_tests++; if (test_multiple_insertions()) tests_passed++;
    total_tests++; if (test_error_handling()) tests_passed++;
    total_tests++; if (test_string_conversions()) tests_passed++;
    total_tests++; if (test_database_maintenance()) tests_passed++;
    
    printf("\n===========================\n");
    printf("📊 Test Results: %d/%d passed\n", tests_passed, total_tests);
    
    // Cleanup
    utils_cleanup_logging();
    unlink(TEST_DB_PATH);
    
    if (tests_passed == total_tests) {
        printf("🎉 All database tests passed!\n");
        return 0;
    } else {
        printf("❌ Some database tests failed!\n");
        return 1;
    }
}