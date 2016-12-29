/*
 * backupapp.h
 *
 * SYSadmin Backup Application class
 *
 *  Created on: 21 Ιαν 2014
 *      Author: gkarak
 */

#ifndef BACKUPAPP_H_
#define BACKUPAPP_H_

#include <application.h>

class BackupApp:
		public Application {
// Constructors
public:
	BackupApp() {
		incremental_ = true;
		ignore_existing_ = false;
		mysqldump_ = false;
		postgresdump_ = false;
		destination_quota_ = 0;
		verbose_ = true;
		dry_run_ = true;
	};

// Members
private:
	string name_;
	string type_;
	bool incremental_;
	bool ignore_existing_;
	bool mysqldump_;
	bool postgresdump_;
	string directories_;
	string excludes_;
	string destination_;
	string destination_type_;
	string destination_device_;
	string destination_encrypted_key_;
	string destination_encrypted_map_;
	string destination_backup_dir_;
	string destination_database_;
	int destination_quota_;
	bool verbose_;
	bool dry_run_;

// Methods
protected:
	void GetAppInfo();
	void DefineParameters();
	bool ValidateParameters();
	int Main();

private:
	int MySqlDump();
	int PostGresDump();
	bool SnarFileStatus(string snarfile);
	int BackupTar();
	int BackupRsync();
	int Crypt(string cmd);
	int Mount();
	void UnMount();
	void TimeStat(time_t t_start);

};

#endif /* BACKUPAPP_H_ */
