#include "stdafx.h"
#include "ServerSide.h"

CServerSide::CServerSide(const std::string& bridgeName,const std::string& prefix ,int iPort)
    :m_acceptor(m_io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v6(), iPort))
{
    m_bRunning = false;
    m_strTunNamePrefix = prefix;
    m_iCurrentTunIndex = 0;
    m_strBridgeName = bridgeName;
}
CServerSide::~CServerSide()
{
    Stop();
}
void CServerSide::Start()
{
    m_bRunning = true;
    m_acceptor.listen();
    StartAccept();
    for (int i = 0; i < 1; i++)
    {
        m_Threads.push_back(std::thread([this]() {
            m_io_context.run();
        }));
    }
}
void CServerSide::Stop()
{
    m_bRunning = false;
    m_acceptor.close();
    for (auto &thread : m_Threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
}
void CServerSide::StartAccept()
{
    SOCKET socket = std::make_shared<boost::asio::ip::tcp::socket>(m_io_context);
    
    m_acceptor.async_accept(*socket, std::bind(&CServerSide::HandleAccept, this, socket, std::placeholders::_1));
}
void CServerSide::HandleAccept(SOCKET socket, const boost::system::error_code ec)
{
    if (ec)
    {
        m_bRunning = false;
        return;
    }
    std::lock_guard<std::mutex> lock(m_Mutex);
    boost::asio::socket_base::keep_alive option(true);
    socket->set_option(option);

    //Start to process this socket
    std::string strTunName = GetNextTunName();
    MYNIC nic = std::make_shared<CVirtualNic>();
    if (!nic->Create(strTunName))
    {
        RecycleTunName(strTunName);
        std::cout<<"failed to create nic"<<std::endl;
        return;
    }

    
    CRelay& relay = m_Relays.emplace_back();
    MYSOCKET sock = std::make_shared<CSocket>(socket);
    if (!relay.StartRelay(sock, nic))
    {
        RecycleTunName(strTunName);
        std::cout<<"failed to start relay"<<std::endl;
        return;
    }
    if(!m_strBridgeName.empty())
    {
        AddNicToBridge(strTunName, m_strBridgeName);
    }
    
    std::cout << "accept  a connection " << std::endl;
    StartAccept();
}

std::string CServerSide::GetNextTunName()
{
    std::string strTunName;
    if(m_AvailableTunNames.empty())
    {
        strTunName = m_strTunNamePrefix + std::to_string(m_iCurrentTunIndex++);
    }
    else
    {
        strTunName = m_AvailableTunNames.front();
        m_AvailableTunNames.pop_front();
    }
    return strTunName;
}

void CServerSide::RecycleTunName(const std::string &name)
{
    m_AvailableTunNames.push_back(name);
}

bool CServerSide::IsRunning()
{
    
    if(!m_bRunning)
    {
        return false;
    }
    std::lock_guard<std::mutex> lock(m_Mutex);
    std::list<CRelay>::iterator it = m_Relays.begin();
    while(it != m_Relays.end())
    {
        if(!it->IsRunning())
        {
            RecycleTunName(it->GetNicName());
            it = m_Relays.erase(it);
            continue;
        }
        it++;
    }
    return m_bRunning;
}

void  CServerSide::AddNicToBridge(const std::string &nicName, const std::string &bridgeName)
{
    std::cout<<"AddNicToBridge " << nicName << " " << bridgeName << std::endl;
    std::string strCommand = "brctl addif  " + bridgeName + "  " + nicName;
    std::system(strCommand.c_str());
}