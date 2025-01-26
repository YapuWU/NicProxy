
// filepath: /home/yapu/nicproxy/CVirtualNic.cpp
#include "stdafx.h"
#include "CVirtualNic.h"

CVirtualNic::CVirtualNic()
{
    
}

CVirtualNic::~CVirtualNic()
{
    Close();
}
bool CVirtualNic::Create(const std::string &devName)
{
    dev_name = devName;
    tun_fd = createTunDevice(devName);
    if (tun_fd < 0)
    {
        std::cout << "Failed to create tun device" << std::endl;
        return false;
    }
    return true;
}
void CVirtualNic::Close()
{
    if (tun_fd >= 0)
    {
        close(tun_fd);
        tun_fd = -1;
    }
     std::string strCommand = "ip link delete " + dev_name;
    std::system(strCommand.c_str());
}
int CVirtualNic::createTunDevice(const std::string &devName)
{
    struct ifreq ifr;
    int fd, err;

    //delete the old one
    std::string strCommand = "ip link delete " + devName;
    std::system(strCommand.c_str());
    
    if ((fd = open("/dev/net/tun", O_RDWR)) < 0)
    {
        perror("Opening /dev/net/tun");
        return fd;
    }

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    strncpy(ifr.ifr_name, devName.c_str(), IFNAMSIZ);

    if ((err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0)
    {
        perror("ioctl(TUNSETIFF)");
        close(fd);
        return err;
    }

    strCommand = "ip link set " + devName + " up";
    std::system(strCommand.c_str());

    return fd;
}

int CVirtualNic::read(char *buffer, int len)
{
    return ::read(tun_fd, buffer, len);
}

int CVirtualNic::write(const char *buffer, int len)
{
    return ::write(tun_fd, buffer, len);
}