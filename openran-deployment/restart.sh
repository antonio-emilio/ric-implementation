#!/bin/bash
echo "🔄 Restarting OpenRAN deployment..."
"$(dirname "$0")/stop.sh"
sleep 5
"$(dirname "$0")/deploy.sh" --restart
