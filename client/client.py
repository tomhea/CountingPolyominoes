import socket
from redelClient import *
from sys import argv


steps = int(argv[1])
job_name = argv[2]
num_of_jobs = int(argv[3])

jobs_dir = "C:/Users/tomhe/OneDrive - Technion/Documents/CountingPolyominoes/Jobs/"


summed = 0
for i in range(num_of_jobs):
	counted = execute_job(str(steps), jobs_dir + job_name + "_" + str(i))
	print(counted)
	summed += counted
print(summed)
