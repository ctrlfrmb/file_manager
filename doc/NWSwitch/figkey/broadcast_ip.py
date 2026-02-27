#!/usr/bin/env python3
import socket
import time
import subprocess
import re

# 获取br0接口IP地址
try:
    output = subprocess.check_output("ip addr show br0", shell=True).decode()
    match = re.search(r'inet (\d+\.\d+\.\d+\.\d+)', output)
    if match:
        ip_address = match.group(1)
    else:
        print("无法获取br0接口IP地址，使用默认值")
        ip_address = "192.168.1.120"
except:
    ip_address = "192.168.1.120"

port = 45678
message = f"FIGKEY_BOARD_IP:{ip_address}".encode()
duration = 60  # 秒
end_time = time.time() + duration

print(f"开始广播IP地址: {ip_address}, 持续时间: {duration}秒...")

# 创建UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

# 发送广播
count = 0
while time.time() < end_time:
    try:
        sock.sendto(message, ('255.255.255.255', port))
        count += 1
        print(f"已发送广播 #{count}")
    except Exception as e:
        print(f"发送失败: {e}")
    time.sleep(6)

sock.close()
print(f"广播完成，共发送{count}次")
