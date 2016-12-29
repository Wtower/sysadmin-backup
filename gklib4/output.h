/*
 * output.h
 *
 *  Created on: 7 Φεβ 2014
 *      Author: gkarak
 */

#ifndef OUTPUT_H_
#define OUTPUT_H_

#include <string>
#include <sstream>

using namespace std;

class Output {
public:
	// Constructors
	Output() {};
	~Output() { Close(); };

private:
	// Members
	string filename_;
	ostringstream stream_;
	string recipient_;
	string subject_;

	// Methods
private:
	// default values go in .h
	// http://stackoverflow.com/questions/14902129/default-parameters-in-h-and-cpp-files
	static string Colour(string colour = "");

public:
	void set_filename(string filename) { filename_ = filename; };
	void set_recipient(string recipient) { recipient_ = recipient; };
	// TODO strip subject
	void set_subject(string subject) { subject_ = subject; };
	static string Status(string msg, string status = "ok", bool colour = true);
	void p(string msg, string status = "",
			bool newline = true, bool log = true);
	static string Replace(string& s, string to_replace, string replace_with);
	static string Date(string format = "%Y-%m-%d", struct tm* date = NULL);
	static string DateT(string format, time_t t);
	static string Int(int n);
	void Close();

	// in future, overload (more complicated)
	// http://www.programming-techniques.com/2011/11/stream-operator-insertion-and.html
	// http://stackoverflow.com/questions/8415890/c-chaining-of-the-operator-for-stdcout-like-usage
	//friend ostream &operator<<(ostream& stream, const Output& o);

};

#endif /* OUTPUT_H_ */
