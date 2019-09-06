#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iomanip> 

#include "re2/re2.h"
#include "xre.h"
#include "utils.h"
#include "xlog.h"

using namespace std;
using namespace xdb;

#define VVV_INIT_IN_THREAD
const int nthread = 1;

void printUsage(const char *msg=nullptr) {
	if (msg) printf("%s \n",msg);
	std::cout << "xre -rx <RegEx string> \n";
	std::cout << "      [-h    This usage help]\n";
	// Hidden options - uused for tests
	// std::cout << "      [-cmp   Compare Results]\n";
	// std::cout << "      [-pages  <Number of Pages>]\n";
	std::cout << "      [-if   <Input file name; If omitted, STDIN assumed>]\n";
	std::cout << "      [-of   <Output file name; If omitted, STDOUT assumed>]\n";
	std::cout << "      [-lf   <Log file name; If omitted, STDOUT assumed>]\n";  
}


struct thctxt_t {
	int myid;
	std::string testfile_dir;
	std::vector<std::string> *regex_patterns;
	std::vector<std::string> *input_files;
	std::vector<int> *xre_results;
	std::vector<double> *xre_exec_times;
};

void *runxre(void *arg)
{
	thctxt_t *ctxt = (thctxt_t *) arg;
	XTimer timer;

#ifdef VVV_INIT_IN_THREAD
	// Init the FPGA - one time cos
	timer.start();
	if (! XRE::InitFpga ("regex_aws_hw.xclbin", "regex_aws")) {
		std::cout << "FPGA Init failed. " << std::endl; 
		return 0;
	}
	timer.end();
	double t_fpga = timer.duration();
#endif

	std::cout << "Thread: " << ctxt->myid 
		<< " work size " << ctxt->regex_patterns->size() << std::endl;

	for (int idx = 0; idx < (int) ctxt->regex_patterns->size(); idx++) {
		if (idx % nthread == ctxt->myid) {
			std::string regex_str = (*ctxt->regex_patterns)[idx];
			std::string input_file = ctxt->testfile_dir;
			input_file.append((*ctxt->input_files)[idx]);
			std::string output_file("xre_out_");
			output_file.append(std::to_string(idx)).append(".log");

			std::cout << "Running xre '" << regex_str << "' on " 
				<< idx << "-th file " << input_file << std::endl;

			(*ctxt->xre_results)[idx] = 0; 
			(*ctxt->xre_exec_times)[idx] = 0;

			for (int i = 0; i < 10; i++) {
				string outf_ith = output_file + "." + std::to_string(i);
				std::cout << "Run xre " << i << "-th time, output to " << outf_ith << std::endl;
				// regex compiler and load instructions
				timer.start();
				XRE xre(regex_str);
				if (!xre.ok()) {
					L_(lerror) << "XRE Invalid regex " << regex_str;
					continue;
				}
				// execute regex
				if (!xre.FullMatch(input_file, outf_ith)) { 
					std::cout << "FullMatch fail? What does this mean?" << std::endl;
					continue;
				}
				if (!xre.ok()) {
					L_(lerror) << "Full match ran with error";
					continue;
				}    
				timer.end();
				(*ctxt->xre_results)[idx] = 1; 
				(*ctxt->xre_exec_times)[idx] = timer.duration();
			}
		}
	}

#ifdef VVV_INIT_IN_THREAD
	timer.start();
	XRE::EndFpga();
	t_fpga += timer.duration();
	std::cout << "FPGA Init/Deinit total " << t_fpga << " timeunit." << std::endl;
#endif

	return 0;
}


int main (int argc, char* argv[]) {
	ArgParser inp(argc, argv);
	std::string optValue;

	if (inp.getCmdOption("-h")){
		printUsage();
		return 0;
	}

	std::string testfile_dir("./regex_testdata/");
	if (inp.getCmdOption("-tf",optValue)) {
		testfile_dir = optValue;
	}
	std::cout << "Using dir " << testfile_dir << std::endl;

	// Now execute the regex 
	std::vector<std::string> regex_patterns = {".*final.*"}; 
	std::vector<std::string> input_files = {"nation.tbl"};
	if (inp.getCmdOption("-rx",optValue)) {
		regex_patterns.resize(1);
		regex_patterns[0] = optValue;
	}

	if (inp.getCmdOption("-if",optValue)) {
		input_files.resize(1);
		input_files[0] = optValue;
	}
	std::cout << "Run regex on " << input_files.size() << " files, first is " << input_files[0] << " regex is " << regex_patterns[0] << std::endl;

	if (!initLogger("run_tests.log", linfo)) {
		std::cout << "Error: Cannot open logfile for writing" << endl;
		return 0;
	}

	XTimer timer;
	int total_tests = regex_patterns.size();

	std::vector<int> xre_results;
	xre_results.resize(total_tests);
	std::vector<double> xre_exec_times;
	xre_exec_times.resize(total_tests);  

	// Init the FPGA - one time cost
	timer.start();
#ifndef VVV_INIT_IN_THREAD
	if (! XRE::InitFpga ("regex_aws_hw.xclbin", "regex_aws")) {
		L_(lerror) << "FPGA Open failed";
		return 0;
	}
#endif
	timer.end();
	double t_fpga = timer.duration();

	pthread_t threads[nthread];
	thctxt_t thctxt[nthread];
	for (int i = 0; i < nthread; i++) {
		thctxt[i].myid = i;
		thctxt[i].testfile_dir = testfile_dir;
		thctxt[i].regex_patterns = &regex_patterns;
		thctxt[i].input_files = &input_files;
		thctxt[i].xre_results = &xre_results;
		thctxt[i].xre_exec_times = &xre_exec_times;

		std::cout << "Start thread " << i << std::endl;
		if (pthread_create(&threads[i], NULL, runxre, &thctxt[i])) {
			L_(lerror) << "Cannot create threads.";
			return -1;
		}
	}
	for (int i = 0; i < nthread; i++) {
		if (pthread_join(threads[i], NULL)) {
			L_(lerror) << "Cannot join threads.";
			return -2;
		}
	}

	timer.start();
#ifndef VVV_INIT_IN_THREAD
	XRE::EndFpga();
#endif
	timer.end();
	t_fpga += timer.duration();

	// Now run the RE2 design 
	std::vector<bool> re2_results;
	re2_results.resize(total_tests);
	std::vector<double> re2_exec_times;
	re2_exec_times.resize(total_tests);

	for (int idx=0; idx<total_tests; idx++) {

		std::string regex_str = regex_patterns[idx];
		std::string input_file = testfile_dir;
		input_file.append(input_files[idx]);

		std::string output_file("re2_out_");
		output_file.append(std::to_string(idx)).append(".log");

		re2_results[idx] = false;
		re2_exec_times[idx] = 0;
		timer.start();

		RE2 re(regex_str);
		if (!re.ok()) {
			L_(lerror) << "RE2 Invalid regex " << regex_str;
			continue;
		}
		std::ifstream ifs;
		ifs.open(input_file, std::ifstream::in | std::ifstream::binary);
		if(!ifs.good()) {
			L_(lerror) << "Couldn't open input_file: " << input_file;
			continue;
		}

		std::ofstream ofs;
		ofs.open(output_file, std::ofstream::out | std::ofstream::binary);    
		if(!ofs.good()) {
			L_(lerror) << "Couldn't open output file: " << output_file;
			return 0;
		}

		std::string in_str;
		while(std::getline(ifs, in_str)) {
			if (RE2::FullMatch(in_str, re)) { 
				ofs << in_str << std::endl;
			}
		}

		timer.end();
		re2_results[idx] = true;
		re2_exec_times[idx] = timer.duration();
	}

	// Print Runtime Stats
	L_(linfo) << "Runtimes (ms): " << std::endl;
	L_(linfo) << std::setw(20) << "FPGA Setup" << std::setw(20) << std::fixed << std::setprecision(8) << 1000*t_fpga<< std::endl;

	double aggr_speedup = 0;
	for (int idx=0; idx<total_tests; idx++) {
		L_(linfo) << idx << " " << std::setw(10) << "XRE" << std::setw(4) << (xre_results[idx]?"P":"F")
			<< std::setw(10) << std::fixed << std::setprecision(3) << 1000*xre_exec_times[idx]
			<< std::setw(10) << "RE2" << std::setw(4) << (re2_results[idx]?"P":"F")
			<< std::setw(10) << std::fixed << std::setprecision(3) << 1000*re2_exec_times[idx]
			<< " Speedup=" << std::setw(5) << std::fixed << std::setprecision(2) << re2_exec_times[idx]/xre_exec_times[idx] ;
		aggr_speedup += re2_exec_times[idx]/xre_exec_times[idx];
	}
	double avg_speedup = aggr_speedup/total_tests;
	L_(linfo) << "Avg Speedup=" << std::setw(5) << std::fixed << std::setprecision(2) << avg_speedup ;
	endLogger();

	return 0;
}

