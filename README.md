# NIC Link Status Checker

Linux C++ 程序，用于实时监控指定网卡的物理连接状态（Link Up/Down）和速率（百兆/千兆）。

## 功能特性

- ✅ **有线网卡**：检测连接状态 + 实时速率（100Mbps / 1000Mbps）
- ✅ **无线网卡**：检测连接状态 + 信号质量（百分比 + dBm）
- ✅ **实时监控**：循环检测，状态变化时立即输出
- ✅ **优雅退出**：支持 Ctrl+C 终止

## 编译运行

```bash
mkdir build && cd build
cmake ..
make
./Linktext <网卡名>  # 例如：./Linktext eno1 或 ./Linktext wlan0
```

## 查看网卡编号

运行以下命令查看系统中的网卡列表：

```bash
ls /sys/class/net
```

输出示例：
```
eno1  lo  Meta  virbr0  wlan0
```

其中 `eno1` 和 `wlan0` 是物理网卡（`lo` 是回环，`virbr0` 是虚拟网桥）。

## 示例输出

**有线网卡（已连接）**：
```
[UP] eno1 - 千兆(1Gbps)
```

**有线网卡（未连接）**：
```
[DOWN] eno1
```

**无线网卡（已连接）**：
```
[已连接] wlan0 - 信号中 (67%) -43dBm
```

**无线网卡（未连接）**：
```
[未连接] wlan0 - 未关联到AP
```

## 技术实现

本项目使用 **`ioctl` 系统调用** 实现网卡状态检测。

### 核心接口

| 功能 | ioctl 命令 | 说明 |
|------|------------|------|
| 有线网卡连接状态 | `SIOCETHTOOL` + `ETHTOOL_GLINK` | 通过 ethtool 接口获取 link up/down |
| 有线网卡速率 | `SIOCETHTOOL` + `ETHTOOL_GSET` | 获取当前速率（100Mbps/1000Mbps） |
| 无线网卡检测 | `SIOCGIWNAME` | 检测无线扩展是否存在 |
| 无线网卡信号质量 | `SIOCGIWSTATS` | 获取信号强度（quality/level） |

### 依赖

- C++11
- Linux 内核头文件（`net/if.h`, `linux/ethtool.h`, `linux/wireless.h` 等）

---

**作者**：Yukonila  
**创建时间**：2026-05-09