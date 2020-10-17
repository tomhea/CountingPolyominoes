from redelClient import *
from sys import argv, stdin
from select import select
from socket import socket


# job_base_name = argv[1]
# num_of_jobs = int(argv[2])

# jobs_dir = "Jobs/"
# graphs_dir = "Graphs/"


# summed = 0
# for i in range(num_of_jobs):
# 	counted = execute_job(graphs_dir + "Polyominoes.txt", jobs_dir + job_base_name + "_" + str(i))
# 	# print(counted)
# 	summed += counted
# print(summed)


def connect_to(ip, port):
	s = socket()
	s.connect((ip, port))
	return s


def handle_request(s : socket, request : str):
	s.send(request.encode())
	print(s.recv(1024).decode())


def main():
	s = connect_to('127.0.0.1', 36446)
	while True:
		r,_,_ = select([stdin], [], [])
		if stdin in r:
			handle_request(s, input())


if __name__ == '__main__':
	main()
