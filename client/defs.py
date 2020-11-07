from sys import stdout


NO_JOB_FOUND = "NONE"
JOB_FOUND = "JOB"

GET_JOB = "GET"
POST_RES = "POST"
GET_GRAPH = "GRAPH"
UPDATE_JOB = "UPDATE"

graphs_dir = "./Graphs/"
jobs_dir = "./Jobs/"


GET_STATISTICS = ("statistics", "stats", "s")
HELP = ("help", 'h')
SET_PRINTS = ("prints", "p")
UPDATE = ("update", "up", "^")
CLOSE_APP = ("close", "exit", "x", "quit", "q")

cmd_dict = {
GET_STATISTICS:"""\tStatistics [Stats, S]
		- Prints statistics about the current session.""",
HELP:"""\tHelp [H] (command)
		- Prints command explanation.
		  If command isn't specified - Prints all possible commands, with explanation.""",
SET_PRINTS:"""\tPrints [P] (on|off)
		- Set prints on/off. Important messages will still be displayed.
		  If operation isn't specified, the current condition is printed.
		  Set prints off with operation {off, -, 0}, and on with {on, +, 1}.""",
UPDATE:"""\tUpdate [Up, ^]
		- Sends the server non-finished results.
		  Do it if the app will be close soon (low battery etc.).""",
CLOSE_APP:"""\tClose [Exit, X, Quit, Q]
		- Sends non-finished results, prints closing statistics, and exits app.
		  Use this command, as otherwise non-finished results will be lost,
		  and temporary files won't be deleted from your computer."""
}

WELCOME_MESSAGE = "Welcome to SubgraphCounter Client-App!"
HELP_MSG1 = "List of all possible commands:"
HELP_MSG2 = " The [] specifies more names for the same command (commands are not case sensitive),"
HELP_MSG3 = " and the arguments are placed afterwords. The () specifies optional arguments."
CMD_ORDER = [GET_STATISTICS, HELP, SET_PRINTS, UPDATE, CLOSE_APP]

INPUT_PROMPT = {'utf-32': '⛟ ', 'utf-16': '⛟ ', 'utf-8': '» '}.get(stdout.encoding, '~ ')
RESULT_EXCEPTION = -1

SERVER_IP = '127.0.0.1'
SERVER_PORT = 36446

BUFFER_SIZE = 1024
