from ctypes import *
import pathlib
import os


# os.chdir(r"C:\FastAccess\py-cpp_integration_project".replace('\\', '/')))
os.add_dll_directory(r"C:\Users\tomhe\OneDrive - Technion\Documents\CountingPolyominoes")
libname = str(pathlib.Path().absolute() / "libRedelServer.dll").replace('\\', '/')
# libname = str(r"C:\FastAccess\py-cpp_integration_project\libRedelServer.dll").replace('\\', '/')
print(libname)
redel_server_lib = cdll.LoadLibrary(libname)

def cpp_str(s):
	return create_string_buffer(s.encode('utf-8'))


# bool canIFinishIt(const char* graphFilePath, u32 steps, u64* result);
def can_i_finish_it(graph_file_path : str, steps : int) -> (bool, int):
	result = c_ulonglong()
	redel_server_lib.canIFinishIt.restype = c_bool
	can_i = redel_server_lib.canIFinishIt(cpp_str(graph_file_path), steps, byref(result))
	return (can_i, result.value)

# int mainServer(const char* graphFilePath, u32 steps, int approxNumOfJobs, const char* jobBasePath);
def jobs_creator(graph_file_path : str, steps : int, approx_num_of_jobs : int, job_base_path : str) -> int:
	num_of_jobs = redel_server_lib.jobsCreator(cpp_str(graph_file_path), steps, approx_num_of_jobs, cpp_str(job_base_path))
	return num_of_jobs

print(can_i_finish_it("", 7))
