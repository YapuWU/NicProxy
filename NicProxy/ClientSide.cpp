#include "stdafx.h"

#include "ClientSide.h"

CClientSide::CClientSide(const std::string& bridgeName,const std::string& tunNicName ,int iPort)
{
    m_bRunning = false;
    m_strTunName = tunNicName;
    m_strBridgeName = bridgeName;
    m_iPort = iPort;
}

CClientSide::~CClientSide()
{
   
}

bool CClientSide::Start()
{
    std::string strNicName;
    std::string strGatewayIP;
    m_Relay.StopRelay();
    //Don't start until we get the nic name
    while(!GetUSBNicName(strNicName))
    {
        std::cout<<"Waiting for USB NIC"<<std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    std::cout<<"Found USB Nic " << strNicName << std::endl;
    //RequestDHCP(strNicName,strGatewayIP);
    GetGatewayIPOfNic(strNicName,strGatewayIP);
    std::cout<<"Gateway IP " << strGatewayIP << std::endl;
    SOCKET socket = std::make_shared<boost::asio::ip::tcp::socket>(m_io_context);
    boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string(strGatewayIP), m_iPort);
    boost::system::error_code ec;

    
    bool bConnected = false;
    for(int i=0;i<5;i++)
    {
        std::cout<<"Connecting to " << ep.address().to_string() << ":" << ep.port() << std::endl;
        socket->connect(ep,ec);
        if(!ec)
        {
            boost::asio::socket_base::keep_alive option(true);
            socket->set_option(option);
            bConnected = true;
            break;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    if (!bConnected)
    {
        return false;
    }
    
    std::cout<<"Connected to " << ep.address().to_string() << ":" << ep.port() << std::endl;
    MYNIC nic = std::make_shared<CVirtualNic>();
    nic->Create(m_strTunName);
    
    

    MYSOCKET sock = std::make_shared<CSocket>(socket);
    m_Relay.StartRelay(sock, nic);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    if(!m_strBridgeName.empty())
    {
        AddNicToBridge(m_strTunName, m_strBridgeName);
    }
    return true;
}
bool CClientSide::isRunning()
{
   
    return m_Relay.IsRunning();
}
std::string CClientSide::ReadCommandResult(const std::string& strCommand,const std::string& strSkipStart)
{
    char cJson[2048] = { 0 };
    std::string strJson;
    std::string strLine;
    FILE* pFile = popen(strCommand.c_str(), "r");
    while (fgets(cJson, sizeof(cJson), pFile) != NULL)
    {
        strLine = cJson;
        if(strLine.starts_with(strSkipStart) && strSkipStart.length() > 0)
        {
            continue;
        }    
        strJson += strLine;
    }
    pclose(pFile);
    return strJson;
}

bool CClientSide::GetUSBNicName(std::string &strUsbNicName,const std::string& strPrefix)
{
    char cJson[4096] = { 0 };
    strUsbNicName.clear();
    std::string strJson = ReadCommandResult("ip -j -p link show", "");
    json::value j = json::parse(strJson);
    if (!j.is_array())
    {
        return false;
    }
    json::array& arr = j.as_array();
    for(auto& item : arr)
    {
        json::object& obj = item.as_object();
        if(obj.contains("ifname"))
        {
            json::value& v = obj["ifname"];
            if(v.is_string())
            {
                if(v.as_string().starts_with(strPrefix))
                {
                    strUsbNicName = v.as_string();
                    break;
                }
            }
        }
    }
    // char cJson[2048] = { 0 };
    // std::string strJson = ReadCommandResult("lshw -json -C network", "WARNING");
	// std::string strCommand = "lshw -json -C network";
    // strUsbNicName.clear();
    // json::value j = json::parse(strJson);
    // if (!j.is_array())   
    // {
    //     return false;
    // }
    
	// json::array& arr = j.as_array();
    // for (auto &item : arr)
    // {
    //     json::object &obj = item.as_object();
    //     if (obj.contains("logicalname"))
    //     {
    //         json::value v = obj["logicalname"];
    //         if(!obj["logicalname"].is_string())
    //         {
    //             continue;
    //         }
    //         if (obj["logicalname"].as_string().starts_with(strPrefix))
    //         {
    //              strUsbNicName = obj["logicalname"].as_string();
    //              break;
    //         }
    //     }
    // }
    //Start to process Json
    return strUsbNicName.empty() ? false : true;
}

std::string getIpAddressForNic(const std::string& nicName) {
    std::ifstream arpFile("/proc/net/arp");
    if (!arpFile.is_open()) {
        std::cerr << "Failed to open /proc/net/arp" << std::endl;
        return "";
    }

    std::string line;
    std::getline(arpFile, line); // Skip the header line

    while (std::getline(arpFile, line)) {
        std::istringstream iss(line);
        std::string ip, hw_type, flags, mac, mask, device;
        iss >> ip >> hw_type >> flags >> mac >> mask >> device;
        if (device == nicName) {
            return ip;
        }
    }

    return "";
}
bool CClientSide::GetGatewayIPOfNic(const std::string& nicName,std::string& strGatewayIP)
{
    strGatewayIP.clear();
    std::string nicIp = getIpAddressForNic(nicName);
    if (nicIp.empty()) {
        std::cerr << "Failed to get IP address for " << nicName << std::endl;
        return false;
    }
    std::string strCmd = "ip -j -p route show dev " + nicName;

    std::string strJson = ReadCommandResult(strCmd, "");
    json::value j = json::parse(strJson);
    if (!j.is_array())
    {
        return false;
    }
    json::array& arr = j.as_array();
    for(auto& item : arr)
    {
        json::object& obj = item.as_object();
        if(obj.contains("gateway"))
        {
            json::value& v = obj["gateway"];
            if(v.is_string())
            {
                strGatewayIP = v.as_string();
                return true;
            }
        }
    }

    return true;
}

std::string CClientSide::GetIPOfNic(const std::string& nicName)
{
    std::string strCommand = "ip -j -p -4 address show dev" + nicName;
    std::string strJson = ReadCommandResult(strCommand, "");
    json::value j = json::parse(strJson);
    if (!j.is_array())
    {
        return "";
    }
    json::object& oneNic = j.as_array().at(0).as_object();
    if(oneNic.contains("addr_info"))
    {
        return "";
    }
    json::value& addrArr= oneNic["addr_info"];
    if(!addrArr.is_array())
    {
        return "";
    }

    for(auto& addr : addrArr.as_array())
    {
        json::object& addrObj = addr.as_object();
        json::value& addrValue = addrObj["local"];
        if(addrValue.is_string())
        {
            std::string strSkipIp = "10.10.1.10";
            if(addrValue.as_string().starts_with(strSkipIp))
            {
                continue;
            }
            return addrValue.as_string().c_str();
        }
    }


    return "";
}
void  CClientSide::AddNicToBridge(const std::string &nicName, const std::string &bridgeName)
{
    std::cout<<"AddNicToBridge " << nicName << " " << bridgeName << std::endl;
    std::string strCommand = "ip link set  " + nicName + "  up";
    std::system(strCommand.c_str());
    strCommand = "brctl addif  " + bridgeName + "  " + nicName;
    std::system(strCommand.c_str());
}
void CClientSide::RequestDHCP(const std::string &nicName,std::string& strGatewayIP)
{
    std::string strCommand = "dhclient " + nicName;
    std::system(strCommand.c_str());
    std::this_thread::sleep_for(std::chrono::seconds(3));

    strGatewayIP = "192.168.0.10";
}