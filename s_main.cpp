/*
 * s_main.cpp
 * main_server
 */
#include "s_server.hpp"
int main(int argc, char **argv)
{
	CServer server;
	int res = server.Run(9999); // Block
	return res;
}
