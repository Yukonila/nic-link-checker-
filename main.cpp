#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/wireless.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>

using namespace std;

class NICMonitor {
    string name;
    int last_status = -1, last_speed = -1;
    int last_quality = -1;

    int getFd() { return socket(AF_INET, SOCK_DGRAM, 0); }

    // 检测是否为无线网卡 (SIOCGIWNAME 是检测无线扩展存在的最可靠方式)[citation:8]
    bool isWireless() {
        int fd = getFd();
        struct iwreq wrq;
        strcpy(wrq.ifr_name, name.c_str());
        bool ret = (ioctl(fd, SIOCGIWNAME, &wrq) == 0);
        close(fd);
        return ret;
    }

    // 有线网卡：物理连接状态
    bool isLinkUp() {
        int fd = getFd();
        struct ifreq ifr;
        struct ethtool_value edata = { ETHTOOL_GLINK };
        strcpy(ifr.ifr_name, name.c_str());
        ifr.ifr_data = (char*)&edata;
        bool ret = (ioctl(fd, SIOCETHTOOL, &ifr) == 0 && edata.data);
        close(fd);
        return ret;
    }

    // 有线网卡：速率
    int getSpeed() {
        int fd = getFd();
        struct ifreq ifr;
        struct ethtool_cmd edata = { ETHTOOL_GSET };
        strcpy(ifr.ifr_name, name.c_str());
        ifr.ifr_data = (char*)&edata;
        int speed = -1;
        if (ioctl(fd, SIOCETHTOOL, &ifr) == 0)
            speed = ethtool_cmd_speed(&edata);
        close(fd);
        return speed;
    }

    // WiFi：通过 SIOCGIWSTATS 获取信号质量[citation:6]
    // 返回: true=已连接, quality 为信号质量(0-100)
    bool getWifiStatus(int& quality, int& level_dbm) {
        int fd = getFd();
        struct iwreq wrq;
        struct iw_statistics stats;

        strcpy(wrq.ifr_name, name.c_str());
        wrq.u.data.pointer = &stats;
        wrq.u.data.length = sizeof(stats);

        if (ioctl(fd, SIOCGIWSTATS, &wrq) == 0) {
            // qual: 链路质量，范围通常 0-100
            quality = stats.qual.qual;
            // level: 信号电平，若支持 DBM 则为 dBm 值
            level_dbm = (signed char)stats.qual.level;
            close(fd);
            return (quality > 0);  // quality > 0 表示已连接
        }
        close(fd);
        quality = 0;
        level_dbm = 0;
        return false;
    }

    string formatWifiQuality(int qual, int dbm) {
        if (qual <= 0) return "未连接";
        string result;
        if (qual >= 70) result = "信号强";
        else if (qual >= 30) result = "信号中";
        else result = "信号弱";
        result += " (" + to_string(qual) + "%)";
        if (dbm < 0) result += " " + to_string(dbm) + "dBm";
        return result;
    }

    string formatSpeed(int s) {
        if (s <= 0) return "Unknown";
        if (s >= 1000) return "千兆(1Gbps)";
        if (s == 100) return "百兆(100Mbps)";
        return to_string(s) + "Mbps";
    }

public:
    NICMonitor(const string& n) : name(n) {}

    void run(int interval_ms = 1000) {
        bool wireless = isWireless();
        cout << "类型: " << (wireless ? "WiFi" : "有线") << endl;
        cout << "监控: " << name << " (Ctrl+C退出)" << endl;
        cout << "-----------------------------------" << endl;

        while (true) {
            if (wireless) {
                int quality, level;
                bool connected = getWifiStatus(quality, level);

                if (connected != last_status) {
                    if (connected) {
                        cout << "[已连接] " << name
                             << " - " << formatWifiQuality(quality, level) << endl;
                    } else {
                        cout << "[未连接] " << name << " - 未关联到AP" << endl;
                    }
                    last_status = connected;
                    last_quality = quality;
                }
                else if (connected && quality != last_quality) {
                    cout << "[信号] " << name << " -> "
                         << formatWifiQuality(quality, level) << endl;
                    last_quality = quality;
                }
            }
            else {  // 有线网卡
                bool up = isLinkUp();
                int speed = getSpeed();

                if (up != last_status) {
                    cout << (up ? "[UP] " : "[DOWN] ") << name;
                    if (up) cout << " - " << formatSpeed(speed);
                    cout << endl;
                    last_status = up;
                    last_speed = speed;
                }
                else if (up && speed != last_speed && speed > 0) {
                    cout << "[SPEED] " << name << " -> " << formatSpeed(speed) << endl;
                    last_speed = speed;
                }
            }
            usleep(interval_ms * 1000);
        }
    }
};

int main(int argc, char* argv[]) {
    string ifname = (argc > 1) ? argv[1] : "eth0";
    try {
        NICMonitor(ifname).run();
    } catch (exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    return 0;
}