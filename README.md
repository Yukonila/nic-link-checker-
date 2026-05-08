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

## 示例输出

**有线网卡（已连接）**：
```
[UP] eno1 - 千兆(1Gbps)
```

**无线网卡（已连接）**：
```
[已连接] wlan0 - 信号中 (67%) -43dBm
```

## 技术实现

- 使用 `ioctl` + `ethtool` 获取有线网卡状态
- 使用 `iwlib` 获取无线网卡信号质量
- 支持自动识别网卡类型（有线/无线）

## 依赖

- C++11
- Linux 内核头文件（`net/if.h`, `linux/ethtool.h` 等）

---

**作者**：Koni  
**创建时间**：2026-05-09
