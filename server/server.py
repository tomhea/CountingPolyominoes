import socket
from redelServer import *
from sys import argv


try:
	steps = int(argv[1])
except:
	steps = 7

try: 
	approx_num_of_jobs = int(argv[2])
except:
	approx_num_of_jobs = 10

jobs_dir = "C:/Users/tomhe/OneDrive - Technion/Documents/CountingPolyominoes/Jobs/"

print(jobs_creator(str(steps), steps, approx_num_of_jobs, jobs_dir + "my" + str(steps) + "Job"))

# print(can_i_finish_it(str(steps), steps))
