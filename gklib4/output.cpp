/*
 * output.cpp
 *
 *  Created on: 7 Φεβ 2014
 *      Author: gkarak
 */

#include <stdio.h>
#include <iostream>
#include <fstream>
#include "output.h"
#include "application.h"

/**
 * Get colour code for terminal
 *
 * http://www.tldp.org/HOWTO/Bash-Prompt-HOWTO/x329.html
 */
string Output::Colour(string colour) {
	string r = "\033[";
	if		(colour == "black") r += "0;30";
	else if (colour == "blue") r += "0;34";
	else if (colour == "green") r += "0;32";
	else if (colour == "cyan") r += "0;36";
	else if (colour == "red") r += "0;31";
	else if (colour == "purple")	r += "0;35";
	else if (colour == "brown") r += "0;33";
	else if (colour == "light-gray") r += "0;37";
	else if (colour == "dark-gray") r += "1;30";
	else if (colour == "light-blue") r += "1;34";
	else if (colour == "light-green")r += "1;32";
	else if (colour == "light-cyan") r += "1;36";
	else if (colour == "light-red") r += "1;31";
	else if (colour == "light-purple") r += "1;35";
	else if (colour == "yellow") r += "1;33";
	else if (colour == "white") r += "1;37";
	else r += "0";
	return r + "m";
}

/**
 * Output a status message
 */
string Output::Status(string msg, string status, bool colour) {
	string c, c2;
	if (colour) {
		if 		(status == "ok") c = Colour("light-green");
		else if (status == "fail") c = Colour("light-red");
		else if (status == "wrng") c = Colour("yellow");
		else if (status == "note") c = Colour("light-blue");
		else c = Colour("light-blue");
		c2 = Colour("");
	}
	if 		(status.length() == 1) status = "[  " + status + " ]";
	else if (status.length() == 2) status = "[ " + status + " ]";
	else if (status.length() == 3) status = "[ " + status + "]";
	else if (status.length() > 3) status = "[" + status.substr(0, 4) + "]";
	// print message
	// http://stackoverflow.com/questions/10865957/c-printf-with-stdstring
	// maximum 400 characters!
	char buf[500];
	sprintf(buf, "%-73s", msg.substr(0, 400).c_str());
	msg = buf;
	// if too big change line
	if (msg.length() > 73) {
		sprintf(buf, "\n%-73s", "");
		msg += buf;
	}
	// print status
	return msg + c + status + c2;
}

/**
 * Print a message or status to console and to stream.
 */
void Output::p(string msg, string status, bool newline, bool log) {
	if (!status.empty()) cout << Status(msg, status);
	else cout << msg;
	if (newline) cout << endl;
	if (log) {
		if (!status.empty()) stream_ << Status(msg, status, false);
		else stream_ << msg;
		if (newline) stream_ << endl;
	}
}

/**
 * Replace string with another
 *
 * @see http://stackoverflow.com/questions/5878775/how-to-find-and-replace-string
 */
string Output::Replace(string& s, string to_replace, string replace_with) {
	size_t n, pos = 0;
	size_t len_to = to_replace.length();
	size_t len_with = replace_with.length();
	do {
		n = s.find(to_replace, pos);
		pos = n + len_with;
		if (n != string::npos)
			s.replace(n, len_to, replace_with);
	} while (n != string::npos);
	return s;
}


/**
 * Format Date.
 *
 * Week day is 0-based.
 *
 * @see http://www.cplusplus.com/reference/ctime/strftime/
 */
string Output::Date(string format, struct tm* date) {
	time_t t = time(0);
	if (!date) date = localtime(&t);
	char r[32];
	if (strftime(r, sizeof(r), format.c_str(), date)) return r;
	else return "";
}

string Output::DateT(string format, time_t t) {
	struct tm* date = localtime(&t);
	return Output::Date(format, date);
}


/**
 * Format Integer
 */
string Output::Int(int n) {
	stringstream s;
	s << n;
	return s.str();
}

/**
 * Close stream_ and write log file, send mail
 *
 * http://www.cplusplus.com/forum/windows/88843/
 */
void Output::Close() {
	if (!recipient_.empty() && filename_.empty())
		filename_ = "/tmp/sysadmin-backup.log";
	if (!filename_.empty()) {
		ofstream file(filename_.c_str());
		if (file) {
			file << stream_.str();
			file.close();
		}
		else p("Error writing file [" + filename_ + "]", "wrng");
	}
	if (!recipient_.empty()) {
		string stdout;
		Application::Exec(
				"cat " + filename_ +
				"| mail -s \"" + subject_ + "\" " +
				recipient_, stdout);
	}
	filename_ = "";
	recipient_ = "";
}
