#
## Filename: chatclient.py
## Author: Tim Robinson
## Date created: 8/4/2015
## Last modification date: 8/8/2015 11:41 PM PDT
# 

from socket import *
from struct import *
from os import listdir
import sys

#####
### Function: incorrect_syntax
### Description: This function prints the correct syntax to run the program.
###	This function is used when incorrect syntax is given by the user.
### Parameters: None
### Preconditions: None.
### Postconditions: This function terminates with the correct syntax printed
### 	to the screen.
#####
def incorrect_syntax():
	print "Incorrect syntax!"
	print "Use \"python ftclient.py <server name> <server port> -l <data port>\""
	print " or \"python ftclient.py <server name> <server port> -g <filename> <data port>\""

#####
### Function: check_port_nums
### Description: This function ensures that the port numbers provided by the
###	user are valid port numbers. This function returns true if the port 
###	numbers provided are not valid.
### Parameters: arg1 and arg2 are command line arguments provided by the user
### Preconditions: The command line arguments passed to this function must be
###	integers for this function to act as expected.
### Postconditions: This function terminates by returning true for invalid port
###	numbers and returning false for valid port numbers.
#####
def check_port_nums(arg1, arg2):
	if (not (0 < arg1 < 65535)) or (not (0 < arg2 < 65535)):
		return True
	else:
		return False

#####
### Function: check_user_args
### Description: This function ensures that the syntax used by the user is
###	valid. The only allowable commands given by the user are "-l" and "-g".
###	If the user requests a file with the "-g" command, they must also provide
###	a file name. The user must also provide valid port numbers with either
###	command given. If the user's syntax is not correct, this function ends
###	the program after printing the expected syntax.
### Parameters: argv is the array of command line arguments provided by the 
###	user.
### Preconditions: None - this function will validate the arguments provided
###	by the user.
### Postconditions: This function terminates the program if the syntax
###	provided by the user is incorrect or unexpected. If the user provides
###	exactly 5 command line arguments, the "-l" command must be the third 
###	argument. If the user provides exactly 6 command line arguments, the "-g"
###	command must be the third argument. This function then calls the
###	check_port_nums function to ensure that valid port numbers have been
###	provided by the user in the appropriate place, depending on the command
###	given. If the syntax is invalid, the correct syntax is printed to the
###	screen using the incorrect_syntax function and the program ends. 
#####
def check_user_args(argv):
	if len(argv) == 5:
		if check_port_nums(int(argv[2]), int(argv[4])):
			print "You must enter valid port numbers."
			sys.exit()
		elif argv[3] != "-l":
			incorrect_syntax()
			sys.exit()
		return 'l'
	elif len(argv) == 6:
		if check_port_nums(int(argv[2]), int(argv[5])):
			print "You must enter valid port numbers."
			sys.exit()
		elif argv[3] != "-g":
			incorrect_syntax()
			sys.exit()
		return 'g'
	else:
		incorrect_syntax()
		sys.exit()

#####
### Function: recv_msg
### Description: This function is used for file transfers. This function
###	ensures that the client remains open and receives the entire requested
###	file before closing down the connection. Each requested file is prepended
###	with 4 bytes that contain the size of the incoming file transfer. The
###	client then uses this function to keep the connection open until the
###	correct amount of data has been reached and returns the all of the file
###	contents.
### Parameters: The socket that makes up the client's end of the data 
###	connection.
### Preconditions: The data connection using the socket parameter passed to 
###	this function must be ready to accept data communications in order for
###	this function to act as expected. The incoming file must be prepended
###	with 4 bytes of data containing the exact file size so that the first 4
###	4 bytes of data received by the client can be used to set the amount of
###	data expected by the client. The file transfer will not work as expected
###	if the first 4 bytes of data received by the client are not the file size
###	of the incoming file.
### Postconditions: This function returns the entire contents of the file
###	requested by the client.
#####
def recv_msg(sock):
	data = ''
	flag = True
	n = 4
	while len(data) < n:
		packet = sock.recv(n - len(data))
		if flag:
			n = unpack('>I', packet)[0]
			flag = False
		else:
			data += packet

	return data

# This stores the user's command in a single character, either "l" for a 
# directory listing request or "g" for a specific file transfer request.
command = check_user_args(sys.argv)

# This stores the server's IP address provided by the user.
ip_address = sys.argv[1]

# This stores the server's port number.
server_port = int(sys.argv[2])

# This stores the file requested by the client if one is provided.
file_request = ''

# If the user provided the "-l" command, the data port is the fourth command
# line argument provided by the user. Otherwise the requested file name is the
# fourth command line argument and the data port is the fifth command line 
# argument.
if command == 'l':
	data_port = sys.argv[4]
else:
	file_request = sys.argv[4]
	data_port = sys.argv[5]

# Create TCP sockets by the name commandSocket and dataSocket.
commandSocket = socket(AF_INET, SOCK_STREAM)
dataSocket = socket(AF_INET, SOCK_STREAM)

# This stores the entire server socket address.
server_address = (ip_address, server_port)

# This stores a list of the contents of the current directory.
file_list = listdir('.')

# Iterate through the file list and check to see if the user already has the
# file that they are requesting. Exit the program if they already have that
# file.
for i in file_list:
	if i == file_request:
		print "You already have that file!"
		sys.exit()

# Try to connect to the server at the provided address. Exit the program if the
# connection is not successful.
try:
	commandSocket.connect(server_address)
except:
	print "No go on the connection..."
	print "Command not delivered."
	sys.exit()

# Allow for the command socket to be reused.
commandSocket.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)

message = command

# Build the appropriate message to send to the server that contains the request
# information. The syntax of the message sent of the server is either:
#	- "l <data port>"
#		- or -
#	- "g <file name> <data port>"
if command == 'l':
	message += ' '
	message += data_port
	message += '\0'
else:
	message += ' ' 
	message += file_request
	message += ' ' 
	message += data_port 
	message += '\0'

# If the port is invalid at the server location, receiving a message from the
# server may take longer than expected.
print "Command sending... "
print "This may take dozens of seconds... "

# Send the message to the server over the command connection.
commandSocket.send(message)

# This stores any server response over the command connection.
command_incoming = ''

# This stores any server response over the data connection.
data_incoming = ''

# Allow for the data socket to be reused.
dataSocket.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)

# Try to bind the specified data port provided by the client.
try:
	dataSocket.bind((gethostname(), int(data_port)))
except:
	print "Could not bind data port"

# This will store the server's address used for the data connection.
server_address_data = (ip_address, int(data_port))

# Try to connect to the server at the address specified for the data 
# connection.
try:
	dataSocket.connect(server_address_data)

# Exit the program if the data connection fails to allow the client to connect
# using a different port. The connection may fail if the specified port is 
# currently being used by either the client or the server. The client can 
# either wait to see if the port is freed up in time or try another port for
# the data connection.
except:
	print "Either wait or try another port!"
	sys.exit()

# Wait for a response from the server over the command connection. If there is
# error regarding the client's request, an error message will come over the 
# command connection. If there is not an error with the request, the client
# will receive a confirmation response over the command connection and should
# expect to receive data from the server over the data connection.
command_incoming = commandSocket.recv(4096)

# If any of the available error messages are received by the server, print the
# error to the screen, close the command connection, and exit the program.
if (command_incoming == "FILE NOT FOUND") or (command_incoming == "COMMAND INVALID") or (command_incoming == "NO TEXT FILES") or (command_incoming == "PORT INVALID ON SERVER"):
	print ip_address + ":" + str(server_port) + " says " + command_incoming
	commandSocket.close()
	sys.exit()

# Close the command connection.
commandSocket.close()

# If the user provided the command to list the server directory contents, wait
# for data over the data connection and print the server's response. Exit the
# program afterwards.
if command == 'l':
	print "Receiving directory structure from " + ip_address + ":" + str(data_port)
	
	# The server's response is the formatted directory listing of ".txt" files.
	data_incoming = dataSocket.recv(1024)
	print data_incoming
	sys.exit()

# Reaching this point means that the client is expecting a response to its file
# request. Use the recv_msg to ensure the client receives the file in its
# entirety before automatically closing the data connection. The file will be
# transferred by creating a file by the same name and reading in all of the
# contents of the requested file.
data_incoming = recv_msg(dataSocket)

if command == 'g':
	print "Receiving \"" + file_request + "\" from " + ip_address + ":" + str(data_port)
	
	# Open a file with the name of the requested file.
	file_output = open(file_request, 'a')

	# Write the file contents given by the server to the new file.
	file_output.write(data_incoming)
	print "File transfer complete."

# Close the newly received constructed file.
file_output.close()

# Exit the program.
sys.exit()