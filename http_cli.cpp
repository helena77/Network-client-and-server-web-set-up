// Author: Helena
// FileName: http_cli.cpp
// Date: 10/28/2018
// version: 1

// Description:
// This client program is a simple web (HTTP) client

#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <stdbool.h>
#include <bits/stdc++.h>
#include <iostream>
#include <sstream>
using namespace std;

const int MAX_SIZE = 512;
const string STR = "\r\n\r\n";

void error(const char *msg)
{
	perror(msg);
	exit(0);
}

int main(int argc, char *argv[])
{
	string request_info;              // the server information	
	string request;
	string request_path;
	string request_hostname;	
	string port;                     // the string of port		
	struct addrinfo hints;
	struct addrinfo *result;
	struct addrinfo *rp;
	int server;
	int sockfd;	
	char buffer[MAX_SIZE];

	// Check if the command line is valid	
	if (argc < 2) {
		cerr << "usage " << argv[0] << " hostname port\n";
		exit(0);
	}

	// Initialize valuable
	request_info = argv[1];
	
	// Get the host name
	string host_and_path = request_info.substr(request_info.find('/') + 2);
	
	char a = '/';
	bool exist = false;
	for (unsigned i = 0; i < host_and_path.size(); i++) {
		if (host_and_path.at(i) == a) {
			exist = true;
		}
	}
	
	if (!exist) {		
		request_path = "";		
	}
	else {
		request_path = host_and_path.substr(host_and_path.find('/') + 1);
	}	
	
	int len_1 = host_and_path.size();
	int len_2 = request_path.size();
	if (exist) {
		len_2 = request_path.size() + 1;
	}	
	int len_3 = len_1 - len_2;	
	request_hostname = host_and_path.substr(0, len_3);	
	
	// Assign port to socket
	char b = ':';
	exist = false;
	for (unsigned i = 0; i < request_hostname.size(); i++) {
		if (request_hostname.at(i) == b) {
			exist = true;
		}
	}
	if (!exist) {		
		port = "80";		
	}
	else {
		port = request_hostname.substr(request_hostname.find(':') + 1);
		request_hostname = request_hostname.substr(0, request_hostname.find(':'));
	}	

	// Obtain address matching host/port
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;        // both IPv4 and IPv6
	hints.ai_socktype = SOCK_STREAM;    // the TCP

	//convert to char
	char requestHostname[request_hostname.size()];
	char portNum[port.size()];
	strcpy(requestHostname, request_hostname.c_str());
	strcpy(portNum, port.c_str());
	server = getaddrinfo(requestHostname, portNum, &hints, &result);
	if (server != 0) {
		cerr << "getaddrinfo: Error\n";
		exit(0);
	}

	// getaddrinfo() returns a list of address structures.
	// Try each address until we successfully connect(2).
	// If socket(2) (or connect(2)) fails, we (close the socket
	// and) try the next address. 
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sockfd = socket(rp->ai_family, rp->ai_socktype,
			rp->ai_protocol);
		if (sockfd == -1)
			continue;

		if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) != -1)
			break;                  // Success 

		close(sockfd);
	}

	if (rp == NULL) {               // No address succeeded 
		cerr << "ERROR connecting\n";
		exit(0);
	}

	freeaddrinfo(result);           // No longer needed 

	// Build Request
	string header = "GET /" + request_path + " HTTP/1.1\r\n";
	string host = "Host: " + request_hostname + "\r\n";
	string connection = "Connection: close\r\n\r\n";

	request = header + host + connection;
	char send_buffer[request.size()];
	// print GET Request
	cerr << "\nRequest: \n";
	cerr << request << "\n";

	// COMMUNICATE		
	// send request			
	bzero(send_buffer, request.size());
	strcpy(send_buffer, request.c_str());
	int bytesLeft = request.size();
	char *bp = send_buffer;
	while (bytesLeft) {
		if (bytesLeft <= 0) {
			cerr << "Error sending message\n";
		}
		int bytesSent = send(sockfd, (void *)bp, bytesLeft, 0);
		bytesLeft = bytesLeft - bytesSent;
		bp = bp - bytesSent;
	}
	
	// print the response Header
	cerr << "\nResponse Header: \n";
	string s = "\r\n\r\n";
	char line[4];
	strcpy(line, s.c_str());
    int size = 4;
	char *p = buffer;            // we'll be using a pointer to the buffer than to mess with buffer directly
    int eol_matched=0;            // this is used to see that the recieved byte matched the buffer byte or not 
	bool isStop = false;
    while(isStop == false && recv(sockfd,p,1,0)!=0)        // start recv bytes 1 by 1
    {
        if( *p == line[eol_matched])    // if the byte matched the first eol byte that is '\r'
        {    
            ++eol_matched; 		
            if(eol_matched == size)    // if both the bytes matches the EOL
            {
                *(p + 1) = '\0';    // End the string
                //return(strlen(buffer));    // return the bytes recieved 				
				isStop = true;
            }
        }
        else
        {
            eol_matched=0;            
        }
		if (isStop == false) {
			p++;                    // increment the pointer to recv next byte
		}
	}
	cerr << buffer << endl;
	

	string receiverMsg = buffer;
	
	// get content length
	if (receiverMsg.find("Content-Length:") != std::string::npos) {
		size_t content_index = receiverMsg.find("Content-Length:");
		string left_header = receiverMsg.substr(content_index);
		int line_index = left_header.find("\n");
		string content_length = left_header.substr(0, line_index);
		string text = "Content-Length: ";
		string length = content_length.substr(text.size());	
		int body_length = atoi(length.c_str());		
	
		// get content body	
		cerr << "Body: \n";		
		char rev_body_buffer[body_length];
		bzero(rev_body_buffer, body_length);
		int rev_body_bytesLeft = body_length;
		char *rev_body_bp = rev_body_buffer;
		while (rev_body_bytesLeft) {
			int bytesRecv = recv(sockfd, rev_body_bp, rev_body_bytesLeft, 0);
			if (bytesRecv <= 0) {
				cerr << "ERROR reading from socket\n";
				exit(-1);
			}	
			rev_body_bytesLeft = rev_body_bytesLeft - bytesRecv;			
			rev_body_bp = rev_body_bp + bytesRecv;
		}
		cout.write(rev_body_buffer, body_length);
	}
	
			
	// CLOSE CONNECTION
	close(sockfd);
	return 0;

}

