#pragma once

#include "Relay.h"

class CServerSide
{
    public:
        CServerSide(const std::string& bridgeName,const std::string& prefix ,int iPort);
        ~CServerSide();
        void Start();
        void Stop();
        bool IsRunning();
    private:
    std::atomic_bool m_bRunning;
    boost::asio::io_context m_io_context;
    boost::asio::ip::tcp::acceptor m_acceptor;

    std::list<std::thread> m_Threads;

    void StartAccept();
    void HandleAccept(SOCKET socket, const boost::system::error_code ec);
    std::string m_strBridgeName;
    std::string m_strTunNamePrefix;
    int         m_iCurrentTunIndex;
    std::list<std::string> m_AvailableTunNames;

    std::string GetNextTunName();
    void RecycleTunName(const std::string &name);
    void AddNicToBridge(const std::string &nicName, const std::string &bridgeName);
    std::list<CRelay> m_Relays;

    std::mutex m_Mutex;
};