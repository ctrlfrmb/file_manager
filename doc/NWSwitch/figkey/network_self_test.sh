#!/bin/bash

# 执行网络恢复脚本
echo "执行网络恢复脚本..."
/bin/bash /usr/local/bin/figkey/network_recovery.sh

# 检查并添加广播路由
echo "检查并添加广播路由..."
if ! /sbin/route -n | grep -q "255.255.255.255.*br0"; then
    echo "添加广播路由..."
    /sbin/route add -host 255.255.255.255 dev br0
else
    echo "广播路由已存在，无需添加"
fi

# 执行Python广播脚本
echo "启动IP广播..."
/usr/bin/python3 /usr/local/bin/figkey/broadcast_ip.py
