/******************************************************************************
 ** Filename: chatserver.c
 ** Author: Tim Robinson
 ** Date created: 8/4/2015
 ** Last modification date: 8/8/2015 11:42 PM PDT
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

int main(int argc, char *argv[]) {

	// This is used to store the command port number.
	char *PORT;

	// This is used to store the data port number.
	char *DATA_PORT;

	// This stores the status of the getaddrinfo function in creating the list
	// of potential command port sockets.
	int status;

	// This stores the status of the getaddrinfo function in creating the list
	// of potential data port sockets.
	int status_data;

	// This stores the file descriptor for the command socket returned by the
	// socket function.
	int s;

	// This stores the file descriptor for the data socket returned by the
	// socket function.
	int s_data;

	// This stores the file descriptor for the command connection returned by
	// the accept function.
	int new_fd;

	// This stores the file descriptor for the data connection returned by the
	// accept function.
	int data_fd;

	// This stores the file descriptor for the client requested file.
	int send_fd;

	// This is used to send the 1 as an address while setting the socket
	// options.
	int yes = 1;
	
	// These structs will store the client's socket address information. The
	// first is for the command socket information; the second is for the data
	// socket information.
	struct sockaddr_storage their_addr_1, their_addr_2;

	// Stores the address size of the client's socket. THis is used in accepting
	// the connection with the client.
	socklen_t addr_size;

	// These structs will hold the address information for the server's sockets.
	// These are used in creating the sockets. The "res" struct will store a 
	// linked list of addrinfo structs used to create the socket, the "p" struct
	// will be used as an iterator through the "res" linked list.
	struct addrinfo servSet, data_sock, *res, *p;

	// This stores the client's IP address
	char inaddr[INET6_ADDRSTRLEN];

	// The server must run the program using the appropriate syntax. If the user
	// does not provide a port number, provides too many arguments, or provides
	// an invalid port number, the program exits and prints the syntax error for
	// the user.

	// There must be exactly one argument provided to the program.
	if(argc != 2) {
		fprintf(stderr, "Port must be provided. Use \"./ftserver <port number>\"\n");
		exit(1);

	// The port number must be between 0 and 65535.
	} else if((atoi(argv[1]) > 65535) || (atoi(argv[1]) < 0)) {
		fprintf(stderr, "You must use valid port number.\n");
		exit(1);

	// Store the provided argument as the port number.
	} else {
		PORT = argv[1];
	}



// SET UP THE SERVER COMMAND SOCKET

	// Clear all of the data at the servSet and data_sock structures.
	memset(&servSet, 0, sizeof servSet);
	memset(&data_sock, 0, sizeof data_sock);

	// Set the socket for a TCP connection with SOCK_STREAM. AI_PASSIVE allows
	// the IP address to fill in for us, which we do with the localhost for
	// testing. Do this for both the command socket and the data socket.
	servSet.ai_socktype = SOCK_STREAM;
	servSet.ai_flags = AI_PASSIVE;
	data_sock.ai_socktype = SOCK_STREAM;
	data_sock.ai_flags = AI_PASSIVE;

	// The "res" variable points to a linked list of addrinfo structs created
	// using the information in PORT and servSet. NULL is used here to use the
	// localhost as the server location.
	if((status = getaddrinfo(NULL, PORT, &servSet, &res)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(1);
	}

	// Loop through the returned linked list of addrinfo structs and create a
	// socket using the first successful link in the list.
	for(p = res; p != NULL; p = p->ai_next) {

		// Try to create a socket using the current addrinfo struct; if it is not
		// successful try the next struct in the linked list. Store the socket
		// using the 's' file descriptor.
		if((s = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("server socket failure");
			continue;
		}

		// Allow the reuse of the socket created above.
		int optval = 1;
		setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

		// Assign the socket address to the socket referred to by 's'.
		if(bind(s, p->ai_addr, p->ai_addrlen) == -1) {
			close(s);
			perror("server command bind failure");
			continue;
		}

		// If we get here, we successfully created and bound the socket. Stop
		// iterating.
		break;
	}

	// We no longer need the linked list since we have the socket and file
	// descriptor for the command socket.
	freeaddrinfo(res);

	// Open the port for connections and make sure it didn't fail.
	if(listen(s, 10) == -1) {
		perror("server listen failure");
		exit(1);
	}

	// Always wait for a new connection to be made with the server.
	while(1) {
		printf("Server listening on %s...\n", PORT);

		// This will store the client's message to the server.
		char * message = (char *)malloc(sizeof(char) * 100);

		// This will store the status of receiving the client's message.
		int received;

		// This will store the server's response to the client.
		char * response = malloc(sizeof(char) * 1000);

		// This will store the size of the server's response to the client.
		int response_size = 0;

		// Accept a new connection when requested. Necessarily, the client will
		// try to establish the command connection before the data connection.
		// The "new_fd" variable will store the command connection file 
		// descriptor.
		new_fd = accept(s, (struct sockaddr *)&their_addr, &addr_size);

		// Store the size of the client's address.
		addr_size = sizeof their_addr_1;

		// Ensure that accepting the command connection did not fail. If it did,
		// continue waiting for further connections.
		if(new_fd == -1) {
			perror("server accept failure");
			continue;
		}

		// This will store the client's port number used to establish the 
		// connection with the server.
		unsigned short inport;

		// This gets the IP address of the client and store it the "inaddr"
		// variable. This assumes an IPv4 for the client's IP for simplicity.
		inet_ntop(AF_INET, &(((struct sockaddr_in *)(struct sockaddr *)&their_addr_1)->sin_addr), 
					inaddr, sizeof inaddr);

		// This gets the client's port number and stores it in the inport 
		// variable.
		inport = ((struct sockaddr_in *)(struct sockaddr *)&their_addr_1)->sin_port;
		printf("Connected to client at %s\n", inaddr);

		// Wait to receive a message from the client. This is done immediately
		// upon establishing a connection for this program.
		while((received = recv(new_fd, message, 100, 0))) { break; }

		// If the user's command is not "-g" or "-l" in the client program,
		// give an error response to the client. The client's program should
		// process this potentiality before reaching this point, but this
		// fortifies the check.
		if((message[0] != 'g') && (message[0] != 'l')) {
			response = "COMMAND INVALID";
			response_size = strlen(response);

			// Send the error response to the client over the command connection.
			int newmessage = send(new_fd, response, response_size, 0);

		// If the command given by the client is valid...	
		} else {

			// These are iterators. "i" is used to move through the client's 
			// command. "parse_count" is used to track the arguments provided by
			// the client.
			int i = 0;
			int parse_count = 0;

			// This will help process the port number given by the client for the
			// data connection.
			char * tmpString = malloc(sizeof(char) * 24);


			// The server must parse each command ("-l" and "-g") differently.
			// The client provides a file name and a data port number to complete 
			// a file request ("-g") and provides only a data port number for a 
			// directory list request ("-l").

			// This if-else statement ends with the data port number stored in the
			// tmpString variable, regardless of the command provided by the user.

			// If the command is a file request...
			if(message[0] == 'g') {

				// Iterate through the client's message.
				while(i < strlen(message)) {

					// The second argument after the "-g" command provided by the 
					// client is the data port number. The first argument is the
					// file name, which will be processed later.
					if(message[i] == ' ') {
						if(parse_count == 1) {	

							// This is an iterator to process the data port number out
							// of the client's message.	
							int j = 0;

							// Build the port number in tmpString from the client's
							// message.
							while((j + i + 1) < strlen(message)) {
								tmpString[j] = message[j + i + 1];
								j += 1;
							}

							// Ensure that tmpString is terminated with '\0'.
							tmpString[j] = '\0';
							break;
						}
						parse_count += 1;
					}
					i += 1;
				}
			} else {
				while(i < strlen(message)) {

					// The first and only argument after the "-l" command provided
					// by the client is the data port number.
					if(message[i] == ' ') {

						// This is an iterator to process the data port number out of
						// the client's message.
						int j = 0;

						// Build the port nubmer in tmpString from the client's
						// message.
						while((j + i + 1) < strlen(message)) {
							tmpString[j] = message[j + i + 1];
							j += 1;
						}

						// Ensure that tmpString is terminated with '\0'.
						tmpString[j] = '\0';
						break;
					}
					i += 1;
				}
			}

			// Store the data port number from the client's message in the
			// "DATA_PORT" variable.
			DATA_PORT = tmpString;
	
			printf("Processing command...\n");


			// Now that we have the data port number from the client's message, we
			// must create a data connection between the server and the client in
			// addition to the command connection they already have.

			// The "res" variable points to a linked list of addrinfo structs created
			// using the information in DATA_PORT and data_sock. NULL is used here to
			// use the localhost as the server location.
			if((status_data = getaddrinfo(NULL, DATA_PORT, &data_sock, &res)) != 0) {
				fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
				exit(1);
			}

			// Loop through the returned linked list of addrinfo structs and create a
			// socket using the first successful link in the list.
			for(p = res; p != NULL; p = p->ai_next) {

				// Try to create a socket using the current addrinfo struct; if it is 
				// not successful try the next struct in the linked list. Store the 
				// socket using the 's_data' file descriptor.
				if((s_data = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
					perror("server socket failure");
					continue;
				}

				// Allow the reuse of the socket created above.
				int optval = 1;
				setsockopt(s_data, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

				// Assign the socket address to the socket referred to by 's'. If
				// binding the socket fails, then the port requested for data by the
				// client is not valid on the server. The server then sends the
				// relevant error message to the client so the client can try using
				// a different port.
				if(bind(s_data, p->ai_addr, p->ai_addrlen) == -1) {
					close(s_data);
					response = "PORT INVALID ON SERVER";
					response_size = strlen(response);

					// Send the error message over the command connection to the 
					// client.
					int newmessage = send(new_fd, response, response_size, 0);
					perror("server data bind failure");
					break;
				}

				break;
			}

			// We no longer need the linked list since we have the socket and file
			// descriptor for the data socket.
			freeaddrinfo(res);

			// Open the data port for connections and make sure it didn't fail.
			if(listen(s_data, 10) == -1) {
				perror("server listen failure");
				continue;
			}

			// Accept a new connection when requested. Necessarily, the client will
			// try to establish the data connection. The "data_fd" variable will 
			// store the data connection file descriptor.
			data_fd = accept(s_data, (struct sockaddr *)&their_addr_2, &addr_size);

		}

		// Clear the contents of tmpString to be used to process the requested
		// file name from the client.
		char * tmpString = malloc(sizeof(char) * 100);
		
		// If the client has given the file request command, we must process the
		// first argument, which is the requested file's name.
		if(message[0] == 'g') {

			// This is an iterator to work through the whole client's message.
			int i;
			
			while(i < strlen(message)) {
				if(message[i] == ' ') {
					
					// This is an iterator to process the client's requested file
					// name.
					int j = 0;

					// Build the file name in the tmpString variable. No white space
					// are allowed in the file name.
					while(message[j + i + 1] != ' ') {
						tmpString[j] = message[j + i + 1];
						j += 1;
					}

					// Ensure that tmpString ends with '\0'.
					tmpString[j] = '\0';
					break;
				}
				i += 1;
			}

			printf("File \"%s\" requested on port %s\n", tmpString, DATA_PORT);
		}

		// This is used to process all of the files in the directory of this
		// file.
		DIR *dirp;
		struct dirent *dir;

		// Open the current directory.
		dirp = opendir(".");

		// This is used to track whether or not the user's requested file is
		// found in the current directory.
		int match = 0;

		// If we have successfully opened the current directory for iteration...
		if(dirp) {
			
			// Iterate through the contents of this directory.
			while((dir = readdir(dirp)) != NULL) {

				// We will only concern ourselves here with files ending with 
				// ".txt". This program can only process, list, and transfer ".txt"
				// files.
				int max_idx = strlen(dir->d_name);

				// Store the final four characters of the current file as the
				// extension of the file.
				const char *ext = &(dir->d_name[max_idx - 4]);

				// If the file has the extension ".txt", store the file name as a
				// part of the response to the client.
				if(strncmp(ext, ".txt", 4) == 0) {

					// This is used only in the event of a client providing a "-g"
					// command.
					if(strncmp(dir->d_name, tmpString, max_idx) == 0) {
						match = 1;
					}
		
					// Add the ".txt" file to the response to the client.
					strcat(response, dir->d_name);
					strcat(response, "\n");

					// Track the size of the response to the client so that we know
					// how large the response we are sending to the client.
					// This is used only in the event of a client providing a "-l"
					// command.
					response_size += max_idx;
					response_size += 1;
				}
			}
		}

		closedir(dirp);

		// If the client requested a file and it was not found, send the error
		// response to the client over the command connection.
		if((message[0] == 'g') && (match == 0)) {
			response = "FILE NOT FOUND";
			response_size = strlen(response);

			printf("File not found. Sending error message to %s:%s\n", inaddr, PORT);

			// Send the error response to the client over the command connection.
			int newmessage = send(new_fd, response, response_size, 0);

		// If the client requested a file, we must process the file appropriately
		// and send it to the client in a reliable manner. This assumes that a
		// matching file has been found on the server.
		} else if(message[0] == 'g') {
			

			// In order to reliably transfer all file contents to the client, the
			// server must communicate the size of the file so the client knows to
			// remain open and receiving content over the data connection until
			// that file size is reached.
			// This means that we must get the size of the requested file and add
			// this size to the beginning of the response to the client. The 
			// client then removes this size measurement and stays open until the
			// amount of data received matches this size measurement by the 
			// server.
			// On the server, we do this by getting the size of the requested
			// file, adding this size to a new temporary file, reading the 
			// requested file contents into that temporary file line by line, and
			// then send the temporary file to the client line by line. The client
			// then removes the first four bytes from the file, which the client
			// knows contains the size of the file, and then remains open to
			// receive communication over the data connection until the size of
			// the file has been recieved.

			// These will be used to process the file requested by the client.
			char * line = NULL;
			size_t len = 0;
			ssize_t read;

			// Send a confirmation to the client over the command connection. This
			// means that the client should wait for data over the data 
			// connection.
			char * confirm = " ";
			int confirm_command = send(new_fd, confirm, 1, 0);

			// The tmpString contains the name of the client's requested file.
			// Open the requested file to read in its contents to be processed.
			send_fd = open(tmpString, O_RDONLY);
			FILE * fp = fdopen(send_fd, "r");

			// Get and store the size of the requested file.
			struct stat st;
			stat(tmpString, &st);
			int size = st.st_size;


			// Convert the file size to bytes. The file size in bytes will prepend
			// the data communicated to the client so the client knows how much 
			// data to remain open and receiving for.

			// This will store the file size in bytes.
			unsigned char bytes[4];
			unsigned long file_size = size;
			bytes[0] = (file_size >> 24) & 0xFF;
			bytes[1] = (file_size >> 16) & 0xFF;
			bytes[2] = (file_size >> 8) & 0xFF;
			bytes[3] = file_size & 0xFF;

			// Open the temporary named "tmpfile" for writing.
			FILE * fp_size = fopen("tmpfile", "w");
			
			// Ensure that the temporary file was created successfully.
			if(fp_size == NULL) {
				perror("Error in opening file to store size.\n");
			}

			// Write the file size in bytes to the file as the first contents.
			fwrite(bytes, sizeof *bytes, sizeof bytes, fp_size);

			// Close the file for writing only. The only content of the file is
			// the size of the file requested by the client.
			fclose(fp_size);

			// Open the temporary file for appending.
			fp_size = fopen("tmpfile", "a");

			// Read in the contents of the file requested by the client and write
			// them in the temporary file after the four bytes containing the file
			// size.
			while((read = getline(&line, &len, fp)) != -1) {
				fwrite(line, 1, strlen(line), fp_size);
			}

			// Close the temporary file for appending.
			fclose(fp_size);

			// Open the temporary file for reading.
			fp_size = fopen("tmpfile", "r");

			// These will be used to process the temporary file and send the 
			// contents appropriately to the client.
			char * line_in = NULL;
			char size_to_send[4];
			char * tmpline = NULL;
			int flag = 1;

			// Read the first four bytes from the temporary file to send to the
			// client. This is the first message sent to the client in response to
			// the file request.
			fread(size_to_send, 4, 1, fp_size);
			int firstmessage = send(data_fd, size_to_send, 4, 0);

			// Read in the contents of the temporary file line by line and send
			// each line to the client. The client will continue to wait for 
			// response lines until they have received the correct amount of data.
			while((read = getline(&line_in, &len, fp_size)) > 0) {
					int newmessage = send(data_fd, line_in, strlen(line_in), 0);
			}

			printf("Sending \"%s\" to %s:%s\n", tmpString, inaddr, DATA_PORT);
			
			// Delete the temporary file from the current directory.
			remove("tmpfile");

			// Close the data connection with the client.
			close(data_fd);
		}

		// If the user gave the "-l" command...
		if(message[0] == 'l') {
			printf("List directory requested on port %s\n", DATA_PORT);

			// If the response_size is 0, then no ".txt" files are in this
			// directory.
			if(response_size == 0) {
				response = "NO TEXT FILES";
				response_size = strlen(response);
			}

			printf("Sending directory contents to %s:%s\n", inaddr, DATA_PORT);

			// Send a confirmation to the client over the command connection. This
			// means that the client should wait for data over the data 
			// connection.
			char * confirm = " ";
			int confirm_command = send(new_fd, confirm, 1, 0);

			// Send the file list to the client.
			int newmessage = send(data_fd, response, response_size, 0);

			// Close the data connection with the client.
			close(data_fd);
		}
	}

	return 0;
}