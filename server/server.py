from redelServer import *
from sys import argv, stdin
from select import select
from socket import socket
import os

from databaseManager import DatabaseManager
from jobManager import JobManager
import transaction

global job_m, db_m
global server_socket

NO_JOB_FOUND = "None"
JOB_FOUND = "Job"

GET_JOB = "Get"
POST_RES = "Post"
GET_GRAPH = "Graph"
CLOSE_CON = ""

CREATE_JOBS = ("create",)	# + graph_file_path : str, steps : int, approx_num_of_jobs : int, job_base_path : str, (name : str to be added to DB)
ADD_GRAPH = ("graph",)		# + graph_file_path : str, (name : str to be added to DB)
START_JOBS = ("start",)		# + JobGroup name;    folder and graph_path are inferred from DB
STOP_JOBS = ("stop",)		# + JobGroup name
HELP = ("help", 'h')		# print every command possible
LIST_DATA = ("list",)		# + graph/group;
GET_PERCENTAGE = ("percentage", "%")	# + JobGroup name
GET_LATEST_RESULTS = ("results", "res")	# + jobGroup name
PRIORITY = ("priority", "prio")		#   ( + jobGroup name )
CLOSE_APP = ("close",)		# Closes the app


def listen_on(ip : str, port : int):
	s = socket()
	s.bind((ip, port))
	s.listen(5)
	return s


def sendfile(s : socket, path : str):
	file_size = os.path.getsize(path)
	s.send(str(file_size).zfill(8).encode())
	with open(path, 'rb') as f:
		s.sendall(f.read())


def handle_request(request : str):
	command = request.split(' ')[0].lower()
	if command in CREATE_JOBS:
		graph_name, steps, num, folder_path, name = request.split(' ')[1:]
		graph_file_path = db_m.get_graph(graph_name)
		if not folder_path.endswith("/"):
			folder_path += "/"
		print(f"Creating {name}.")
		job_m.create_jobGroup(graph_file_path, steps, num, folder_path, name)
		print(f"Completed creating {name}.")
	elif command in ADD_GRAPH:
		graph_path, graph_name = request.split(' ')[1:]
		db_m.register_graph(graph_path, graph_name)
		print(f"Added {graph_name}")
	elif command in START_JOBS:
		name = request.split(' ')[1:]
		if job_m.add_jobGroup(name):
			print(f"Added {name} to jobs queue.")
		else:
			print(f"Adding {name} failed! queued already.")
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
		if len(request.lower().split(' ')) == 2:
			data_type = request.lower().split(' ')[1]
		if not data_type or data_type == "graph":
			data = db_m.get_all_graphs()
			print("Available graphs:")
			print(data)
		if not data_type or data_type == "group":
			data = list(db_m.get_all_jobGroups().keys())
			print("Available groups:")
			print(data)
		if not data_type or data_type == "queue":
			data = job_m.get_queued_jobGroups()
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
		db_m.close()
		server_socket.close()
		print("Done closing.")
		exit()
	else:
		print(f"Failed! \"{command}\" is not a valid command.")

def handle_client(clients, c : socket, addr):
	request = c.recv(1024).decode()
	if request == CLOSE_CON:
		clients.remove(c)
		print('connection closed: ' + str(addr))
	else:
		if request.startswith(GET_JOB):
			job = job_m.get_next_job()
			if not job:
				c.send(NO_JOB_FOUND.encode())
				return
			c.send(JOB_FOUND.encode())
			job_id, job_file_path, graph_name = job
			c.send(graph_name)
			c.send(job_id)
			sendfile(c, job_file_path)
			pass
		elif request.startswith(POST_RES):
			_, job_id, result = request.split(' ')
			job_m.post_result(job_id, int(result))
		elif request.startswith(GET_GRAPH):
			_, graph_name = request.split(' ')
			graph_file_path = db_m.get_graph(graph_name)
			sendfile(c, graph_file_path)
			pass

		# c.send(request.encode())
		# print(request)



def main():
	global job_m, db_m
	global server_socket
	job_m = JobManager()
	db_m = DatabaseManager()
	server_socket = listen_on('0.0.0.0', 36446)
	clients = []
	socket2addr = {}
	handle_request("Help")
	while True:
		r,_,_ = select([server_socket,stdin]+clients, [], [])
		for c in r:
			if c == server_socket:
				c, addr = s.accept()
				socket2addr[c] = addr
				clients.append(c)
			elif c == stdin:
				handle_request(input())
			else:
				handle_client(clients, c, socket2addr[c])
		transaction.commit()

if __name__ == '__main__':
	main()
