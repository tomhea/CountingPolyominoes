from redelServer import *
from sys import argv, stdin
from select import select
from socket import socket
import os

from databaseManager import DatabaseManager
from jobManager import JobManager


NO_JOB_FOUND = "None"
JOB_FOUND = "Job"

GET_JOB = "Get"
POST_RES = "Post"
GET_GRAPH = "Graph"

CREATE_JOBS = ("create",)	# + graph_file_path : str, steps : int, approx_num_of_jobs : int, job_base_path : str, (name : str to be added to DB)
ADD_GRAPH = ("graph",)		# + graph_file_path : str, (name : str to be added to DB)
START_JOBS = ("start",)		# + JobGroup name;    folder and graph_path are inferred from DB
STOP_JOBS = ("stop",)		# + JobGroup name
HELP = ("help", 'h')		# print every command possible
GET_PERCENTAGE = ("percentage", "%")	# + JobGroup name
GET_LATEST_RESULTS = ("results", "res")	# + jobGroup name
PRIORITY = ("priority", "prio")		#   ( + jobGroup name )


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
	if   command in CREATE_JOBS:
		pass
	elif command in ADD_GRAPH:
		pass
	elif command in START_JOBS:
		pass
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
				Percentage [%] name
					- Prints the percentage (%) of the completed jobs registered under 'name'.
				Results [Res] name
					- Prints the current total results of the jobs registered under 'name'.
				Priority [Prio] (name, index)
					- If no arguments given - prints all active job groups by their priority.
					  Else - moves jobs registered under 'name' to priority 'index'.
			""")
		pass
	elif command in GET_PERCENTAGE:

		pass
	elif command in GET_LATEST_RESULTS:
		pass
	elif command in PRIORITY:
		pass


def handle_client(clients, c : socket, addr):
	global job_m

	request = c.recv(1024).decode()
	if request in ('', 'close'):
		clients.remove(c)
		print('connection closed: ' + str(addr))
	else:
		if   request.startswith(GET_JOB):
			job = job_m.get_new_job()
			if job == None:
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
	job_m = JobManager()
	db_m = DatabaseManager()

	s = listen_on('0.0.0.0', 36446)
	clients = []
	socket2addr = {}
	while True:
		r,_,_ = select([s,stdin]+clients, [], [])
		for c in r:
			if c == s:
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
