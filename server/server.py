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
		send(s, f.read(), False)

def recvfile(s : socket, path : str):
	with open(path, 'wb') as f:
		f.write(recv(s, False))

def send(s : socket, msg : str, encode = True):
	s.sendall(str(len(msg)).zfill(8).encode())
	for start_chunk in range(0, len(msg), BUFFER_SIZE):
		if encode:
			s.sendall(msg[start_chunk:start_chunk + BUFFER_SIZE].encode())
		else:
			s.sendall(msg[start_chunk:start_chunk + BUFFER_SIZE])

def recv(s : socket, decode = True):
	meta_data = s.recv(8).decode()
	if not meta_data:
		return None
	size = int(meta_data)
	msg = b""
	while len(msg) < size:
		msg += s.recv(min(BUFFER_SIZE, size - len(msg)))
	if decode:
		msg = msg.decode()
	return msg



def handle_request(request : str):
	command, *args = request.split(' ')
	command = command.lower()

	if not command:
		pass
	elif command in CREATE_JOBS:
		if len(args) != 4:
			print("Create takes exactly 4 arguments.")
			return
		name, graph_name, steps, approximate = args
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
	elif command in REGISTER_GRAPH:
		if len(args) != 2:
			print("Graph takes exactly 2 arguments.")
			return
		graph_name, graph_path = args
		defs.db_m.register_graph(graph_path, graph_name)
		print(f"Added {graph_name}")
	elif command in START_JOBS:
		if len(args) != 1:
			print("Start takes exactly 1 argument.")
			return
		name = args[0]
		if defs.job_m.add_jobGroup(name):
			print(f"Added {name} to jobs queue.")
	elif command in STOP_JOBS:
		if len(args) != 1:
			print("Stop takes exactly 1 argument.")
			return
		name = args[0]
		if defs.job_m.remove_jobGroup(name):
			print(f"Removed {name} from jobs queue." )
	elif command in HELP:
		if len(args) > 1:
			print("Help takes exactly 0 or 1 arguments.")
			return
		elif args:
			cmd = args[0]
			possible_cmds = [key for key in cmd_dict.keys() if cmd in key]
			if not possible_cmds:
				print("No such command.")
			else:
				print(cmd_dict[possible_cmds[0]])
		else:
			print(WELCOME_MESSAGE)
			print("List of all possible commands:")
			print('\n'.join([cmd_dict[cmd] for cmd in CMD_ORDER]))
	elif command in LIST_DATA:
		if len(args) > 1:
			print("List takes exactly 0 or 1 arguments.")
			return
		data_type = args[0] if args else None
		if not data_type or data_type == "graph":
			data = defs.db_m.get_all_graphs()
			print("Available graphs:")
			print('\n'.join([f"\t{name}: \t{path}" for name, path in data.items()]))
		if not data_type or data_type == "group":
			data = {name: jobGroup.get_percentage() for name, jobGroup in defs.db_m.get_all_jobGroups().items()}
			print("Available groups:")
			print('\n'.join([f"\t{name}: \t{perc:.2f}%" for name,perc in data.items()]))
		if not data_type or data_type == "queue":
			data = defs.job_m.get_queued_jobGroups()
			print("Queued groups:")
			print('\t' + ', '.join(data))
	elif command in GET_PERCENTAGE:
		if len(args) != 1:
			print("Percentage takes exactly 1 argument.")
			return
		name = args[0]
		jobGroup = defs.db_m.get_jobGroup(name)
		if not jobGroup:
			print(f"{name} not found.")
			return
		print(f"\t{jobGroup.get_percentage():.4f}%")
		pass
	elif command in GET_LATEST_RESULTS:
		if len(args) != 1:
			print("Results takes exactly 1 argument.")
			return
		#TODO Exposing jobGroup, might want to find encapsulating solution
		name = args[0]
		jobGroup = defs.db_m.get_jobGroup(name)
		if not jobGroup:
			print(f"{name} not found.")
			return
		res = jobGroup.get_result()
		print(f"Current results:   \t {res:,}")
		perc = jobGroup.get_percentage()
		print(f"Estimated results: \t~{int(res / (perc/100)):,}")
	elif command in PRIORITY:
		if len(args) not in (0,2):
			print("Priority takes exactly 0 or 2 arguments.")
			return
		elif not args:
			for i,name in enumerate(defs.job_m.get_queued_jobGroups()):
				print(f'\t{i+1}) \t{name}')
			pass
		else:
			name, prio = args
			defs.db_m.set_priority(name, int(prio))
			pass
	elif command in CLOSE_APP:
		if len(args) > 1:
			print("Close takes no arguments.")
			return
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
	data = recv(c)
	if not data:
		clients.remove(c)
		print('connection closed: ' + str(addr))
	else:
		request, *args = data.split(' ')
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
	print(WELCOME_MESSAGE)
	print("Enter 'Help' or 'H' for more details.\n")
	print("⛟ ", end="")
	while True:
		r,_,_ = select([server_socket,stdin]+clients, [], [])
		for c in r:
			if c == server_socket:
				c, addr = server_socket.accept()
				socket2addr[c] = addr
				clients.append(c)
			elif c == stdin:
				handle_request(input())
				print("⛟ ", end="")
			else:
				handle_client(c, socket2addr[c])
		transaction.commit()

if __name__ == '__main__':
	main()
