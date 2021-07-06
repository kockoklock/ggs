#pragma once
#include <iostream> // printf
#include <string> // std::string
#include <cstring> // memset
#include <vector> // std::vector

#include <sys/socket.h> // For socket functions
#include <netinet/in.h> // For sockaddr_in
#include <unistd.h> // For read

#define rcast(type, thing) reinterpret_cast<type>(thing)
#define SOCKET_FAILURE ~0 // -1

class CServer
{
private:
	int m_sock;
	sockaddr_in m_sockaddr;

	std::vector<int> m_connections;
	
public:
	CServer();
	~CServer();
	int Run(int port);

	void SendTo(int connection, const std::string& message);
	int AwaitConnection();
};

CServer::CServer()
	: m_sock(0)
{
	printf("I am SERVER\n");
}

CServer::~CServer()
{
	
}

int CServer::Run(int port)
{
	printf("DO -> Creating socket\n");
	m_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (m_sock == SOCKET_FAILURE)
	{
		printf("NO  = Disgrace\n");
		return -1;
	}
	printf("YES = Created socket %i\n", m_sock);

	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_addr.s_addr = INADDR_ANY;
	m_sockaddr.sin_port = htons(port);

	printf("DO -> Binding to port %i\n", port);
	if (bind(m_sock, rcast(sockaddr*, &m_sockaddr), sizeof(m_sockaddr)) < 0)
	{
		printf("NO  = Failed to bind to port %i. errno: %i\n", port, errno);
		return -1;
	}
	printf("YES = Binded to port %i\n", port);

	printf("DO -> Listen to 10 connections\n");
	// Start listening. Hold at most 10 connections in the queue
	if (listen(m_sock, 10) < 0) {
		printf("NO  = Failed to listen on socket. errno: %i\n", errno);
		return -1;
	}
	printf("YES = Started listening to 10 connections\n");

	printf("Please input number of clients.\n> ");
	int connections;
	scanf("%i", &connections);
	for (int i = 0; i < connections; ++i)
	{
		std::string info;
		
		printf("Waiting for client %i/%i...\n", i + 1, connections);
		int connection = AwaitConnection();
		m_connections.push_back(connection);

		info = "ID ";
		info += std::to_string(connection);
		info += "\n";
		SendTo(connection, info);
		
		for (int& reader : m_connections)
		{
			info = "SERVER Player ";
			info += std::to_string(i + 1);
			info += " joined the lobby.\n";
			SendTo(reader, info);
			info = "SERVER Waiting for ";
			info += std::to_string(connections - i - 1);
			info += " more player(s) to join.\n";
			SendTo(reader, info);
		}
	}
	
	char buffer[100];
	std::string resp;
	while (true)
	{
		for (int& sender : m_connections)
		{
			send(sender, "SEND", 4, 0);

			printf("WAITING FOR CONNECTION \"%i\" TO SEND A COMMAND\n", sender);
			
			memset(buffer, 0, sizeof(buffer));
			read(sender, buffer, 100);
			resp = std::string(buffer);
			resp = std::string(resp.c_str());

			std::string info = "CLIENT ";
			info += std::to_string(sender);
			info += " ";
			info += resp;

			printf("SENDING \"%s\" TO ALL OTHER CONNECTIONS\n", info.c_str());

			for (int& reader : m_connections)
			{
				if (reader == sender)
					continue;

				SendTo(reader, info);
			}
		}
	}

	for(int& connection : m_connections)
	{
		close(connection);
	}
	close(m_sock);
	return 0;
}

void CServer::SendTo(int connection, const std::string& message)
{
	send(connection, message.c_str(), message.size(), 0);
}

int CServer::AwaitConnection()
{
	auto addrlen = sizeof(m_sockaddr);
	int connection = accept(m_sock, rcast(sockaddr*, &m_sockaddr), rcast(socklen_t*, &addrlen));
	return connection;
}

/*
 * List of Commands
 *
 * ID %i
 * ^ Server assigned id to client
 *
 * SEND
 * ^ Clients turn to send a command
 *
 * SERVER
 * ^ Message from server
 *
 * CLIENT %i ...
 * ^ Custom argument sent by client %i
 *
 */
