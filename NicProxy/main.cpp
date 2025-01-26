#include "stdafx.h"
#include "ServerSide.h"
#include "ClientSide.h"
int main(int argc, char *argv[])
{
    int iPort = 3000;
    std::string strBridgeName = "";
    std::string strTunName = "tun";

    po::options_description desc("Allowed options");

    desc.add_options()
        ("help", "produce help message")
        ("server", "run as server")
        ("client", "run as client")
        ("bridge", po::value<std::string>(&strBridgeName)->default_value(""), "set bridge name")
        ("tun", po::value<std::string>(&strTunName)->default_value("tap"), "set tun name")
        ("port", po::value<int>(&iPort)->default_value(3000), "set port number")
        ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    if(vm.count("help"))
    {
        std::cout<<desc<<std::endl;
    }
    if (vm.count("server"))
    {
        CServerSide server(strBridgeName, strTunName, iPort);
        server.Start();

        while (server.IsRunning())
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    else if (vm.count("client"))
    {
        CClientSide client(strBridgeName, strTunName, iPort);
        while(true)
        {
            if(client.isRunning())
            {
                std::this_thread::sleep_for(std::chrono::seconds(10));
                std::string strNic;
                if(client.GetUSBNicName(strNic))
                {
                    continue;
                }


            }
            client.Start();
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
        
    }
    else
    {
        std::cout<<"Please specify server or client"<<std::endl;
    }
    std::cout << "Hello World!\n";
    return 0;
}