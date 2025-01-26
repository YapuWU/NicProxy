#include "stdafx.h"

#include "CVirtualNic.h"

#include "Relay.h"
using namespace boost;

std::string ReadCommandResult(const std::string& strCommand,const std::string& strSkipStart)
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


bool ReadHardwareConfig(std::string &strUsbNicName)
{
    char cJson[2048] = { 0 };
    std::string strJson = ReadCommandResult("lshw -json -C network", "WARNING");
	std::string strCommand = "lshw -json -C network";
    strUsbNicName.clear();
    json::value j = json::parse(strJson);
    if (!j.is_array())   
    {
        return false;
    }
    
	json::array& arr = j.as_array();
    for (auto &item : arr)
    {
        json::object &obj = item.as_object();
        if (obj.contains("logicalname"))
        {
            json::value v = obj["logicalname"];
            bool bRet = obj["logicalname"].is_string();
            if(!bRet)
            {
                continue;
            }
            if (obj["logicalname"].as_string().starts_with("enx"))
            {
                 strUsbNicName = obj["logicalname"].as_string();
                 break;
            }
        }
    }
    //Start to process Json
    return true;
}



int main(int argc, char *argv[])
{
    std::string strCommand;
    std::string strUsbNicName;

    std::string strVirtualNicName = "tap0";
    CVirtualNic nic(strVirtualNicName);

    std::string strGatewayIP;

    

    //Enable virtual nic;
    strCommand = "ip link set " + strVirtualNicName + " up";
    std::system(strCommand.c_str());

    

    while(true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        bool bRestart = false;
        // get usb nic name
        if(!ReadHardwareConfig(strUsbNicName))
        {
            continue;
        }
        if(strUsbNicName.empty())
        {
            continue;
        }

        //Enable usb nic;
        strCommand = "ip link set " + strUsbNicName + " up";
        std::cout<<strCommand<<std::endl;
        std::system(strCommand.c_str());

        //get ip address by using dhcp
        strCommand = "dhclient " + strUsbNicName;
        std::cout<<strCommand<<std::endl;
        std::system(strCommand.c_str());

        int iTry = 0;
        //Start To Read 
        while(true)
        {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            strGatewayIP = getIpAddressForNic(strUsbNicName);
            if(strGatewayIP.empty() && ++iTry>5)
            {
                break;
            }
            iTry = 0;
            strGatewayIP = "192.168.0.218";
            CRelay relay(strGatewayIP, 3000);
            if(!relay.Start(nic))
            {
                continue;
            }
            relay.Start(nic);
            while(relay.IsRunning())
            {
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }
        }
        
    }


    return 0;
}