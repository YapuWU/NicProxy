#include "stdafx.h"
#include "CSocket.h"

CSocket::CSocket(SOCKET socket)
    : m_socket(socket)
{
   
}   
CSocket::~CSocket()
{
    Close(); 
}

int CSocket::read(char *buffer, int len)
{
    std::size_t ret = boost::asio::read(*m_socket, boost::asio::buffer(buffer, len), boost::asio::transfer_exactly(len));
     
    return ret;
}
int CSocket::write(const char *buffer, int len)
{
    std::size_t ret;

    ret = boost::asio::write(*m_socket, boost::asio::buffer(buffer, len), boost::asio::transfer_exactly(len));

  
    
    return ret;
}
void CSocket::Close()
{
    if(m_socket->is_open())
    {
        m_socket->close();
    }
}