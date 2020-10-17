#include "pybind11/include/pybind11/pybind11.h"
#include "redelClient.h"
#include "redelServer.h"


PYBIND11_MODULE(example, m) {
    m.doc() = "pybind11 example plugin"; // optional module docstring
    m.def("can_i_finish_it", &canIFinishIt, "func1");
    m.def("jobs_creator", &jobsCreator, "func2");
    m.def("execute_job", &executeJob, "func3");
}

