/*
 * Analytics Tests for Smart Monitor xApp
 * 
 * Unit tests for the analytics module
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
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

// Test analytics initialization
int test_analytics_init() {
    printf("\n🧪 Testing Analytics Initialization...\n");
    
    analytics_context_t* ctx = analytics_init(NULL);
    TEST_ASSERT(ctx != NULL, "Analytics context should be created");
    TEST_ASSERT(ctx->config.window_size == 100, "Default window size should be 100");
    TEST_ASSERT(ctx->config.outlier_threshold == 2.0, "Default outlier threshold should be 2.0");
    
    analytics_cleanup(ctx);
    TEST_ASSERT(1, "Analytics cleanup should not crash");
    
    return 1;
}

// Test metric processing
int test_metric_processing() {
    printf("\n🧪 Testing Metric Processing...\n");
    
    analytics_context_t* ctx = analytics_init(NULL);
    TEST_ASSERT(ctx != NULL, "Analytics context should be created");
    
    // Add some test metrics
    for (int i = 0; i < 20; i++) {
        double value = 50.0 + i * 2.0;  // Linear increase
        int result = analytics_add_metric(ctx, METRIC_THROUGHPUT, value, 1, 1);
        TEST_ASSERT(result == 0, "Metric should be added successfully");
    }
    
    TEST_ASSERT(ctx->processed_metrics == 20, "Should have processed 20 metrics");
    
    // Check history
    metric_history_t* history = analytics_get_history(ctx, METRIC_THROUGHPUT);
    TEST_ASSERT(history != NULL, "History should be available");
    TEST_ASSERT(history->count == 20, "History should contain 20 entries");
    
    analytics_cleanup(ctx);
    return 1;
}

// Test statistical analysis
int test_statistical_analysis() {
    printf("\n🧪 Testing Statistical Analysis...\n");
    
    // Create test data
    metric_data_t data[10];
    for (int i = 0; i < 10; i++) {
        data[i].value = (double)(i + 1);  // Values 1-10
        data[i].timestamp = time(NULL);
    }
    
    stats_result_t stats = analytics_calculate_stats(data, 10);
    
    TEST_ASSERT(stats.mean == 5.5, "Mean should be 5.5");
    TEST_ASSERT(stats.min == 1.0, "Min should be 1.0");
    TEST_ASSERT(stats.max == 10.0, "Max should be 10.0");
    TEST_ASSERT(fabs(stats.std_dev - 2.87) < 0.1, "Standard deviation should be ~2.87");
    
    return 1;
}

// Test trend analysis
int test_trend_analysis() {
    printf("\n🧪 Testing Trend Analysis...\n");
    
    // Create increasing trend data
    metric_data_t data[10];
    for (int i = 0; i < 10; i++) {
        data[i].value = (double)i * 2.0;  // Increasing trend
        data[i].timestamp = time(NULL);
    }
    
    trend_result_t trend = analytics_calculate_trend(data, 10);
    
    TEST_ASSERT(trend.slope > 1.0, "Slope should be positive for increasing trend");
    TEST_ASSERT(trend.is_increasing, "Should detect increasing trend");
    TEST_ASSERT(!trend.is_decreasing, "Should not detect decreasing trend");
    
    return 1;
}

// Test anomaly detection
int test_anomaly_detection() {
    printf("\n🧪 Testing Anomaly Detection...\n");
    
    analytics_context_t* ctx = analytics_init(NULL);
    TEST_ASSERT(ctx != NULL, "Analytics context should be created");
    
    // Add normal metrics
    for (int i = 0; i < 15; i++) {
        analytics_add_metric(ctx, METRIC_THROUGHPUT, 50.0, 1, 1);
    }
    
    // Add an anomalous metric
    metric_data_t anomalous_metric = {
        .type = METRIC_THROUGHPUT,
        .value = 200.0,  // Much higher than normal
        .node_id = 1,
        .cell_id = 1,
        .timestamp = time(NULL)
    };
    
    anomaly_result_t anomaly = analytics_detect_anomaly(ctx, &anomalous_metric);
    TEST_ASSERT(anomaly.severity > ANOMALY_NONE, "Should detect anomaly");
    TEST_ASSERT(anomaly.actual_value == 200.0, "Should record actual value");
    
    analytics_cleanup(ctx);
    return 1;
}

// Test Z-score calculation
int test_z_score() {
    printf("\n🧪 Testing Z-Score Calculation...\n");
    
    double z_score = analytics_calculate_z_score(10.0, 5.0, 2.0);
    TEST_ASSERT(fabs(z_score - 2.5) < 0.01, "Z-score should be 2.5");
    
    bool is_outlier = analytics_is_outlier(z_score, 2.0);
    TEST_ASSERT(is_outlier, "Should detect outlier with z-score > 2.0");
    
    return 1;
}

// Test string conversions
int test_string_conversions() {
    printf("\n🧪 Testing String Conversions...\n");
    
    const char* metric_str = analytics_metric_type_to_string(METRIC_THROUGHPUT);
    TEST_ASSERT(strcmp(metric_str, "Throughput") == 0, "Should convert metric type to string");
    
    const char* severity_str = analytics_anomaly_severity_to_string(ANOMALY_CRITICAL);
    TEST_ASSERT(strcmp(severity_str, "Critical") == 0, "Should convert severity to string");
    
    const char* rec_str = analytics_recommendation_type_to_string(RECOMMENDATION_INCREASE_POWER);
    TEST_ASSERT(strcmp(rec_str, "Increase Power") == 0, "Should convert recommendation type to string");
    
    return 1;
}

// Main test function
int main() {
    printf("🚀 Starting Analytics Tests\n");
    printf("============================\n");
    
    int tests_passed = 0;
    int total_tests = 0;
    
    total_tests++; if (test_analytics_init()) tests_passed++;
    total_tests++; if (test_metric_processing()) tests_passed++;
    total_tests++; if (test_statistical_analysis()) tests_passed++;
    total_tests++; if (test_trend_analysis()) tests_passed++;
    total_tests++; if (test_anomaly_detection()) tests_passed++;
    total_tests++; if (test_z_score()) tests_passed++;
    total_tests++; if (test_string_conversions()) tests_passed++;
    
    printf("\n============================\n");
    printf("📊 Test Results: %d/%d passed\n", tests_passed, total_tests);
    
    if (tests_passed == total_tests) {
        printf("🎉 All analytics tests passed!\n");
        return 0;
    } else {
        printf("❌ Some analytics tests failed!\n");
        return 1;
    }
}