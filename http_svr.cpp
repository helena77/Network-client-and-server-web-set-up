/* A simple server in the internet domain using TCP
The port number is passed as an argument */
// Author: Helena
// FileName: warmup_svr.cpp
// Date: 10/17/2018
// version: 1

// Description:
// The server responds by sending the reversed message back to the client
// A simple server in the internet domain using TCP
// The port number is passed as an argument


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>
#include <netdb.h>       				// defines structure hostent
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sysmacros.h>
#include <string>
#include <strings.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <fstream>
using namespace std;

const int MAX_SIZE = 512;
const int BACKLOG = 10;
const string webroot = "./web_root";

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

/*
 * Send messages
 */
void sendMsg(int sockfd, string msg)
{
    int len = msg.length();
	int bytesLeft = len;
	char buffer[len];
	strcpy(buffer, msg.c_str());
	char *bp = buffer;
	while (bytesLeft) {
		int bytesSent = send(sockfd, (void *)bp, bytesLeft, 0);
		if (bytesLeft <= 0) {
			cerr << "Error sending message\n";
		}
		bytesLeft = bytesLeft - bytesSent;
		bp = bp - bytesSent;
	}
	//cerr << "Send: " << buffer;
}

void * threadMain(void *args)
{
	long sockfd = (long) args;
	char request[MAX_SIZE];	
	string resource = "";
	char *ptr;        
    int sockfd1;
	int length;
	struct stat sb;
	string msg ="";
	stringstream length_str;
	string file_length;
	string contentType ="";
    
	

	//receive msg
	string s = "\r\n\r\n";
	char line[4];
	strcpy(line, s.c_str());
    int size = 4;
	char *p = request;            // we'll be using a pointer to the buffer than to mess with buffer directly
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
	cerr << "Recv: \n" << request << endl;
    

    // Checking for a valid browser request
	string http = "HTTP/";	
	string request_cli = request;
	size_t index = request_cli.find(http);
	string command = "/../";
	size_t bad_index = request_cli.find(command);
    if(index == std::string::npos || bad_index != std::string::npos)
    {		 
		msg = "HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n";
        sendMsg(sockfd, msg);
    }
    else
    {
        ptr = NULL;   	
        if( strncmp(request,"GET ",4) == 0)
        {
			string a = request_cli.substr(4, index - 5);
			char request1[500];
            strcpy(request1, a.c_str());
			ptr = request1;				
        }       
        if(ptr == NULL)
        {
			msg = "HTTP/1.1 501 Not Implemented\r\nConnection: close\r\n\r\n";
            sendMsg(sockfd, msg);
        }
        else
        {
            if( ptr[strlen(ptr) - 1] == '/' )
            {
				strcat(ptr,"index.html");
				contentType = ".html";
				//cerr << ptr << endl;
            }
			else {
				char delim = '.';
				int delim_index = -1;
				for (unsigned i = 0; i < strlen(ptr); i++) {
					if (ptr[i] == delim) {
						delim_index = (int)i;
						i = strlen(ptr);
						for (unsigned j = delim_index; j < strlen(ptr); j++)
							contentType += ptr[j];
						//cerr << contentType;
					}
				}
				if(delim_index == -1)
					strcat(ptr,"/index.html");
					contentType = ".html";
			}
			//cerr << ptr << endl;
            resource = webroot;
			//cerr << resource << endl;
            resource += ptr;
			//cerr << resource << endl;
            sockfd1 = open(resource.c_str(), 0, O_RDONLY);
            cerr << "Opening " << resource << "\n";
            if(sockfd1 == -1)
            {
                cerr << "404 File not found Error\n";
				msg = "HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\n";
                sendMsg(sockfd, msg);               
            }
            else
            {
				stat(resource.c_str(), &sb);
				length =  (int)sb.st_size;
				cerr << length << endl;
				length_str << length;
				file_length = length_str.str();
                cerr << "200 OK!!!\n";
				string message1 = "HTTP/1.0 200 OK\r\n";
				string message2 = "Connection: close\r\n";
				string time = ctime(&sb.st_atime);
				string message3 = "Date : " + time;
				string time_mod = ctime(&sb.st_mtime);
				string message4 = "Last-Modified: " + time_mod;
				string message5 = "Content-Length: " + file_length + "\r\n";
				string type = "";
				if (contentType == ".txt") {
					type = "text/plain";
				} else if (contentType == ".html") {
					type = "text/html";
				} else if (contentType == ".htm") {
					type = "text/htm";
				} else if (contentType == ".css") {
					type = "text/css";
				} else if (contentType == ".jpg" || contentType == ".jpeg") {
					type = "image/jpeg";
				} else if (contentType == ".png") {
					type = "image/png";
				} else {
				}
				string message6 = "Content-Type: " + type + "\r\n\r\n";  
				msg = message1 + message2 + message3 + message4 + message5 + message6;
                sendMsg(sockfd, msg);               					
				//read(sockfd1, ptr, length);
				ifstream fin(resource.c_str(), ios_base::binary);
				string msg2 = "";
				//fin.open(resource);
				char temp;
				while (fin.good()) {
					temp = fin.get();
					msg2 += temp;
				}
				sendMsg(sockfd, msg2);								
            }
        }
    }
	
	pthread_detach(pthread_self());	
	close(sockfd);
	return 0;
}


int main(int argc, char *argv[])
{
	int sockfd;						// the socket only listens for connections
	long newsockfd;					// the connected socket
	struct addrinfo hints;
    struct addrinfo *result;
	struct addrinfo *rp;
	int server;

	// check if the command line is valid
	// the command line will be: warmup_svr port
	// it means there are two elements in argv[] array
	// the argc stands for the amount of elements
	// so it need to be at least 2(0, 1) so that the 1 stands for the port number
	if (argc < 2) {
		cerr << "ERROR, no port provided\n";
		exit(1);
	}
	
	memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    		// Allow IPv4 or IPv6 
    hints.ai_socktype = SOCK_STREAM; 		// TCP socket 
    hints.ai_flags = AI_PASSIVE;    		// For wildcard IP address 
    hints.ai_protocol = 0;          		// Any protocol 
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    server = getaddrinfo(NULL, argv[1], &hints, &result);
    if (server != 0) {
        cerr << "getaddrinfo: " << gai_strerror(server) << "\n";
        exit(1);
    }

    // getaddrinfo() returns a list of address structures.
    // Try each address until we successfully bind(2).
    // If socket(2) (or bind(2)) fails, we (close the socket
    // and) try the next address. */

	for (rp = result; rp != NULL; rp = rp->ai_next) {
        sockfd = socket(rp->ai_family, rp->ai_socktype,
                rp->ai_protocol);
        if (sockfd == -1)
            continue;

        if (bind(sockfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;                  /* Success */

       close(sockfd);
    }

    if (rp == NULL) {               /* No address succeeded */
        fprintf(stderr, "Could not bind\n");
        exit(1);
    }

    freeaddrinfo(result);           /* No longer needed */

	// SET SOCKET TO LISTEN
	// BACKLOG means the max number of connections that can be waiting while the process 
	// is handling a particular connection
	listen(sockfd, BACKLOG);

	while (true) {

		// Accept connection from client

		struct sockaddr_in cli_addr;					// client address in a struct which containing an internet address
		socklen_t clilen = sizeof(cli_addr);			// size of client's address

		// ACCEPT CONNECTION (CREATE A NEW SOCKET)
		// parameter 1: the socket
		// parameter 2: client address which cast to sockaddr
		// parameter 3: the size of client address
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

		// check if new socket is valid
		if (newsockfd < 0)
			error("ERROR on accept");

		// Create client thread
		pthread_t threadID;
		int status = pthread_create(&threadID, NULL, threadMain, (void *)newsockfd);
		if (status != 0) 
			exit(-1);      // Note: status == 0 is GOOD
	}

	//close(sockfd);
	return 0;
}


