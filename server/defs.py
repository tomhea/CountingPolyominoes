from sys import stdout


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

graphs_dir = "./Graphs/"
jobs_dir = "./Jobs/"

CREATE_JOBS = ("create",)
REGISTER_GRAPH = ("graph",)
UNREGISTER_GRAPH = ("ungraph",)
START_JOBS = ("start", "add")
STOP_JOBS = ("stop",)
DELETE_JOBS = ("delete",)
HELP = ("help", 'h')
LIST_DATA = ("list", "l")
GET_PERCENTAGE = ("percentage", "%")
GET_LATEST_RESULTS = ("results", "res", "#")
RESCHEDULE = ("reschedule", "resched")
PRIORITY = ("priority", "prio")
CLOSE_APP = ("close", "exit", "x", "quit", "q")

cmd_dict = {
CREATE_JOBS:"""\tCreate name graph steps num
		- Creates (About) 'num' jobs (Jobs/[name]/[name]_0,1,2,..) for calculating
		  the number of subgraphs of the registered 'graph' with 'steps' connected nodes.
		  The created jobs are registered in the server's database under 'name'.""",
DELETE_JOBS:"""\tDelete name
        - Deletes the data, jobs and folder that registered under 'name.""",
START_JOBS:"""\tStart [Add] name
		- Start (or continue) working on the jobs registered under 'name'.""",
STOP_JOBS:"""\tStop name
		- Stop (pause) working on the jobs registered under 'name'.""",
REGISTER_GRAPH:"""\tGraph name path
		- Register the graph in 'path' under 'name'.""",
UNREGISTER_GRAPH:"""\tUngraph name
		- Unegister the graph named 'name'.""",
RESCHEDULE:"""\tReschedule [Resched] name (time)
		- Requeue jobs registered under 'name' that are alive for more then 'time' minutes.
		  If 'time' isn't specified, all active jobs will be requeued.""",
HELP:"""\tHelp [H] (command)
		- Prints command explanation.
		  If command isn't specified - Prints all possible commands, with explanation.""",
LIST_DATA:"""\tList [L] (graph|group|queue)
		- Prints data about graphs, groups and queued groups.
		  Filter the data by the optional argument.""",
GET_PERCENTAGE:"""\tPercentage [%] name
		- Prints the percentage (%) of the completed jobs registered under 'name'.""",
GET_LATEST_RESULTS:"""\tResults [Res, #] name
		- Prints the current total results of the jobs registered under 'name'.""",
PRIORITY:"""\tPriority [Prio] (name, index)
		- If no arguments given - prints all active job groups by their priority.
		  Else - moves jobs registered under 'name' to priority 'index'.""",
CLOSE_APP:"""\tClose [Exit, X, Quit, Q]
		- Exits app, closes server and database."""
}

WELCOME_MESSAGE = "Welcome to SubgraphCounter Server-App!"
HELP_MSG1 = "List of all possible commands:"
HELP_MSG2 = " The [] specifies more names for the same command (commands are not case sensitive),"
HELP_MSG3 = " and the arguments are placed afterwords. The () specifies optional arguments."
CMD_ORDER = [START_JOBS, STOP_JOBS, CREATE_JOBS, DELETE_JOBS, REGISTER_GRAPH, UNREGISTER_GRAPH, RESCHEDULE, HELP, LIST_DATA, GET_PERCENTAGE, GET_LATEST_RESULTS, PRIORITY, CLOSE_APP]

INPUT_PROMPT = {'utf-32': '⛟ ', 'utf-16': '⛟ ', 'utf-8': '» '}.get(stdout.encoding, '~ ')
SEND_TIMEOUT = 1

SERVER_BIND = '0.0.0.0'
SERVER_PORT = 36446

BUFFER_SIZE = 1024

SCHEDULER_CHECK_TIME = 60   # seconds
