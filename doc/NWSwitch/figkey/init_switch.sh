#!/bin/bash
# 设置脚本权限
chmod -R 755 /usr/local/bin/figkey

# 安装nfttables
dpkg -i /usr/local/bin/figkey/nftables_1.0.2-1ubuntu3_arm64.deb
nft --version

# 拷贝网络优化配置文件
cp -f /usr/local/bin/figkey/sysctl.conf /etc/sysctl.conf
# 应用网络配置
sysctl -p

# 配置网络自检服务
ln -sf /usr/local/bin/figkey/network_self_test.service /etc/systemd/system/
systemctl daemon-reload
systemctl enable network_self_test.service
systemctl start network_self_test.service

# 配置iperf客户端守护进程
# ln -sf /usr/local/bin/figkey/iperf_client_daemon.service /etc/systemd/system/
# systemctl daemon-reload
# systemctl enable iperf_client_daemon.service
# systemctl start iperf_client_daemon.service

# 移除iperf客户端守护进程
systemctl stop iperf_client_daemon.service
systemctl disable iperf_client_daemon.service
rm -f /etc/systemd/system/iperf_client_daemon.service
sudo systemctl daemon-reload
rm -f /usr/local/bin/figkey/iperf_client_daemon.service
rm -f /usr/local/bin/figkey/iperf_client_manager.py


