/*
 * application.h
 *
 * Console application framework
 *
 *  Created on: 6 Φεβ 2014
 *      Author: gkarak
 *
 * To use:
 * - Target project properties, C/C++ Build, settings, tool settings:
 * - GCC Linker, libraries:
 * - add search path: "${workspace_loc:/gklib4/Debug}",
 * - add library: gklib4
 * - GC++ Compiler, Includes: "${workspace_loc:/gklib4}"
 * - #include <application.h>
 *
 *  http://unity.ubuntu.com/wp-content/uploads/2012/03/cppguide.html
 *  http://stackoverflow.com/questions/8332460/how-do-i-include-a-statically-linked-library-in-my-eclipse-c-project
 *  http://stackoverflow.com/questions/9078513/how-do-you-properly-link-to-a-static-library-using-g
 *
 *  Create debian package:
 *  http://askubuntu.com/questions/170348/how-to-make-my-own-local-repository
 *  http://askubuntu.com/questions/76963/auto-compile-service-on-for-ubuntu-packages?lq=1
 */

#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <string>
#include <map>
#include "output.h"
#include "parameterrecord.h"

using namespace std;

class Application {

// Type definitions
private:
	typedef map <string, ParameterRecord> map_sp;
	typedef map <char, string> map_cs;

// Constructors
public:
	Application() {};
	virtual ~Application() {}; // Virtual destructor necessary for virtual functions

// Members
protected:
	map_sp params_;
	map_cs params_short_;

private:
	Output output_;
	string argv0_;
	string confname_;
	string lockname_;

// Methods
public:
	Output* output() { return& output_; };
	string argv0() { return argv0_; };
	string confname() { return confname_; };

	static int Exec(const char* cmd, string& stdout); // Execute a shell command
	static int Exec(string cmd, string& stdout); // Execute a shell command
	static void Cat(const char* filename); // Get file contents
	static bool FileExists(string filename);
	static bool FileExists(string filename, struct stat& attrib);

	int Init(int argc, char** argv, bool conf);
	virtual int Main() { return 0; };

protected:
	ParameterRecord* GetParam(string key);
	string GetParamFull(char ckey); // get paramater from short
	bool CheckParameter(string key); // Check parameter exists
	string GetParamValue(string key);
	virtual void GetAppInfo(); // Get App info
	virtual void DefineParameters(); // Define allowable parameters
	virtual bool ValidateParameters();
	bool Lock(string lockname, int ttl = 1); // Lock app for a default of 1 day
	void Unlock();

private:
	void ParseConf(string filename); // Get configuration file into parameters
	void ParseParameters(int argc, char** argv, bool conf);
	void ListParameters(bool conf);
	void ListParameterValues();

};

#endif /* APPLICATION_H_ */
