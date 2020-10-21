from redelServer import *
from sys import argv, stdin
from select import select
from socket import socket
import os

from defs import *
import defs
from databaseManager import DatabaseManager
from jobManager import JobManager
import transaction

global server_socket, clients

def listen_on(ip : str, port : int):
	s = socket()
	s.bind((ip, port))
	s.listen(5)
	return s


BUFFER_SIZE = 1024
def sendfile(s : socket, path : str):
	with open(path, 'rb') as f:
		send(s, f.read())

def recvfile(s : socket, path : str):
	with open(path, 'wb') as f:
		f.write(recv(s))

def send(s : socket, msg : str):
	s.sendall(str(len(msg)).zfill(8).encode())
	for start_chunk in range(0, len(msg), BUFFER_SIZE):
		s.sendall(msg[start_chunk:start_chunk+BUFFER_SIZE])
	s.sendall(msg.encode())

def recv(s : socket):
	size = int(s.recv(8).decode())
	msg = ""
	while len(msg) < size:
		msg += s.recv(min(BUFFER_SIZE, size-len(msg))).decode()
	return msg



def handle_request(request : str):
	command, *args = request.split(' ')
	command = command.lower()
	if command in CREATE_JOBS:
		if len(args) != 4:
			print("Create takes exactly 4 arguments.")
			return
		graph_name, steps, approximate, name = args
		if defs.db_m.get_jobGroup(name):
			print("JobGroup already exists.")
			return
		folder_path = f"./Jobs/{name}/"
		os.makedirs(folder_path)
		if not folder_path.endswith("/"):
			folder_path += "/"
		print(f"Creating {name}.")
		defs.job_m.create_jobGroup(graph_name, int(steps), int(approximate), folder_path, name)
		print(f"Completed creating {name}.")
	elif command in ADD_GRAPH:
		graph_path, graph_name = args
		defs.db_m.register_graph(graph_path, graph_name)
		print(f"Added {graph_name}")
	elif command in START_JOBS:
		name = args[0]
		if defs.job_m.add_jobGroup(name):
			print(f"Added {name} to jobs queue.")
	elif command in STOP_JOBS:
		pass
	elif command in HELP:
		print("""Welcome to SubgraphCounter Server-App!
Commands:
	Create graph steps num path name
		- Creates (About) 'num' jobs ([path]_0, [path]_1, ...) for calculating
		  the number of subgraphs of the registered 'graph' with 'steps' connected nodes.
		  The created jobs are registered in the server's database under 'name'.
	Graph path name
		- Register the graph in 'path' under 'name'.
	Start name
		- Start (or continue) working on the jobs registered under 'name'.
	Stop name
		- Stop (pause) working on the jobs registered under 'name'.
	Help [H]
		- Present all possible commands, with explanation.
	List (optional graph|group|queue)
		- Prints data about groups and graphs, leave empty for all data, or filter by optinal.
	Percentage [%] name
		- Prints the percentage (%) of the completed jobs registered under 'name'.
	Results [Res] name
		- Prints the current total results of the jobs registered under 'name'.
	Priority [Prio] (name, index)
		- If no arguments given - prints all active job groups by their priority.
		  Else - moves jobs registered under 'name' to priority 'index'.
	Close
		- Exits app, closes server and database 
""")
	elif command in LIST_DATA:
		data_type = None
		if len(args) == 1:
			data_type = args[0]
		if not data_type or data_type == "graph":
			data = defs.db_m.get_all_graphs()
			print("Available graphs:")
			print(data)
		if not data_type or data_type == "group":
			data = list(defs.db_m.get_all_jobGroups().keys())
			print("Available groups:")
			print(data)
		if not data_type or data_type == "queue":
			data = defs.job_m.get_queued_jobGroups()
			print("Queued groups:")
			print(data)
	elif command in GET_PERCENTAGE:
		pass
	elif command in GET_LATEST_RESULTS:
		pass
	elif command in PRIORITY:
		pass
	elif command in CLOSE_APP:
		print("Start closing.")
		defs.db_m.close()
		server_socket.close()
		for c in clients:
			c.close()
		print("Done closing.")
		exit()
	else:
		print(f"Failed! \"{command}\" is not a valid command.")

def handle_client(c : socket, addr):
	request, *args = recv(c).split(' ')

	if request == CLOSE_CON:
		clients.remove(c)
		print('connection closed: ' + str(addr))
	else:
		if request == GET_JOB and len(args) == 0:
			job = defs.job_m.get_next_job()
			if not job:
				send(c, NO_JOB_FOUND)
				return
			send(c, JOB_FOUND)
			job_id, job_file_path, graph_name = job
			send(c, graph_name)
			send(c, str(job_id))
			sendfile(c, job_file_path)
		elif request == POST_RES and len(args) == 2:
			job_id, result = args
			defs.job_m.post_result(int(job_id), int(result))
		elif request == GET_GRAPH and len(args) == 1:
			graph_name = args[0]
			graph_file_path = defs.db_m.get_graph(graph_name)
			sendfile(c, graph_file_path)



def main():
	# global job_m, db_m
	global server_socket, clients
	defs.db_m = DatabaseManager()

	defs.job_m = defs.db_m.get_jobManager()
	if not defs.job_m:
		defs.job_m = JobManager()
		defs.db_m.register_jobManager(defs.job_m)
		transaction.commit()

	server_socket = listen_on('0.0.0.0', 36446)
	clients = []
	socket2addr = {}
	handle_request("list")
	while True:
		r,_,_ = select([server_socket,stdin]+clients, [], [])
		for c in r:
			if c == server_socket:
				c, addr = server_socket.accept()
				socket2addr[c] = addr
				clients.append(c)
			elif c == stdin:
				handle_request(input())
			else:
				handle_client(c, socket2addr[c])
		transaction.commit()

if __name__ == '__main__':
	main()
