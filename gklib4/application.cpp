/*
 * application.cpp
 *
 *  Created on: 6 Φεβ 2014
 *      Author: gkarak
 */

#include <stdlib.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include "application.h"

/**
 * Set default application characteristics
 */
void Application::GetAppInfo() {
	output_.p("SYSadmin Utilities");
	output_.p("(c) 2011-2014 George Karakostas");
	output_.p("Version 2.0.0");
}

/**
 * Execute a command and return output
 *
 * http://stackoverflow.com/questions/478898/how-to-execute-a-command-and-get-output-of-command-within-c
 * http://www.sw-at.com/blog/2011/03/23/popen-execute-shell-command-from-cc/
 */
int Application::Exec(const char* cmd, string& stdout) {
	FILE* pipe = popen(cmd, "r");
	if (!pipe) return -1;
	char buffer[128];
	stdout = "";
	while (!feof(pipe)) {
		if (fgets(buffer, 128, pipe))
			stdout += buffer;
	}
	return pclose(pipe) / 256;
}

int Application::Exec(string cmd, string& stdout) {
	return Exec(cmd.c_str(), stdout);
}

/**
 * Output the contents of a file
 *
 * http://www.cplusplus.com/forum/beginner/11304/#msg53361
 */
void Application::Cat(const char* filename) {
	string line;
	ifstream file(filename);
	if (file) {
		while (getline(file, line)) {
			cout << line << endl;
		}
		file.close();
	}
	// disabled; static function
	//else output_.p("Error opening file", "fail");
}

/**
 * Check that a file exists
 *
 * @see http://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exist-using-standard-c-c11-c
 */
bool Application::FileExists(string filename) {
	struct stat attrib;
	return (stat(filename.c_str(), &attrib) == 0);
}

/**
 * Check that a file exists & retrieve attributes
 *
 * @see http://stackoverflow.com/questions/13697941/how-to-get-file-creation-time-in-linux
 */
bool Application::FileExists(string filename, struct stat& attrib) {
	if (!&attrib) return false;
	return (stat(filename.c_str(), &attrib) == 0);
}

/**
 * Read configuration from file
 *
 * http://stackoverflow.com/questions/6892754/creating-a-simple-configuration-file-and-parser-in-c
 * http://www.cplusplus.com/reference/map/map/
 * http://www.cprogramming.com/tutorial/stl/stlmap.html
 * http://stackoverflow.com/questions/4844886/how-to-loop-through-a-c-map
 * https://apps.ubuntu.com/cat/applications/saucy/libconfig++9/
 */
void Application::ParseConf(string filename) {
	// append .conf if missing
	if ((filename.length() < 5) || ((filename.length() >= 5) && (filename.substr(filename.length() - 5, 5) != ".conf"))) {
		confname_ = filename;
		filename += ".conf";
	}
	else confname_ = filename.substr(0, filename.length() - 5);
	// open file
	ifstream file(filename.c_str());
	// line buffer
	string line;
	// category section
	string category;
	if (file) {
		while (getline(file, line)) {
			// put string in stream in order to use getline on it
			istringstream istr_line(line);
			string key;
			if (getline(istr_line, key, '=')) {
				// check if not a comment
				if ((key.substr(0, 1) != "#") && (key.substr(0, 1) != ";")) {
					// check if section
					if ((key.substr(0, 1) == "[") && (key.length() > 2)) {
						// ignore if global section
						if (key == "[global]") category = "";
						// store the section
						else category = key.substr(1, key.length() - 2) + "-";
					}
					// if not section
					else {
						key = category + key;
						string value;
						if (!getline(istr_line, value)) value = "";
						ParameterRecord* p = GetParam(key);
						if (p) p->set_value(value);
					}
				}
			}
		}
		file.close();
	}
	else output_.p("Error opening configuration file [" + filename + "]", "wrng");
}

/**
 * Get parameters
 *
 * If (conf) then parameters need to start with - or --.
 */
void Application::ParseParameters(int argc, char** argv, bool conf) {
	argv0_ = argv[0];
	bool conf_processed = false;
	for (int i = 1; i < argc; i++) {
		// put string into stream in order to use getline
		istringstream istr_line(argv[i]);
		string key;
		if (getline(istr_line, key, '=')) {
			// if parameter does not start with - then read conf file
			if (conf && (key.substr(0, 1) != "-")) {
				if (!conf_processed) {
					ParseConf(key);
					conf_processed = true;
				}
				else output_.p("Invalid argument [" + key + "]", "wrng");
			}
			// else is proper parameter
			else {
				// if starts with --
				if ((key.length() > 2) && (key.substr(0, 2) == "--"))
					// remove --
					key = key.substr(2, key.length() - 2);
				// if starts with -
				if ((key.length() > 1) && (key.substr(0, 1) == "-")) {
					for (unsigned int k = 1; k < key.length(); k++) {
						ParameterRecord* p = GetParam(GetParamFull(key[k]));
						if (p) p->set_value("1");
					}
				}
				// then it is a proper parameter
				else {
					string value;
					if (!getline(istr_line, value)) value = "1";
					ParameterRecord* p = GetParam(key);
					if (p) p->set_value(value);
				}
			}
		}
	}
}

/**
 * Define allowable parameters
 */
void Application::DefineParameters() {
	params_["help"] = ParameterRecord("help", 'h', "This help screen.");
	params_short_['h'] = "help";
	params_["verbose"] = ParameterRecord("verbose", 'v', "Print additional information.");
	params_short_['v'] = "verbose";
}

/**
 * Iterate parameters
 *
 * http://www.java2s.com/Code/Cpp/Data-Structure/Cyclethroughamapusinganiterator.htm
 */
void Application::ListParameters(bool conf) {
	cout << "Usage: " << argv0_ << " (options)" << (conf? " (configuration file)": "") << endl
			<< endl
			<< "Options:" << endl;
	for (map_sp::iterator i = params_.begin(); i != params_.end(); i++)
		cout << "\t" << i->second.descr() << endl;
}

/**
 * List parameter values
 */
void Application::ListParameterValues() {
	cout << "List of command line parameters" << endl;
	for (map_sp::iterator i = params_.begin(); i != params_.end(); i++)
		cout << i->first << ": " << i->second.value() << endl;
}

/**
 * Validate parameters
 */
bool Application::ValidateParameters() {
	bool r = true;
	for (map_sp::iterator i = params_.begin(); i != params_.end(); i++) {
		if (!i->second.Validate()) {
			output_.p("Missing parameter [" + i->first + "]", "fail");
			r = false;
		}
	}
	return r;
}

/**
 * Get parameter
 */
ParameterRecord* Application::GetParam(string key) {
	if (params_.find(key) != params_.end())
		return &(params_.find(key)->second);
	output_.p("Invalid parameter [" + key + "]", "wrng");
	return NULL;
}

/**
 * Get parameter name from short key
 */
string Application::GetParamFull(char ckey) {
	if (params_short_.find(ckey) != params_short_.end())
		return params_short_.find(ckey)->second;
	return string(1, ckey);
}


/**
 * Check if help is called
 */
bool Application::CheckParameter(string key) {
	ParameterRecord* p = GetParam(key);
	return (p && !p->value().empty());
}

/**
 * Get parameter value
 */
string Application::GetParamValue(string key) {
	ParameterRecord* p = GetParam(key);
	return p? p->value(): "";
}

/**
 * Lock application for a default time of 1 day
 */
bool Application::Lock(string lockname, int ttl) {
	lockname_ = lockname;
	string dir = "/tmp";
	string mtime;
	if (ttl > 0) mtime = " -mtime " + Output::Int(--ttl);
	string stdout;
	Application::Exec("find " + dir + " -maxdepth 1 -name '" + lockname +
			"-lock'" + mtime, stdout);
	// if not empty then lock file in place
	if (stdout.length()) return false;
	Application::Exec("touch " + dir + "/" + lockname + "-lock", stdout);
	return true;
}

/**
 * Unlock app
 */
void Application::Unlock() {
	string dir = "/tmp";
	string stdout;
	Application::Exec("rm -f " + dir + "/" + lockname_ + "-lock", stdout);
}

/**
 * MAIN
 */
int Application::Init(int argc, char** argv, bool conf) {
	DefineParameters();
	GetAppInfo(); // run before Parse because it may fire warning for invalid parameter
	ParseParameters(argc, argv, conf);
	if (CheckParameter("help")) {
		ListParameters(conf);
		return 0;
	}
	if (!ValidateParameters()) {
		ListParameters(conf);
		return 1;
	}
	if (CheckParameter("verbose"))
		ListParameterValues();
	int r = Main();
	output_.Close();
	return r;
}
