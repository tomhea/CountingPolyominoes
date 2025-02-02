from sys import argv, stdin
from os import remove
from os.path import isfile, isdir, abspath
from select import select
from socket import socket
from time import time, sleep

import defs
from defs import *
from redelServer import *
from databaseManager import DatabaseManager
from jobManager import JobManager
import transaction

global server_socket, clients
global last_auto_resched


# handle annoying whitespaces from start, middle and end
def remove_whitespaces(s: str) -> str:
	return ' '.join(s.split())


def safe_int(x: str):
	try:
		return int(x)
	except ValueError:
		return None


# Non-Deterministic Print
def ndprint(s: str):
	print(f'\n{s}')
	show_prompt()


def show_prompt():
	print(INPUT_PROMPT, end="", flush=True)



def listen_on(ip: str, port: int) -> socket:
	s = socket()
	s.bind((ip, port))
	s.listen(5)
	return s


def sendfile(s: socket, path: str) -> bool:
	with open(path, 'rb') as f:
		return send(s, f.read())


def recvfile(s: socket, path: str) -> bool:
	data = recv(s, False)
	if data is None:
		return False
	with open(path, 'wb') as f:
		f.write(data)
	return True


def sendall_timed(s: socket, data: bytes) -> bool:
	send_start_time = time()
	while data:
		if time() - send_start_time > SEND_TIMEOUT:
			return False
		bytes_sent = s.send(data)
		data = data[bytes_sent:]
	return True


# msg can be string or bytes
def send(s: socket, msg) -> bool:
	if not sendall_timed(s, str(len(msg)).zfill(8).encode()):
		return False
	encode = (type(msg) == str)
	for start_chunk in range(0, len(msg), BUFFER_SIZE):
		data = msg[start_chunk:start_chunk + BUFFER_SIZE]
		if not sendall_timed(s, data.encode() if encode else data):
			return False
	return True


def recv(s: socket, decode=True):
	meta_data = s.recv(8).decode()
	if not meta_data:
		return None
	size = safe_int(meta_data)
	if size is None:
		return None
	msg = b""
	while len(msg) < size:
		msg += s.recv(min(BUFFER_SIZE, size - len(msg)))
	if decode:
		msg = msg.decode()
	return msg


def handle_request(request: str):
	request = remove_whitespaces(request)
	command, *args = request.split(' ')
	command = command.lower()

	if not command:
		pass

	elif command in CREATE_JOBS:
		if len(args) != 4:
			print("Create takes exactly 4 arguments.")
			return
		name, graph_name, steps, approximate = args
		steps, approximate = safe_int(steps), safe_int(approximate)
		if None in (steps, approximate):
			print("Failed. 'steps' and 'num' should be numbers")
			return
		if defs.db_m.get_jobGroup(name):
			print(f"Failed. Name {name} is already registered.")
			return
		print('Started creating jobs, it may take a while...')
		res = defs.job_m.create_jobGroup(graph_name, steps, approximate, name)
		if type(res) is str:
			print(res)  # print error
		else:
			print(f"Done! A total of {res} jobs were created.")

	elif command in REGISTER_GRAPH:
		if len(args) != 2:
			print("Graph takes exactly 2 arguments.")
			return
		graph_name, graph_path = args
		graph = defs.db_m.get_graph(graph_name)
		if graph:
			print(f"Failed. Graph {graph_name} already exist.")
			return
		if not isfile(graph_path):
			print(f"Failed. Path {abspath(graph_path)} doesn't exist.")
			return
		defs.db_m.register_graph(graph_path, graph_name)

	elif command in UNREGISTER_GRAPH:
		if len(args) != 1:
			print("Unregister takes exactly 1 argument.")
			return
		graph_name = args[0]
		graph = defs.db_m.get_graph(graph_name)
		if not graph:
			print(f"Failed. Graph {graph_name} doesn't exist.")
			return
		jobGroups = defs.db_m.get_all_jobGroups()
		in_use = []
		for name, jobGroup in jobGroups.items():
			if jobGroup.get_graph() == graph_name and not jobGroup.is_completed():
				in_use.append(name)
		if in_use:
			print(f"Failed, Graph {graph_name} is used by uncompleted jobs {in_use}")
			return
		defs.db_m.unregister_graph(graph_name)

	elif command in START_JOBS:
		if len(args) != 1:
			print("Start takes exactly 1 argument.")
			return
		name = args[0]
		err = defs.job_m.add_jobGroup(name)
		if err:
			print(err)

	elif command in STOP_JOBS:
		if len(args) != 1:
			print("Stop takes exactly 1 argument.")
			return
		name = args[0]
		err = defs.job_m.remove_jobGroup(name)
		if err:
			print(err)

	elif command in DELETE_JOBS:
		if len(args) != 1:
			print("Delete takes exactly 1 argument.")
			return
		name = args[0]
		err = defs.job_m.delete_jobGroup(name)
		if err:
			print(err)

	elif command in RESCHEDULE:
		if len(args) not in (1, 2):
			print("Stop takes exactly 1 or 2 arguments.")
			return
		name = args[0]
		minutes = safe_int(args[1]) if len(args) > 1 else 0
		if minutes is None or minutes < 0:
			print('Failed. Time should be a positive integer (or 0).')

		jobGroup = defs.db_m.get_jobGroup(name)
		if not jobGroup:
			print(f"Failed. {name} not found.")
			return
		rescheduled = jobGroup.reschedule_long_waiting_jobs(minutes)
		if rescheduled > 0:
			print(f"Rescheduled {rescheduled} jobs.")

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
			print(HELP_MSG1)
			print(HELP_MSG2)
			print(HELP_MSG3)
			print('\n'.join([cmd_dict[cmd] for cmd in CMD_ORDER]))

	elif command in LIST_DATA:
		if len(args) > 1:
			print("List takes exactly 0 or 1 arguments.")
			return
		data_type = args[0].lower() if args else None
		if not data_type or data_type in {"graph", "graphs"}:
			data = defs.db_m.get_all_graphs()
			print("Available graphs:")
			print('\n'.join([f"\t{name}: \t{path}" for name, path in data.items()]))
		if not data_type or data_type in {"group", "groups"}:
			data = {name: jobGroup.get_percentage() for name, jobGroup in defs.db_m.get_all_jobGroups().items()}
			print("Available groups:")
			print('\n'.join([f"\t{name}: \t{perc:.2f}%" for name, perc in data.items()]))
		if not data_type or data_type in {"queue", "queues"}:
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
			print(f"Failed. {name} not found.")
			return
		print(f"\t{jobGroup.get_percentage():.4f}%")

	elif command in GET_LATEST_RESULTS:
		if len(args) != 1:
			print("Results takes exactly 1 argument.")
			return
		name = args[0]
		jobGroup = defs.db_m.get_jobGroup(name)
		if not jobGroup:
			print(f"Failed. {name} not found.")
			return
		res = jobGroup.get_result()
		print(f"Current results:   \t {res:,}")
		perc = jobGroup.get_percentage()
		if 0 < perc < 100:
			print(f"Estimated results: \t~{int(res / (perc/100)):,}")

	elif command in PRIORITY:
		if len(args) not in (0, 2):
			print("Priority takes exactly 0 or 2 arguments.")
			return
		elif not args:
			for i, name in enumerate(defs.job_m.get_queued_jobGroups()):
				print(f'\t{i+1}) \t{name}')
			pass
		else:
			name, prio = args
			prio = safe_int(prio)
			if prio is None:
				print("Failed. index should be a number.")
			err = defs.job_m.set_priority(name, prio)
			if err:
				print(err)

	elif command in CLOSE_APP:
		if len(args) >= 1:
			print("Close takes no arguments.")
			return
		exit()

	else:
		print(f'No such command: "{command}".')


def handle_client(c: socket, addr: (str, int)):
	data = recv(c)
	if not data:
		clients.remove(c)
		ndprint(f'Connection closed: {str(addr)}.')
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

		elif request == GET_GRAPH and len(args) == 1:
			graph_name = args[0]
			graph_file_path = defs.db_m.get_graph(graph_name)
			if not graph_file_path:
				ndprint(f'graph named "{graph_name}" (not registered) asked by {str(addr)}')
				return
			sendfile(c, graph_file_path)

		elif request == POST_RES and len(args) == 2:
			job_id, result = args
			int_results = [safe_int(arg) for arg in args]
			if None in int_results:
				ndprint(f"Got non-numbered result from {str(addr)}\n\tjob id = {job_id}, \tresult = {result}")
				return
			# TODO: delete files of done jobs?
			post_success, post_result = defs.job_m.post_result(*int_results)
			if post_result:
				name, result = post_result
				ndprint(f'NEW RESULT!\n\t{name}: \t{result:,}')
		elif request == UPDATE_JOB and len(args) == 1:
			job_id = args[0]
			print(f"Got update {job_id}")
			updated_job_path = f"{jobs_dir}update_{job_id}"
			recvfile(c, updated_job_path)
			if defs.job_m.update_job(int(job_id), updated_job_path):
				print(f"Updated {job_id}")
			remove(updated_job_path)


def auto_resched():
	global last_auto_resched
	if time() > last_auto_resched + SCHEDULER_CHECK_TIME:
		last_auto_resched = time()
		rescheduled = defs.job_m.reschedule_long_waiting_jobs()
		if rescheduled:
			ndprint(f"Automatically rescheduled {rescheduled} jobs.")


def main():
	global server_socket, clients
	server_socket, clients = None, []
	global last_auto_resched

	try:
		defs.db_m = DatabaseManager()
		defs.job_m = defs.db_m.get_jobManager()
		if not defs.job_m:
			defs.job_m = JobManager()
			defs.db_m.register_jobManager(defs.job_m)
			transaction.commit()

		last_auto_resched = time()

		server_socket = listen_on(SERVER_BIND, SERVER_PORT)
		socket2addr = {}

		print(WELCOME_MESSAGE)
		print("Enter 'Help' or 'H' for more details.\n")
		show_prompt()

		while True:
			r, _, _ = select([server_socket, stdin] + clients, [], [])
			for c in r:
				if c == server_socket:
					c, addr = server_socket.accept()
					socket2addr[c] = addr
					clients.append(c)
				elif c == stdin:
					handle_request(input())
					show_prompt()
				else:
					handle_client(c, socket2addr[c])

			auto_resched()
			transaction.commit()

	except Exception as e:
		ndprint('Error occurred: \n\t' + repr(e))

	finally:
		print("Start closing.")
		if defs.db_m:
			defs.db_m.close()
		if server_socket:
			server_socket.close()
		for c in clients:
			c.close()
		print("Done closing.")


if __name__ == '__main__':
	main()
