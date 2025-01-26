#pragma once

class CSocket
{
    public:
        CSocket(SOCKET socket);
        ~CSocket();
        int read(char *buffer, int len);
        int write(const char *buffer, int len);
        void Close();
    private:
        SOCKET m_socket;
};

using MYSOCKET = std::shared_ptr<CSocket>;