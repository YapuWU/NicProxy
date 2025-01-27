#include "stdafx.h"
#include "Relay.h"

CRelay::CRelay()
{
    m_bRunning = false;
}
CRelay::~CRelay()
{
    StopRelay();
}
void CRelay::StopRelay()
{
    m_bRunning = false;
    try
    {
        if(m_socket)
            m_socket->Close();
    }
    catch(const std::exception& )
    {
        
    }

    try
    {
        if(m_nic)
            m_nic->Close();
    }
    catch(const std::exception& )
    {
        
    }

    for (auto &thread : m_threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
    m_threads.clear();
    m_socket = nullptr;
    m_nic = nullptr;
}
bool CRelay::StartRelay(MYSOCKET socket, MYNIC nic)
{
    m_socket = socket;
    m_nic = nic;
    m_bRunning = true;

    m_threads.push_back(std::thread(&CRelay::RelayToNic, this));
    m_threads.push_back(std::thread(&CRelay::RelayToSocket, this));
    std::cout<<"Relay started" << std::endl;
    return true;
}
void CRelay::RelayToSocket()
{
    char buffer[2048];
    uint32_t iDataLen = 0;
    try
    {
        while (m_bRunning)
        {
            int len = m_socket->read((char*)&iDataLen, sizeof(uint32_t));

			iDataLen = ntohl(iDataLen);
            len = m_socket->read(buffer,(int)iDataLen);
            if (len != (int)iDataLen)
            {
				std::cout << "Failed to read data from socket len=" <<len <<"  expecting " << iDataLen << std::endl;
                break;
            }
            m_nic->write(buffer, len);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
    try
    {
        m_nic->Close();
        m_socket->Close();
    }
    catch(const std::exception& )
    {
        
    }
    std::cout<<"RelayToSocket exit" << std::endl;
    m_bRunning = false;
}
void CRelay::RelayToNic()
{
    char buffer[2048];
    try
    {
        while (m_bRunning)
        {
            int len = m_nic->read(buffer, sizeof(buffer));
            if (len < 0)
            {
				std::cout << "Failed to read data from nic len="<<len << std::endl;
                break;
            }
            uint32_t iDataLen = len;
			iDataLen = htonl(iDataLen);
            m_socket->write((char*)&iDataLen, sizeof(uint32_t));
            m_socket->write(buffer, len);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }


     try
    {
        m_socket->Close();
        m_nic->Close();
    }
    catch(const std::exception& )
    {
        
    }
    std::cout<<"RelayToNic exit" << std::endl;
    m_bRunning = false;
}