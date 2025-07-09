#!/bin/bash
echo "🛑 Stopping OpenRAN deployment..."
source "$(dirname "$0")/configs/deployment_config.env"
docker-compose -f "$(dirname "$0")/configs/docker-compose.yml" down 2>/dev/null || true
pkill -f "nr-softmodem\|nr-uesoftmodem\|nearRT-RIC\|gnb\|srsue" 2>/dev/null || true
echo "✅ Deployment stopped"
