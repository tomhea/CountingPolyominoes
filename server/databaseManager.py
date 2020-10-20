from jobManager import JobStatus, JobGroup, JobManager

from ZODB import FileStorage, DB
import transaction


class MyZODB(object):
	def __init__(self, path):
		self.storage = FileStorage.FileStorage(path)
		self.db = DB(self.storage)
		self.connection = self.db.open()
		self.dbroot = self.connection.root()

	def close(self):
		self.connection.close()
		self.db.close()
		self.storage.close()


class DatabaseManager:
	def __init__(self):
		self.jobsDB = MyZODB('./db/jobs.fs')				# id   -> jobStatus
		self.jobGroupsDB = MyZODB('./db/jobGroups.fs')		# name -> jobGroup
		self.jobManagerDB = MyZODB('./db/jobManager.fs')	# self is jobManager

	def register_jobStatus(self, jobStatus: JobStatus):
		self.jobsDB.dbroot[jobStatus.job_id] = jobStatus

	def get_jobStatus(self, job_id: int):
		return self.jobsDB.dbroot[job_id]

	def register_jobGroup(self, jobGroup: JobGroup):
		self.jobGroupsDB.dbroot[jobGroup.name] = jobGroup

	def get_jobGroup(self, name: str):
		jobGroup = self.jobsDB.dbroot[name]
		jobGroup.reload_dict()
		return jobGroup

	def register_jobManager(self, jobManager : JobManager):
		self.jobManagerDB.dbroot['self'] = jobManager

	def get_jobManager(self):
		jobManager = self.jobManagerDB.dbroot['self']
		jobManager.reload_dict()
		return jobManager
