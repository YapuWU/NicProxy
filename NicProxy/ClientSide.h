#pragma once
#include "Relay.h"
using namespace boost;
class CClientSide
{
    public:
        CClientSide(const std::string& strBridgeName, const std::string& tunNicName,int iPort);
        ~CClientSide();
        bool isRunning();
        bool Start();

         bool GetUSBNicName(std::string &strUsbNicName,const std::string& strPrefix="usbnet");
    private:
    std::string ReadCommandResult(const std::string& strCommand,const std::string& strSkipStart);
   
    bool GetGatewayIPOfNic(const std::string& nicName,std::string& strGatewayIP);
    std::string GetIPOfNic(const std::string& nicName);

    void AddNicToBridge(const std::string &nicName, const std::string &bridgeName);
    void RequestDHCP(const std::string &nicName,std::string& strGatewayIP);
    std::string m_strBridgeName;
    std::string m_strTunName;
    int m_iPort;
    std::atomic_bool m_bRunning;

    boost::asio::io_context m_io_context;

    CRelay m_Relay;
};