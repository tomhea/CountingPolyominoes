global job_m, db_m
job_m = None
db_m = None

NO_JOB_FOUND = "NONE"
JOB_FOUND = "JOB"

GET_JOB = "GET"
POST_RES = "POST"
GET_GRAPH = "GRAPH"
UPDATE_JOB = "UPDATE"
CLOSE_CON = ""

CREATE_JOBS = ("create",)
REGISTER_GRAPH = ("graph",)
START_JOBS = ("start", "add")
STOP_JOBS = ("stop",)
HELP = ("help", 'h')
LIST_DATA = ("list", "l")
GET_PERCENTAGE = ("percentage", "%")
GET_LATEST_RESULTS = ("results", "res", "#")
RESCHEDULE = ("reschedule", "resched")
PRIORITY = ("priority", "prio")
CLOSE_APP = ("exit",'close','quit','q')

cmd_dict = {
CREATE_JOBS:"""\tCreate name graph steps num
		- Creates (About) 'num' jobs (Jobs/[name]/[name]_0,1,2,..) for calculating
		  the number of subgraphs of the registered 'graph' with 'steps' connected nodes.
		  The created jobs are registered in the server's database under 'name'.""",
REGISTER_GRAPH:"""\tGraph name path
		- Register the graph in 'path' under 'name'.""",
START_JOBS:"""\tStart [Add] name
		- Start (or continue) working on the jobs registered under 'name'.""",
STOP_JOBS:"""\tStop name
		- Stop (pause) working on the jobs registered under 'name'.""",
RESCHEDULE:"""\tReschedule [Resched] name (time)
        - Requeue jobs registered under 'name' that are alive for more then 'time' minutes.
          If 'time' isn't specified, all active jobs will be requeued.""",
HELP:"""\tHelp [H] (command)
		- Prints command explanation.
		  if command isn't specified - Prints all possible commands, with explanation.""",
LIST_DATA:"""\tList [L] (optional graph|group|queue)
		- Prints data about groups and graphs, leave empty for all data, or filter by optional.""",
GET_PERCENTAGE:"""\tPercentage [%] name
		- Prints the percentage (%) of the completed jobs registered under 'name'.""",
GET_LATEST_RESULTS:"""\tResults [Res, #] name
		- Prints the current total results of the jobs registered under 'name'.""",
PRIORITY:"""\tPriority [Prio] (name, index)
		- If no arguments given - prints all active job groups by their priority.
		  Else - moves jobs registered under 'name' to priority 'index'.""",
CLOSE_APP:"""\tClose [Exit, Quit, Q]
		- Exits app, closes server and database """
}

WELCOME_MESSAGE = "Welcome to SubgraphCounter Server-App!"
CMD_ORDER = [CREATE_JOBS, REGISTER_GRAPH, START_JOBS, STOP_JOBS, RESCHEDULE, HELP, LIST_DATA, GET_PERCENTAGE, GET_LATEST_RESULTS, PRIORITY, CLOSE_APP]


SERVER_BIND = '0.0.0.0'
SERVER_PORT = 36446

BUFFER_SIZE = 1024

SCHEDULER_CHECK_TIME = 60   # seconds
