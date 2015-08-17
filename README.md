# File Transfer Server and Client
## I. File list
ftserver.c&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;File transfer server implementation<br />
ftclient.py&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;File transfer client implementation<br />
README.md&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;This file<br />


## II. Program Instructions

#### A. Brief program overview

This program consists of two programs running in conjunction, the server and 
the client. The server accepts requests from the client and responds on over 
one of two connections between them, either the command connection or the data
connection. The command connection is used to communicate requests from the 
client to the server and error messages from the server to the client. The data
connection is used by the server to send a data response based on the client's
valid request. The server either sends a list of the ".txt" files in its
current directory or sends a file requested by the client over the data
connection.

#### B. Compiling and running the server

In order to run the server program on the server, the user must provide a port
number that will serve as an endpoint for the command connection. This is one
of the ports that the client specifies when the client runs their program. The
syntax to compile the server program is as follows (from here forward, the '"' 
characters used to indicate command line input may be ignored):

	"gcc -o <server object file> ftserver.c"

So for testing, the syntax that I used, including the object file that I will
refer to for the rest of this document is as follows:

	"gcc -o ftserver ftserver.c"

Running the object file will then create a socket on which the server will wait
for clients to establish contact. Upon a client's contact, the connection will
be made and the server will parse any command given by the client. The object
file is run and the server started with the following syntax:

	"./ftserver <port number>"

So for testng, the syntax that I used, including the port number that I will
refer to for the rest of this document is as follows:

	"./ftserver 53148"

The server program then begins and notifies the user that the server is waiting
for connections on port 53148.

#### C. Running the client and establishing a command connection

In order to run the client program, all of the relevant request information
must be included in the program call. There are two commands that the user can
provide, one being a request for the directory listing of ".txt" files in the
server's directory, the other being a request for a specific file located in
the server's directory. The first is executed by the client "-l" command, the 
second by the client "-g" command. The syntax to execute the list command is as
follows:

	"python ftclient.py <server name> <server port num> -l <data port num>"

The syntax to execute the file request command is as follows:

	"python ftclient.py <server name> <server port num> -g <file name> <data port num>"

For either function, the server name and port number must match the server's 
name and port number provided by the user in running the server program. The server runs their program as the "localhost", while the data port number is 
the port number upon which the server and the client will establish 
a connection on both ends of the connection. The syntax must follow the 
structure delineated above exactly in order for the program to function as 
expected.

When a user runs the client program by either syntax, the client program 
attempts to make a connection with the server at the port number and name
specified. If the connection is successful, then the user constructs its
message containing the command and relevant information and sends it to the
server, which is discussed further below. In order for continuous commands to
be sent to the server from different executions of the client program,
different data port numbers must be provided, as the server and client both 
take a varying amount of time to completely clear the use of repeated port
numbers, even though each program allows for the fastest repeated use 
of ports possible. Any erroneous repitition or erroneous selection of ports
currently being used by other applications is undefined for both the server
and client, though some cases are caught and allow for repeated program runs.
The only defined method of obtaining a correct run when an erroneous case is
encountered is to ensure that both the server and client programs are
exited, the server restarted, and the client run again using a different data
port.

#### D. Client and server connections

The two possible connections between the server and the client are the 
command connection and the data connection. The command connection is
established if the server is running and the client is run using valid a valid
command and valid port information. Upon establishing a command connection, 
the client constructs a message to send to the server. The syntax of the
message sent by the client depends upon the valid command entered by the 
client. If the client provides the list command, the syntax of the message is
as follows:

	"l <data port number>"

If the client provides the file request command, the syntax of the message is
as follows:

	"g <file name> <data port number>"

The server then parses the message received by the client, validates the
information received in the message and responds appropriately. In either case,
both the client and the server create a socket at the data port provided in the
client program. The only files that are acknowledged in either command instance
are ".txt" files. The server then listens on the data port, while the client
attempts to connect to the server's data port. If any errors are encountered 
by the server, error messages are transmitted to the client over the command
connection. All successful command responses are given over the data 
connection, from server to client.

#### E. Client requests of the server and responses from the server.

The client can request either a list of the ".txt" files in the server's
directory or the client can request a specific ".txt" file in the server's 
directory to be sent to them by the server.

In either the case, the server iterates through the files in its directory. If
the client requests a list of the ".txt" files, the server stores each ".txt"
file found in a variable and sends the string to the client over the data 
connection. If the client requested a specific file and the file is not found 
by the server, the server sends a "FILE NOT FOUND" error over the command 
connection. If the file is found by the server, the file is sent to the client
line by line over the data connection, according to a specific protocol 
outlined below in II.F and II.G.

#### F. File transfer sending from the server

Assuming that the file name included in the message sent from the client to the
server is valid and is found in the server's directory, the server sends the 
file to the client. An issue that is encountered is that the client does not
know whether or not they have received the entire file before closing the 
connection or ceasing its reception of packets comprising the file. In order
to address this problem, I used a protocol that both the server and client
implement in order to guarantee that the total file size is transferred in the
data stream from the server to the client.

Upon creating receiving a request for a file that exists in the server's
directory, the server obtains the size of the file and stores it in a variable.
The server then converts that size to a value consisting of exactly 4 bytes.
This value is then entered into a temporary file, after which all of requested
file contents are added. Then this temporary file is sent to the client.

Effectively what is happening is that the client is sent the file that they
requested, prepended with exactly 4 extra bytes of data, which gives the 
client the incoming (requested) file size. The inherent limitation is that the
maximum file size that can be sent reliably over the data connection between
the server and client is roughly 2 GB (2147483648 Bytes, to be exact), due to
the use of exactly 4 bytes to encode the file size.

#### G. File transfer receiving by the client

Upon receiving a file over the data connection, the client knows that the first
4 bytes of data received will contain the exact size of the incoming file that
they requested from the server. The client then knows to continue to keep the
data connection open and to expect data over the data connection until the file
size specified in the first 4 bytes is reached. The client then constructs the
file from the data they received, excluding the 4 bytes of data giving the size
measure. The client uses the name of the file in their file request to the 
server to create a file of the same name. All of the received file contents
can be written to that file and thus the file has been transferred from the
server to the client.

If the client already has a file of the name of the requested file, 
communication from the client to the server is prevented and an error message
is printed to the screen. The client program does not verify the lengths of the
file requested and the file received, though this can be verified by the users 
of both the server and the client programs.


## III. Implementation notes

A basis from which this program derived its initial structure came from an
earlier project of mine involving data streams in communication between a
server and a client. Because the implementation of this program leaned on some
of the methodologies and syntax used in that program, I have included many of
the helpful references from this first project in this one as well, located
below in section IV.

To reiterate, the data port number provided by the client used throughout
testing was necessarily highly variable as to allow previously used data ports
to be freed and reused at a later time. Using the same port numbers repeatedly
for the data connection is undefined, as the program should avoid being
interfered with by previous runs of the server and client programs.

While testing, the Project Gutenberg release of The Brothers Karamazov by
Fyodor Dostoevsky, "TheBrothersKaramazov.txt", a 2007708 byte file was used in
implementing the protocol for large file transfer. A test file called
"reallyshort.txt", a 29 byte text file was used in testing the protocol for
small file transfer.

While testing, the server locale used for the server was the flip1 server
on Oregon State University's engineering server. The server runs with its IP
address of localhost. The server locale used for the client was the flip2 
server on Oregon State University's engineering server, accessing the ftserver
by the name "flip1" in the appropriate command line argument. Testing was also 
done using my local machine as the ftserver and ftclient locale, with the 
ftserver using the localhost as its name and address location, and the client 
using the name "localhost" as the server name in the appropriate command line 
argument.


## IV. References

##### A. Beej's Guide to Network Programming | Using Internet Sockets
Brian "Beej Jorgensen" Hall<br />
http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html

	1. This resource was used for proper syntax and structure for a program
	implementing internet sockets to accomplish network programming tasks.

	2. This resource aided in the understanding, creation, and use of various
	structs involved in network programming in C. This includes the handling of
	addrinfo structs, sockaddr structs, and sockaddr_in structs.

	3. This resource aided in the understanding, creation, and use of various
	functions involved in network programming in C. This includes the handling
	of the getaddrinfo() function, the socket() function, the bind() function,
	the listen() function, the accept() function, the inet_ntop() function, the
	recv() function, the send() function, and the close() function.

	4. This resource aided in the understanding and use of the various structs
	and functions listed in IV.A.2 and IV.A.3 in conjunction with one another.
	Much of the structure of this program was aided by the information offered
	in this resource, though the program was ultimately built to accomplish the
	operation outlined in II.A-G. 

##### B. CS 372 Lecture #15 | Socket Programming Primer
Oregon State University, Summer 2015<br />
https://courses.ecampus.oregonstate.edu/index.php?video=cs372/15.mp4<br />
&nbsp;&nbsp;&nbsp;(login required)<br />

	1. This resource was used for proper syntax and structure for a program
	implementing internet sockets to accomplish network programming tasks.

	2. This resource aided in the understanding, creation, and use of various
	functions involved in network programming in Python. This includes the 
	handling of the socket() function, the send() function, the recv() function,
	and the close() function.

	3. This resource aided in the understanding and use of the various functions
	listed in IV.B.2 in conjunction with one another. Much of the structure of 
	this program was aided by the information offered in this resource, though 
	the program was ultimately built to accomplish the operation outlined in 
	II.A-G.

##### C. How to list files in a directory in a C program?
Responding user: Jean-Bernard Jansen on November 17, 2010<br />
http://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program

	1. This resource was used for proper syntax and structure for a program 
	implementing iteration through the file contents of the current working
	directory.

	2. This resource aided in the understanding, creation, and use of the DIR * 
	variable, the dirent struct, opendir function, the readdir function, and the
	closedir function. This also aided in the understanding of the use of the
	dirent.h library.

	3. This resource aided in the understanding and use of the various 
	variables, structs, and functions listed in IV.C.3 in conjunction with one
	another in order to accomplish the processing of the ".txt" files located
	in the server's current directory.

##### D. How to Compare Last n Characters of A String to Another String in C
Responding user: Oliver Charlesworth on March 14, 2011<br />
http://stackoverflow.com/questions/5297248/how-to-compare-last-n-characters-of-a-string-to-another-string-in-c

	1. This resource was used for proper syntax and structure for a program
	implementing the comparison of the final four characters of a string to
	another string.

	2. This resource was used to obtain the appropriate pointer so that I could
	then use the strncmp function to compare the file extensions of each file
	in the server's current directory to ".txt". This allowed me to ensure that
	the server was only giving ".txt" files and file listings to the client
	upon the client's request.

##### E. C read file line by line
Responding user: mbaitoff on August 17, 2010, edited by user Octopus on September 19, 2014
http://stackoverflow.com/questions/3501338/c-read-file-line-by-line

	1. This resource was used for the proper syntax and structure for a program
	implementing the iteration, line by line, through a file. This resource was
	used as a reference in using the getline function to read in file lines of
	the user's requested file to build the appropriate temporary file prepended 
	with the file's size, and then used in reading in file lines of the 
	temporary file to send to the client.

	2. This resource aided in the understanding, creation, and use of a file
	pointer, a pointer to a line to store each line as it is read in, a size_t
	variable to store the length of the line being read, and a ssize_t read 
	variable to store the status of each line as it is read with the getline 
	function.

	3. This resource aided in the understanding and use of the variables and
	function referred to in IV.E.2 in conjunction with one another in order
	to effectively process the file contents in sending the requested file from 
	the server to the client.

##### F. How can I get a file's size in C? [duplicate]
Responding user: Greg Hewgill on October 26, 2008<br />
http://stackoverflow.com/questions/238603/how-can-i-get-a-files-size-in-c

	1. This resource was used for the proper syntax and structure for a program
	obtaining the size of a specific file.

	2. This resource was used in the understanding, creation, and use of the 
	stat struct, the stat function, and the st_size attribute of the stat 
	struct. These were used to obtain the integer measure of the size of the
	file requested by the client. This is instrumental in the reliable transfer
	of the requested file from the server to the client.

	3. This resource aided in the understanding and use of the struct and 
	function in IV.F.2 in conjunction with one another to obtain the file size
	of the client's requested file.

##### G. Converting an int into a 4 byte char array (C)
Responding user: caf on September 24, 2010, edited by betabandido on June 7, 2012<br />
http://stackoverflow.com/questions/3784263/converting-an-int-into-a-4-byte-char-array-c

	1. This resource was used for the proper syntax and structure for a program
	implementing the conversion of an integer to a 4-byte char array. This 
	resource was used in the proper use of the unsigned char array to store the
	converted integer, the unsigned long variable to store the integer file 
	size, the bit-shift operator, and the conversion to hexadecimal in the
	appropriate part of the array.

	2. This resource aided in the understanding of how to convert an integer
	to a consistent 4-byte array so that the server could prepend the requested
	file with a 4-byte measure of the requested file size.

	3. This resource was instrumental in implementing the server-side
	implementation of the large file transfer protocol of adding a 4-byte
	measure of the file size to every file requested so the client knows 
	exactly how long to continue accepting packets of data.

##### H. Invalid argument exception in socket.accept() if I restart immediately after a previous run quit
Responding user: cnicutar on February 13, 2013

	1. This resource was used for the proper syntax and structure of the 
	setsockopt function to ensure that a socket reopen as soon as possible upon
	a program closing its connection with another socket.

	2. This resource was used in the understanding of the SO_REUSEADDR option in
	the socket layer in Python.

##### I. Python Socket Receive Large Amount of Data
Responding user: Adam Rosenfield on July 16, 2013<br />
http://stackoverflow.com/questions/17667903/python-socket-receive-large-amount-of-data

	1. This resource was used for the proper syntax and structure of a program
	implementing a large file transfer protocol in defining a specific function
	that governs how long a file transfer reception stays open according to the
	incoming file's size.

	2. This resource was used in the creation of the recv_msg function in its
	use of a file's size to govern a while loop in continually accept packets
	to accomplish a more reliable file transfer from the server to the client.
	This resource was used to understand and use the unpack function, which 
	allows the client program to read the first 4 bytes of the transmitted file
	so that the file size can be used to govern the reception of the entire 
	file.

	3. This resource was used in the understanding of the necessity and the 
	creation of a specific protocol that stores the requested file's size in the
	server's response to the client. Using the first 4 bytes of the transmission
	of the file to encode the file's size allowed me to accomplish the 
	successful transfer of both small and large files from the server to the 
	client.

	4. This resource was instrumental behind the implementation of the large
	file transfer functionality of the client and the idea behind the 
	transmission of large files from the server. This resource aided in
	building a function that returns all of the received file's contents in one
	variable, which later in the client program is written to complete the file
	transfer.

##### J. How to list all files of a directory in Python
Responding user: pycruft on July 8, 2010, edited by Neftas on April 28, 2014

	1. This resource was used in the understanding and use of a program that
	can get current directory file contents and iterate through them.

	2. This resource aided in the use of the listdir function in order for the
	client program to verify that the file request entered by the user is not
	already present in their current directory.
