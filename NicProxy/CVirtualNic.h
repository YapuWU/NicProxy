#pragma once

#include <string>
#include <stdexcept>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <cstring>

class CVirtualNic
{
public:
    CVirtualNic();
    ~CVirtualNic();

    bool Create(const std::string &devName);
    int read(char *buffer, int len);
    int write(const char *buffer, int len);

    void Close();
    std::string getDevName() { return dev_name; }
private:
    int tun_fd;
    std::string dev_name;
    int createTunDevice(const std::string &devName);
    
};

using MYNIC = std::shared_ptr<CVirtualNic>;