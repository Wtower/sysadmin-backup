/*
 * parameterrecord.h
 *
 *  Created on: 7 Φεβ 2014
 *      Author: gkarak
 */

#ifndef PARAMETERRECORD_H_
#define PARAMETERRECORD_H_

#include <stdlib.h>
#include <string>

using namespace std;

class ParameterRecord {
public:
	// Enums
	enum Types {
		TypeBool,
		TypeInt,
		TypeStr
	};

	// Constructors
	ParameterRecord() {
		name_short_ = '\0';
		type_ = TypeBool;
		required_ = false;
	};

	ParameterRecord(
			string name_long,
			char name_short = '\0',
			string descr = "",
			Types t = TypeBool,
			bool required = false,
			string value = "")
	{
		name_long_ = name_long;
		name_short_ = name_short;
		descr_ = descr;
		type_ = t;
		required_ = required;
		value_ = value;
	};

private:
	// Members
	string name_long_;
	char name_short_;
	string descr_;
	Types type_;
	bool required_;
	string value_;

public:
	// Methods
	void set_required() { required_ = true; };
	void set_value(string value) { value_ = value; };
	string value() { return value_; };

	bool value_bool() {
		return ((type_ == TypeBool) &&
				((value_ == "1") || (value_ == "on") || (value_ == "true")));
	};

	int value_int() {
		if (type_ != TypeInt) return 0;
		else return atoi(value_.c_str());
	}

	string descr() {
		return (name_short_? "-" + string(1, name_short_) + ", ": "") +
			"--" + name_long_+ ": " + descr_;
	};

	bool Validate() {
		return (!required_  || (required_ && !value_.empty()));
	};

};

#endif /* PARAMETERRECORD_H_ */
