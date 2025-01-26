#pragma once
#include "CVirtualNic.h"
#include "CSocket.h"
class CRelay
{
public:
   CRelay();
    ~CRelay();
    bool StartRelay(MYSOCKET socket, MYNIC nic);
    bool IsRunning() { return m_bRunning; }
    void StopRelay();
    std::string GetNicName() { return m_nic->getDevName(); }
private:
    MYSOCKET m_socket;
    MYNIC m_nic;
    std::atomic_bool m_bRunning;

    std::list<std::thread> m_threads;

    void RelayToNic();
    void RelayToSocket();
};