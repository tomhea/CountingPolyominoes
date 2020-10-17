from redelServer import *
from sys import argv, stdin
from select import select
from socket import socket

from databaseManager import *
from jobManager import *

# try:
# 	steps = int(argv[1])
# except:
# 	steps = 5

# try: 
# 	approx_num_of_jobs = int(argv[2])
# except:
# 	approx_num_of_jobs = 10

# jobs_dir = "Jobs/"
# graphs_dir = "Graphs/"

# print(jobs_creator(graphs_dir + "Polyominoes.txt", steps, approx_num_of_jobs, jobs_dir + "my" + str(steps) + "Job"))

# print(can_i_finish_it(graphs_dir + "Polyominoes.txt", steps))

GET_JOB = "Get"
POST_RES = "Post"
GET_GRAPH = "Graph"

CREATE_JOBS = "Create"	# + graph_file_path : str, steps : int, approx_num_of_jobs : int, job_base_path : str, (name : str to be added to DB)
START_JOBS = "Start"	# + JobGroup name;    folder and graph_path are inferred from DB
STOP_JOBS = "Stop"		# + JobGroup name
HELP = "Help" 			# print every command possible
GET_PERCENTAGE = "Percentage"	# + JobGroup name
GET_LAST_RESULTS = "Results"	# + num of results wanted

jobs:
	polyominoes:
		5:
			jobs
	polys:
		5:
			jfwi


def listen_on(ip : str, port : int):
	s = socket()
	s.bind((ip, port))
	s.listen(5)
	return s


def handle_request(request : str):
	if   request.startswith(CREATE_JOBS):
		pass
	elif request.startswith(START_JOBS):
		pass
	elif request.startswith(STOP_JOBS):
		pass
	elif request.startswith(HELP):
		pass
	elif request.startswith(GET_PERCENTAGE):
		pass
	elif request.startswith(GET_LAST_RESULTS):
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
				pass
			job_file_name, graph_file_name = job
					# id 4456: path/file
			# c.send(graph_file_name)
			# c.send(job_file_name)
			# c.send(job_file)
			pass
		elif request.startswith(POST_RES):
			_, file_name, result = request.split(' ')
			job_m.post_result(file_name, int(result))
		elif request.startswith(GET_GRAPH):
			_, graph_file_name = request.split(' ')
			# c.send(graph_file)
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


if __name__ == '__main__':
	main()
