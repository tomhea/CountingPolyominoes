global job_m, db_m
job_m = None
db_m = None

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
