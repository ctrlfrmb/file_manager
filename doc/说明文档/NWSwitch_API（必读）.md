# 丰柯网络交换机 API 说明文档

> 版本：v2.1.0 | 作者：leiwei | 日期：2025-06-09

## 📖 简介

丰柯网络交换机API提供高性能的交换机通信接口，支持命令传输、结果接收和连接管理。

**主要特点**：安全连接、命令执行、超时控制、多设备支持、跨平台兼容

## 📁 目录结构

```
nwswitch_api/
├── README.pdf                 # 说明文档
├── NWSwitchApiReleaseNote.txt # 版本变更记录
├── NWSwitchClient.py          # Python API封装
├── NWSwitchClientTest.py      # Python测试工具
├── include/nwswitchapi.h      # C/C++ 头文件
└── lib/
    ├── Winx64/                # 64位库文件
    │   └── nwswitch_api.dll
    │   └── nwswitch_api.lib
    └── Winx86/                # 32位库文件
        └── nwswitch_api.dll
        └── nwswitch_api.lib
```

注意：运行时需要拷贝依赖库，如Qt/SSH库

## 🚀 快速开始

### Python测试（推荐新手）

1. **启动测试工具**
   ```bash
   python NWSwitchClientTest.py
   # 64位python环境启动测试：
   py -3-64 NWSwitchClientTest.py
   # 32位python环境启动测试：
   py -3-32 NWSwitchClientTest.py
   ```
   
2. **基本操作**
   ```bash
   >>> --connect 192.168.1.120/1     # 连接交换机
   >>> --send "ls -la /home"         # 发送命令
   >>> --autoRead on                 # 开启自动读取
   >>> --read 5000                   # 手动读取结果
   >>> --disconnect                  # 断开连接
   >>> --help                        # 查看帮助
   >>> --exit                        # 退出
   ```

### C/C++ 开发

1. **包含头文件**
   ```cpp
   #include "nwswitchapi.h"
   #pragma comment(lib, "nwswitch_api.lib")
   ```

2. **基本使用**
   
   ```cpp
   // 连接交换机
   int instance = NWSwitchConnect("192.168.1.120/1");
   if (instance > 0) {
       // 发送命令
       NWSwitchSendCmd(instance, "ls -la /home");
   
       // 接收结果
       char buffer[4096] = {0};
       NWSwitchReceiveCmdResult(instance, buffer, sizeof(buffer), 5000);
   
       // 关闭连接
       NWSwitchClose(instance);
   } 
   ```

## 🌐 常用交换机命令

```bash
# 查看当前目录
ls -l

# 显示系统信息
uname -a

# 网络连通性检查
ping 192.168.1.120
```

## ❗ 注意事项

- **IP格式**：必须使用 `IP地址/DUT索引` 格式，如 `192.168.1.120/1`
- **Python架构**：确保Python位数与DLL一致（推荐64位）
- **日志**：仅调试时开启，正常使用关闭节省资源
- **权限**：确保有交换机管理权限

## 🔍 故障排除

| 问题 | 解决方案 |
|------|----------|
| 连接失败 | 检查IP地址、网络连通性、SSH服务 |
| DLL加载失败 | 确认Python架构，安装VC++运行时 |
| 命令超时 | 增加超时时间，检查网络延迟 |
| 认证失败 | 验证用户名密码，检查账户权限 |

## 📞 技术支持

- **邮箱**: wei.lei@figkey.com
- **官网**: https://www.figkey.com

## 📄 版权声明

版权所有 © 2024 丰柯科技。保留所有权利。
