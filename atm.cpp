/**
	@file atm.cpp
	@brief Top level ATM implementation file
 */
#include <string>
#include <iostream>
#include <cassert>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

using namespace std;

void withdraw_send(string& input, char packet[]) {

	int sec_space = input.find(" ", 9);
	string temp = input.substr(9, sec_space);

	printf("%s withdrawn.\n", temp.c_str());

	for(unsigned int i = 0; i < input.size(); ++i) {
		packet[i] = input[i];
	}
	return;
}

void login_send(string& input, char packet[]) {
	//Get PIN from Bank
	//prompt for PIN
	int sec_space = input.find(" ", 6);
	string temp = input.substr(6, sec_space);

	printf("User %s is logging in.\n", temp.c_str());

	for(unsigned int i = 0; i < input.size(); ++i) {
		packet[i] = input[i];
	}
	return;
}

void balance_send(string& input, char packet[]) {
	printf("Checking Balance\n");
	for(unsigned int i = 0; i < input.size(); ++i) {
		packet[i] = input[i];
	}
}

void transfer_send(string& input, char packet[]) {
	int space = input.find(" ", 9);
	string amount = input.substr(9, (space-9));
	printf("amount %s\n", amount.c_str());
	space = input.find(" ", 9+amount.size());
	string destination = input.substr(space+1);
	printf("Transfering %s to %s.\n", amount.c_str(), destination.c_str());

	for(unsigned int i = 0; i < input.size(); ++i) {
		packet[i] = input[i];
	}
}

int send(int sock, int length, char packet[]) {
	//send the packet through the proxy to the bank
	if(sizeof(int) != send(sock, &length, sizeof(int), 0))
	{
		printf("fail to send packet length\n");
		return -1;
	}
	if(length != send(sock, (void*)packet, length, 0))
	{
		printf("fail to send packet\n");
		return -1;
	}
	return 0;
}

int receive(int sock, int length, char packet[]) {
	if(sizeof(int) != recv(sock, &length, sizeof(int), 0))
	{
		printf("fail to read packet length\n");
		return -1;
	}
	if(length >= 1024)
	{
		printf("packet too long\n");
		return -1;
	}
	if(length != recv(sock, packet, length, 0))
	{
		printf("fail to read packet\n");
		return -1;
	}
	return 0;
}

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		printf("Usage: atm proxy-port\n");
		return -1;
	}
	
	//socket setup
	unsigned short proxport = atoi(argv[1]);
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(!sock)
	{
		printf("fail to create socket\n");
		return -1;
	}
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(proxport);
	unsigned char* ipaddr = reinterpret_cast<unsigned char*>(&addr.sin_addr);
	ipaddr[0] = 127;
	ipaddr[1] = 0;
	ipaddr[2] = 0;
	ipaddr[3] = 1;
	if(0 != connect(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)))
	{
		printf("fail to connect to proxy\n");
		return -1;
	}
	
	//input loop
	char buf[80];
	string input;
	while(1)
	{
		printf("atm> ");
		//fgets(buf, 79, stdin);
		//buf[strlen(buf)-1] = '\0';	//trim off trailing newline
		getline(cin, input);
		//TODO: your input parsing code has to put data here
		char packet[1024];
		int length = input.size();
		//char command[8];
		//strncpy(command, buf, 8);

		//input parsing
		if(input.find("logout") == 0)
			break;
		else if(input.find("withdraw ") == 0) {
			//takes in withdraw [amount]
			withdraw_send(input, packet);
		}
		else if(input.find("login ") == 0) {
			//takes in login [username]
			login_send(input, packet);

		}
		else if(input.find("balance") == 0) {
			balance_send(input, packet);
		}
		else if(input.find("transfer ") == 0) {
			//takes in transfer [amount] [username]
			transfer_send(input, packet);
		}
		else {
			printf("invalid input.  Valid operations are:\n \
login | withdraw | balance | transfer | logout\n");
		}
		
		if (send(sock, length, packet) == -1)
			break;

		for(unsigned int i = 0; i < input.size(); ++i) {
			packet[i] = '\0';
		}
		
		//TODO: do something with response packet
		if(receive(sock, length, packet) == -1)
			break;

		string str(packet);
		cout << str << endl;
		
		/////////

		if(input.find("withdraw ") == 0) {
			//Successful or not 
		}
		else if(input.find("login ") == 0) {
			//Is user?
			if( str.find("-1") == 0){
				printf("User not found.\n");
			}
			else if( str.find("1") == 0) {
				printf("Please enter a PIN\n");
				string pin;
				getline(cin, pin); 
				assert(pin.size() == 4);
				cout << pin << endl;
				for(unsigned int i = 0; i < 4; ++i) {
					packet[i] = pin[i];
				}
				if(send(sock, pin.size(), packet) == -1)
					break;
				if(receive(sock, length, packet) == -1)
					break;
			}
				//Prompt for pin
				//Try again
		}
		else if(input.find("balance") == 0) {
			//Return Packet should include balance
		}
		else if(input.find("transfer ") == 0) {
			//Successful or not
		}

		/////////

		

		
	}
	
	//cleanup
	close(sock);
	return 0;
}
