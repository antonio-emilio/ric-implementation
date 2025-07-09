#!/bin/bash
echo "🧹 Cleaning OpenRAN deployment..."
"$(dirname "$0")/stop.sh"
docker system prune -f
for i in {1..10}; do
    sudo ip netns del ue$i 2>/dev/null || true
done
rm -rf "$(dirname "$0")/logs/*"
echo "✅ Deployment cleaned"
