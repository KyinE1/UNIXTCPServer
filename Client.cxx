#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <iostream>

/** Command line arguments:
 * 		argv[1] FQDN of server.
 *		argv[2] port number to send to.
 * 		argv[3] request to send.
 *
 *  To run the server and client:
 * 		server: ./exe_filename port_number (ex: 4321)
 * 		client: ./exe_filename localhost port_number (ex: 4321)
 * 				"GET /" (ex: ., - (home directory), /tmp)
 */
int main(int argc, char* argv[]) {
	if (argc != 4) {
		std::cerr << "USAGE: TCPClient server_name port request.\n";
		exit(EXIT_FAILURE);
	}

	// Lookup FQDN.
	struct addrinfo* res, hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	int error = getaddrinfo(argv[1], argv[2], &hints, &res);
	if (error) {
		std::cerr << argv[1] << ": " << gai_strerror(error) << std::endl;
		exit(EXIT_FAILURE);
	}

	char buffer[1024];
	int sent, received;

	// Create the TCP socket
	int sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock < 0) {
		perror("Failed to create socket");
		exit(EXIT_FAILURE);
	}

	// Connect to server.
	if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
		perror("connect");
		exit(EXIT_FAILURE);
	}

	// Send the message to the server. 
	sent = write(sock, argv[3], strlen(argv[3]) + 1);
	if (sent < 0) {
		perror("write");
		exit(EXIT_FAILURE);
	}

	// Receive the message from the server. 
	do {
		received = read(sock, buffer, sizeof(buffer));
		if (received < 0) {
			perror("read");
			exit(EXIT_FAILURE);
		}

		std::cout.write(buffer, received);
	} while (received > 0);

	std::cout << std::endl;

	close(sock);
}

/**	Error checking.
 *
 * @param path: The specified path.
 */
void error_checking(char* path) {
	if (path[4] != '/') {
		std::cerr << path << ": " << "No '/' found for GET path.\n";
		exit(EXIT_FAILURE);
	}

	for (int i = 0; path[i] != '\0'; i++) {
		if (path[i + 1] == '.' && path[i] == '.') {
			std::cerr << path << ": " << "No .. substrings can be used in the path.\n";
			exit(EXIT_FAILURE);
		}
	}

	// If the file is in the directory, print the contents.
	if (buffer == "index.html") {
		system("CLS");
		myfile.open(path);

		// Read the file.
		if (myfile.is_open()) {
			while (std::getline(myfile, myfile_content))
				strcpy(buffer, dirEntry->d_name);

			strcat(buffer, " ");
			std::cout << buffer << " ";
			write(connSock, buffer, strlen(buffer));
		}

		// File cannot be found.
		if (!myfile.is_open()) {
			std::cout << "Case 2 failing...";
			closedir(dirp);
			close(connSock);
			exit(EXIT_FAILURE);
		}

		myfile.close();
		std::cout << "Index returning... ";
		closedir(dirp);
		close(connSock);
		exit(EXIT_SUCCESS);
	}
}
