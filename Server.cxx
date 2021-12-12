#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fstream>
#include <netinet/in.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <ctime>

void processClientRequest(int connSock) {
	int received;
	char path[1024], buffer[1024], cstring[1024], omit = '.';
	struct dirent* dirEntry;

	// Assign time and date.
	time_t ttime = time(0);
	char* dnt = ctime(&ttime);

	// Read a message from the client.
	if ((received = read(connSock, path, sizeof(path))) < 0) {
		perror("receive");
		exit(EXIT_FAILURE);
	}

	// Path error checking.
	if (path[4] != '/') {
		std::cerr << path << ":" << " No '/' found for GET path.\n";
		strcpy(buffer, path);
		strcat(buffer, ": No '/' found for GET path.\n");
		strcat(buffer, dnt);
		write(connSock, buffer, strlen(buffer));
		exit(EXIT_FAILURE);
	}

	for (int i = 0; path[i] != '\0'; i++) {
		if (path[i + 1] == '.' && path[i] == '.') {
			std::cerr << path << ":" << " No '.' substrings can be used in the path.\n";
			strcpy(buffer, path);
			strcat(buffer, ": No substrings can be used in the path.\n");
			strcat(buffer, dnt);
			write(connSock, buffer, strlen(buffer));
			exit(EXIT_FAILURE);
		}
	}

	// Get the path.
	char* copy = path;
	for (int i = 0; path[i + 1] != '\0'; i++)
		copy[i] = path[i + 4];

	// Prepare the input stream.
	std::ifstream myfile;
	std::string myfile_content;

	struct stat path_stat;
	std::cout << "client request: " << path << "\n";
	stat(path, &path_stat);

	// Case 1: Path refers to file.
	if (S_ISREG(path_stat.st_mode)) {
		myfile.open(path);

		// Read the file.
		if (myfile.is_open()) {
			while (std::getline(myfile, myfile_content)) {
				std::strcpy(cstring, myfile_content.c_str());
				strcpy(buffer, cstring);
				strcat(buffer, " ");
				// std::cout << cstring << " "; // Echo to server.
				write(connSock, buffer, strlen(buffer));
			}

			strcpy(buffer, "\n");
			write(connSock, buffer, strlen(buffer));
			write(connSock, dnt, strlen(dnt));
			dup(connSock);
			myfile.close();
			close(connSock);
			exit(EXIT_SUCCESS);
		}

		// The file is not found in the path specified.
		else {
			strcpy(buffer, path);
			strcat(buffer, ": file does not exist.");

			if (write(connSock, buffer, strlen(buffer)) < 0) {
				perror("write");
				exit(EXIT_FAILURE);
			}
		}

		close(2);
		dup(connSock);
		perror(path);
		exit(EXIT_FAILURE);
	}

	// Case 2: Path refers to directory.
	if (S_ISDIR(path_stat.st_mode)) {
		DIR* dirp = opendir(path);

		// Error checking.
		if (dirp == 0) {
			close(2);
			dup(connSock);
			perror(path);
			exit(EXIT_FAILURE);
		}

		// A list of the files will be returned.
		else {
			while ((dirEntry = readdir(dirp)) != NULL) {
				strcpy(buffer, dirEntry->d_name);
				strcat(buffer, " ");

				if (*buffer == omit)
					continue;

				// std::cout << buffer << " "; (echo to server)
				if (write(connSock, buffer, strlen(buffer)) < 0) {
					perror("write");
					exit(EXIT_FAILURE);
				}
			}

			closedir(dirp);
			strcpy(buffer, "\n");
			write(connSock, buffer, strlen(buffer));
			write(connSock, dnt, strlen(dnt));
			close(connSock);
			exit(EXIT_SUCCESS);
		}
	}

	// Case 3: Path does not lead to existing file or directory.
	strcpy(buffer, path);
	strcat(buffer, ": could not open directory or file.\n");
	write(connSock, buffer, strlen(buffer));
	write(connSock, dnt, strlen(dnt));
	perror("write");
	exit(EXIT_FAILURE);
}

/** Program to implement a simple file TCP server.
 * Send the contents of the specified directory to the client.
 *
 * Command line arguments:
 *	argv[1] port number to receive requests.
 */
int main(int argc, char* argv[]) {
	if (argc != 2) {
		std::cerr << "USAGE: TCPServerReadDir port.\n";
		exit(EXIT_FAILURE);
	}

	// Create the TCP socket.
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	// Create address structures.
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;
	unsigned int addrlen = sizeof(client_address);

	// Construct the server sockaddr_in structure. 
	memset(&server_address, 0, sizeof(server_address));   // Clear struct.
	server_address.sin_family = AF_INET;                  // Internet/IP. 
	server_address.sin_addr.s_addr = INADDR_ANY;          // Any IP address.
	server_address.sin_port = htons(atoi(argv[1]));       // server port.

	// Bind the socket.
	if (bind(sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
		perror("bind");
		exit(EXIT_FAILURE);
	}

	// Make socket passive and set length of queue.
	if (listen(sock, 64) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	std::cout << "TCPServerReadDir listening on port: " << argv[1] << "\n";

	// Run until cancelled.
	while (true) {
		int connSock = accept(sock, (struct sockaddr*)&client_address, &addrlen);

		if (connSock < 0) {
			perror("accept");
			exit(EXIT_FAILURE);
		}

		// Parent process.
		if (fork()) {
			close(connSock);
		}

		// Child process.
		else {
			processClientRequest(connSock);
		}
	}

	close(sock);
	return 0;
}
