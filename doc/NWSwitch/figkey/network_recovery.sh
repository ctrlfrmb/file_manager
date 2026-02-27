#!/bin/bash

# Network recovery script - optimized version

log() {
    echo "$(date '+%Y-%m-%d %H:%M:%S') - $1"
}

log "Network recovery script started"

# 检查br0是否有IP
has_ip() {
    ip addr show br0 2>/dev/null | grep -q "inet "
}

# 如果已经有IP，直接退出
if has_ip; then
    log "br0 already has IP address, exiting"
    exit 0
fi

log "br0 has no IP address, waiting with backoff strategy..."

# 递增等待：2, 5, 8秒 (总共15秒，与原来相同)
for wait_time in 2 5 8; do
    log "Waiting ${wait_time}s..."
    sleep $wait_time
    
    if has_ip; then
        log "br0 now has IP address, no restore needed"
        exit 0
    fi
done

log "br0 still has no IP after waiting, restoring default configuration"

# 恢复默认配置的代码...
cat > /etc/netplan/01-netcfg.yaml << 'EOF'
# Network config by NWSwitchIPConfig
network:
  version: 2
  renderer: networkd
  ethernets:
    eth0:
      dhcp4: no
      dhcp6: no
      optional: true
  bridges:
    br0:
      interfaces:
        - eth0
      addresses:
        - 192.168.1.120/24
      parameters:
        stp: true
        forward-delay: 4
EOF

chmod 600 /etc/netplan/01-netcfg.yaml
netplan apply

log "Default network configuration (192.168.1.120) has been restored"
exit 0
