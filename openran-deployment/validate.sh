#!/bin/bash

# Validation script for OpenRAN deployment
# This script performs basic validation of the deployment structure

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "🔍 Validating OpenRAN deployment structure..."

# Check main deployment script
if [[ -f "$SCRIPT_DIR/deploy.sh" && -x "$SCRIPT_DIR/deploy.sh" ]]; then
    echo "✅ Main deployment script found and executable"
else
    echo "❌ Main deployment script missing or not executable"
    exit 1
fi

# Check status script
if [[ -f "$SCRIPT_DIR/status.sh" && -x "$SCRIPT_DIR/status.sh" ]]; then
    echo "✅ Status script found and executable"
else
    echo "❌ Status script missing or not executable"
    exit 1
fi

# Check scripts directory
if [[ -d "$SCRIPT_DIR/scripts" ]]; then
    echo "✅ Scripts directory found"
    
    # Count scripts
    script_count=$(find "$SCRIPT_DIR/scripts" -name "*.sh" | wc -l)
    echo "📄 Found $script_count scripts"
    
    # Check each script is executable
    for script in "$SCRIPT_DIR/scripts"/*.sh; do
        if [[ -x "$script" ]]; then
            echo "✅ $(basename "$script") is executable"
        else
            echo "❌ $(basename "$script") is not executable"
            exit 1
        fi
    done
else
    echo "❌ Scripts directory not found"
    exit 1
fi

# Check configs directory
if [[ -d "$SCRIPT_DIR/configs" ]]; then
    echo "✅ Configs directory found"
else
    echo "❌ Configs directory not found"
    exit 1
fi

# Check logs directory
if [[ -d "$SCRIPT_DIR/logs" ]]; then
    echo "✅ Logs directory found"
else
    echo "❌ Logs directory not found"
    exit 1
fi

# Check documentation
if [[ -f "$SCRIPT_DIR/README.md" ]]; then
    echo "✅ Main README found"
else
    echo "❌ Main README not found"
    exit 1
fi

if [[ -d "$SCRIPT_DIR/docs" ]]; then
    echo "✅ Documentation directory found"
    
    if [[ -f "$SCRIPT_DIR/docs/README.md" ]]; then
        echo "✅ Detailed documentation found"
    else
        echo "❌ Detailed documentation not found"
        exit 1
    fi
    
    if [[ -f "$SCRIPT_DIR/docs/API_REFERENCE.md" ]]; then
        echo "✅ API reference found"
    else
        echo "❌ API reference not found"
        exit 1
    fi
else
    echo "❌ Documentation directory not found"
    exit 1
fi

# Check syntax of all shell scripts
echo "🔍 Validating shell script syntax..."
for script in "$SCRIPT_DIR"/*.sh "$SCRIPT_DIR/scripts"/*.sh "$SCRIPT_DIR/configs"/*.sh; do
    if [[ -f "$script" ]]; then
        if bash -n "$script" 2>/dev/null; then
            echo "✅ $(basename "$script") syntax OK"
        else
            echo "❌ $(basename "$script") syntax error"
            exit 1
        fi
    fi
done

# Check if directories have proper permissions
echo "🔒 Checking directory permissions..."
for dir in "$SCRIPT_DIR/logs" "$SCRIPT_DIR/configs"; do
    if [[ -w "$dir" ]]; then
        echo "✅ $dir is writable"
    else
        echo "❌ $dir is not writable"
        exit 1
    fi
done

echo ""
echo "🎉 OpenRAN deployment validation completed successfully!"
echo ""
echo "📋 Deployment Summary:"
echo "   • Main deployment script: deploy.sh"
echo "   • Status monitoring: status.sh"
echo "   • Supporting scripts: $(find "$SCRIPT_DIR/scripts" -name "*.sh" | wc -l) files"
echo "   • Configuration templates: configs/"
echo "   • Documentation: docs/"
echo "   • Log storage: logs/"
echo ""
echo "🚀 Ready to deploy! Run ./deploy.sh to start the deployment wizard."