/*
 * backupapp.cpp
 *
 * @see Main()
 *
 *  Created on: 21 Ιαν 2014
 *      Author: gkarak
 */

#include <stdio.h>
#include <sys/stat.h>
#include <iostream>
#include "backupapp.h"

void BackupApp::GetAppInfo() {
	output()->p("SYSadmin Backup");
	output()->p("(c) 2011-2017 George Karakostas <gkarak@9-dev.com>");
	output()->p("Version 1.20.1");
}

void BackupApp::DefineParameters() {
	params_["dry-run"] = ParameterRecord(
			"dry-run",
			'n',
			"Simulate backup.",
			ParameterRecord::TypeBool,
			false);
	params_short_['n'] = "dry-run";

	params_["name"] = ParameterRecord(
			"name",
			'\0',
			"Backup job name.",
			ParameterRecord::TypeStr,
			false);

	params_["type"] = ParameterRecord(
			"type",
			'\0',
			"Specify the backup method (tar, rsync).",
			ParameterRecord::TypeStr,
			true,
			"tar");

	params_["incremental"] = ParameterRecord(
			"incremental",
			'i',
			"If on then increment period is *month* (every month one full backup).",
			ParameterRecord::TypeBool,
			false,
			"1");
	params_short_['i'] = "incremental";

	params_["ignore-existing"] = ParameterRecord(
			"ignore-existing",
			'\0',
			"Ignores existing files on destination (rsync only).",
			ParameterRecord::TypeBool,
			false);

	params_["recipient"] = ParameterRecord(
			"recipient",
			'\0',
			"Where to send mail to notify. If empty no email will be sent.",
			ParameterRecord::TypeStr,
			false);

	params_["mysqldump"] = ParameterRecord(
			"mysqldump",
			'd',
			"Run a mysqldump before the backup.",
			ParameterRecord::TypeBool,
			false);
	params_short_['d'] = "mysqldump";

	params_["postgresdump"] = ParameterRecord(
			"postgresdump",
			'p',
			"Run a postgresdump before the backup.",
			ParameterRecord::TypeBool,
			false);
	params_short_['p'] = "postgresdump";

	params_["logfile"] = ParameterRecord(
			"logfile",
			'\0',
			"Where to keep a log file.",
			ParameterRecord::TypeStr,
			false);

	params_["directories"] = ParameterRecord(
			"directories",
			'\0',
			"Which directories to include.",
			ParameterRecord::TypeStr,
			true);

	params_["excludes"] = ParameterRecord(
			"excludes",
			'\0',
			"Which files or directories to exclude.",
			ParameterRecord::TypeStr,
			false);

	params_["destination"] = ParameterRecord(
			"destination",
			'\0',
			"Where to place backup files.",
			ParameterRecord::TypeStr,
			true);

	params_["destination-type"] = ParameterRecord(
			"forward",
			'\0',
			"Specify type of backup destination (local, ssh, usb).",
			ParameterRecord::TypeStr,
			false,
			"local");

	params_["destination-device"] = ParameterRecord(
			"destination-device",
			'\0',
			"If usb, it is the device to be mounted (eg /dev/sdd1).",
			ParameterRecord::TypeStr,
			false);

	params_["destination-encrypted-key"] = ParameterRecord(
			"destination-encrypted-key",
			'\0',
			"If encrypted usb, set to pathfilename of keyfile.",
			ParameterRecord::TypeStr,
			false);

	params_["destination-encrypted-map"] = ParameterRecord(
			"destination-encrypted-map",
			'\0',
			"If encrypted usb, set to mappoint.",
			ParameterRecord::TypeStr,
			false);

	params_["destination-backup-dir"] = ParameterRecord(
			"destination-backup-dir",
			'\0',
			"Where to keep incremental backups (rsync only).",
			ParameterRecord::TypeStr,
			false);

	params_["destination-database"] = ParameterRecord(
			"destination-database",
			'\0',
			"Specify where to output database dumps (if enabled).",
			ParameterRecord::TypeStr,
			false);

	params_["destination-quota"] = ParameterRecord(
			"destination-quota",
			'\0',
			"Up to which size to allow old backups (in GB).",
			ParameterRecord::TypeInt,
			false);

	Application::DefineParameters();
}

/**
 * Validate backup-specific parameters
 */
bool BackupApp::ValidateParameters() {
	ParameterRecord* p = GetParam("type");
	if ((p->value() != "tar") &&
			(p->value() != "rsync")) {
		// following will also cause Application::ValidateParameters() to fail
		p->set_value("");
		output()->p("Invalid parameter value [type] (tar, rsync)", "fail");
	}
	if ((p->value() == "rsync") && GetParam("incremental")->value_bool())
		GetParam("destination-backup-dir")->set_required();
	p = GetParam("destination-type");
	if ((p->value() != "local") &&
			(p->value() != "ssh") &&
			(p->value() != "usb")) {
		p->set_value("");
		output()->p("Invalid parameter value [destination-type] (local, ssh, usb)", "fail");
	}
	if (p->value() == "usb") {
		GetParam("destination-device")->set_required();
	}
	if (CheckParameter("destination-encrypted-key") ||
			CheckParameter("destination-encrypted-map")) {
		GetParam("destination-encrypted-key")->set_required();
		GetParam("destination-encrypted-map")->set_required();
	}
	return Application::ValidateParameters();
}

/**
 * Perform a MySQL Dump
 */
int BackupApp::MySqlDump() {
	if (!mysqldump_) return 0;
	string cmd = "mysqldump --defaults-extra-file=~/.my.cnf --all-databases";
	if (!dry_run_) cmd += " > "	+ destination_database_;
	cmd += " 2>&1";
	if (verbose_) output()->p(cmd);
	string stdout;
	int r = Exec(cmd, stdout);
	output()->p(stdout, "", false);
	if (r) output()->p("Database backup (mysqldump) failure", "fail");
	else {
		cmd = "cat " + destination_database_ + " | wc -l";
		Exec(cmd, stdout);
		output()->Replace(stdout, "\n", string());
		if (stdout == "0") output()->p("Database backup (MySQLdump) success", "ok");
		else output()->p("Database backup (MySQLdump) success, " +
				stdout + " lines", "ok");
	}
	return r;
}

/**
 * Perform a PostGreSQL Dump-o
 */
int BackupApp::PostGresDump() {
	if (!postgresdump_) return 0;
	string cmd = "pg_dumpall";
	if (!dry_run_) cmd += " > "	+ destination_database_;
	cmd += " 2>&1";
	if (verbose_) output()->p(cmd);
	string stdout;
	int r = Exec(cmd, stdout);
	output()->p(stdout, "", false);
	if (r) output()->p("Database backup (PostGreSQL) failure", "fail");
	else {
		cmd = "cat " + destination_database_ + " | wc -l";
		Exec(cmd, stdout);
		output()->Replace(stdout, "\n", string());
		if (stdout == "0") output()->p("Database backup (PostGreSQL) success", "ok");
		else output()->p("Database backup (PostGreSQL) success, " +
				stdout + " lines", "ok");
	}
	return r;
}

/**
 * Check that snar exists in current month otherwise delete
 *
 * @see http://stackoverflow.com/questions/13697941/how-to-get-file-creation-time-in-linux
 * @see http://www.cplusplus.com/reference/cstdio/remove/
 * @see http://stackoverflow.com/questions/16344748/why-gmtime-and-localtime-give-me-the-same-result
 */
bool BackupApp::SnarFileStatus(string snarfile) {
	struct stat attrib;
	if (!FileExists(snarfile, attrib)) return false;
	// mtime is content modification time (when content changed)
	// ctime is inode modification time (when perms etc have changed)
	struct tm* mtime = gmtime(&(attrib.st_mtime));
	// get a copy of mtime to a string because subsequent calls to time
	// functions (gmtime, localtime) use the same memory pointer
	// so result would be the same - @see (3)
	// if daily then check snar in year-month
	string date_format = "%Y-%m";
	// if weekly then check snar in year
	string str_mtime = output()->Date(date_format, mtime);
	string str_now = output()->Date(date_format);
	// check if snar file is current
	if (str_mtime == str_now) return true;
	int rr = remove(snarfile.c_str());
	//should return zero
	if (rr) output()->p("Could not delete outdated snar file [" +
			snarfile + "]", "wrng");
	return false;
}

/**
 * Perform backup using tar
 */
int BackupApp::BackupTar() {
	// Check if file exists
	// If partial then it ignores it
	// Tar only, rsync performed properly
	// at this stage, we do not care about if incremental and .snar extension
	string filename =	name_ + "." + output()->Date();
	if (FileExists(destination_ + "/" + filename + ".snar.bak.tar.gz") ||
			FileExists(destination_ + "/" + filename + ".bak.tar.gz"))
	{
		output()->p("Backup already performed locally", "note");
		output()->set_recipient("");
		return 0;
	}
	output()->p("Initiating backup...", "", true, false);
	string stdout;
	string snarfile = destination_ + "/" + name_ + ".snar";
	// if incremental, and snar not exists (new or outdated)
	filename += (incremental_ && !SnarFileStatus(snarfile)? ".snar": "");
	string pathfilename = destination_ + "/" + filename + ".partial-" +
			output()->Date("%H-%M-%S") + ".bak.tar.gz";
	string final_pathfilename = destination_ + "/" + filename + ".bak.tar.gz";
	output()->Replace(directories_, ",", " ");
	string cmd = "tar cp" +
			(verbose_? string("v"): string()) +
			"zf " + pathfilename +
			(incremental_? " -g " + snarfile: "") + " " +
			excludes_ + " " +
			directories_ + " 2>&1";
	if (verbose_) output()->p(cmd);
	int r = 0;
	if (!dry_run_) r = Exec(cmd, stdout);
	// Not so sure if we need stdout: it is usually "removing trailing /"
	// http://stackoverflow.com/questions/236129/how-to-split-a-string-in-c
	output()->p(stdout, "", false);
	// tar returns 0: ok, 1: differences (for --diff only) and 2: fatal error
	if (!r) {
		output()->p("Files added to archive", "ok");
		if (rename(pathfilename.c_str(), final_pathfilename.c_str()))
			output()->p("Could not rename archive to remove .partial extention", "wrng");
		// should return 0:
	}
	else output()->p("Files have not been successfully archived", "fail");
	return r;
}

/**
 * Perform a backup using rsync
 *
 * 3 modes:
 * normal: rsync -s (-v) (-e ssh) a/ b
 * incremental: rsync -abs (-v) (-e ssh?) --delete --backup-dir=? a/ b
 * ignore-existing: rsync -s (-v) (-e ssh) --ignore-existing a/ b
 *
 * excludes!
 *
 * @see http://wpkg.org/Rsync_exit_codes
 */
int BackupApp::BackupRsync() {
	output()->p("Initiating backup...", "", true, false);
	string cmd = "rsync";
	if (incremental_) cmd +=
			" -abs --no-links " +
			(verbose_? string("-v "): string()) +
			(dry_run_? string("-n "): string()) +
			"--delete --backup-dir=" +
			destination_backup_dir_ + "/" +
			output()->Date();
	else {
		cmd += " -as --no-links " +
				(verbose_? string("-v "): string()) +
				(dry_run_? string("-n "): string());
		if (!ignore_existing_) cmd += "--delete";
		else cmd += "--ignore-existing";
	}
	if (destination_type_ == "ssh")
		cmd += " -e ssh";
	cmd += 	" " + excludes_ + " " + directories_ + "/ " +
			destination_ + " 2>&1";
	if (verbose_) output()->p(cmd);

	string stdout;
	int r = Exec(cmd, stdout);
	output()->p(stdout, "", false);
	switch (r) {
		case  0: output()->p("Backup rsync completed", "ok"); break;
		case  1: output()->p("Syntax or usage error", "wrng"); break;
		case  2: output()->p("Protocol incompatibility", "wrng"); break;
		case  3: output()->p("Errors selecting input/output files, dirs", "wrng"); break;
		case  4: output()->p("Requested  action not supported", "wrng"); break;
		case  5: output()->p("Error starting client-server protocol", "wrng"); break;
		case  6: output()->p("Daemon unable to append to log-file", "wrng"); break;
		case 10: output()->p("Error in socket I/O", "wrng"); break;
		case 11: output()->p("Error in file I/O", "wrng"); break;
		case 12: output()->p("Error in rsync protocol data stream", "wrng"); break;
		case 13: output()->p("Errors with program diagnostics", "wrng"); break;
		case 14: output()->p("Error in IPC code", "wrng"); break;
		case 20: output()->p("Received SIGUSR1 or SIGINT", "wrng"); break;
		case 21: output()->p("Some error returned by waitpid()", "wrng"); break;
		case 22: output()->p("Error allocating core memory buffers", "wrng"); break;
		case 23: output()->p("Partial transfer due to error", "wrng"); break;
		case 24: output()->p("Partial transfer due to vanished source file", "wrng"); break;
		case 25: output()->p("The --max-delete limit stopped deletions", "wrng"); break;
		case 30: output()->p("Timeout in data send/receive", "wrng"); break;
		case 35: output()->p("Timeout waiting for daemon connection", "wrng"); break;
		default: output()->p("Unspecified error code", "wrng"); break;
	}
	return r;
}

/**
 * Perform a cryptsetup command
 */
int BackupApp::Crypt(string cmd) {
	string stdout;
	int r = Exec("cryptsetup " + cmd + " 2>&1", stdout);
	switch (r) {
		case  0: output()->p("Encrypted backup device confirmed", "ok"); break;
		case  1: output()->p("Unable to confirm encrypted backup device: wrong parameters", "wrng"); return r + 10;
		case  2: output()->p("Unable to confirm encrypted backup device: bad key", "wrng"); return r + 10;
		case  3: output()->p("Unable to confirm encrypted backup device: cryptsetup out of memory", "wrng"); return r + 10;
		case  4: output()->p("Unable to confirm encrypted backup device: wrong device specified", "wrng"); return r + 10;
		case  5: output()->p("Encrypted backup device already open", "ok"); break;
		default: output()->p("Unable to confirm encrypted backup device", "wrng"); return r + 10;
	}
	return 0;
}

/**
 * Mount filesystem.
 *
 * @see http://pubs.opengroup.org/onlinepubs/7908799/xsh/sysstat.h.html
 * @see http://linux.die.net/man/3/mkdir
 * @see http://www.dsm.fordham.edu/cgi-bin/man-cgi.pl?topic=cryptsetup
 * @see http://linux.die.net/man/8/mount
 */
int BackupApp::Mount() {
	if (destination_type_ != "usb") return 0;
	// check mounted
	// destination is now the mountpoint
	string stdout;
	Exec("mount | grep " + destination_, stdout);
	if (!stdout.empty()) {
		output()->p("Backup device is mounted on usb", "ok");
		return 0; // mounted
	}
	// check root
	Exec("whoami", stdout);
	// is expected: "root\n", not "root"
	if (stdout.find("root") == string::npos)
		output()->p("Mounting usb device for forwarding may not be possible for non-root", "wrng");
	// check device exists
	// hold variable as it may temporarily change with crypt
	string dev = destination_device_;
	if (!FileExists(dev)) {
		output()->p("Backup device not found", "note");
		return 1;
	}
	// check mountpoint dir
	// if ls without stat we would get an error msg
	if (FileExists(destination_)) {
		// check empty
		Exec("ls " + destination_, stdout);
		if (!stdout.empty()) {
			output()->p("Backup device cannot be mounted: mount point not empty", "wrng");
			return 2;
		}
	}
	// else mkdir
	else if (mkdir(destination_.c_str(), S_IRWXU | S_IRWXG | S_IRWXO)) {
		output()->p("Backup device cannot be mounted: cannot create mount point", "wrng");
		return 3;
	}
	// luksopen
	int r;
	if (!destination_encrypted_key_.empty()) {
		r = Crypt(
				"luksOpen -d " +
				destination_encrypted_key_ + " " +
				dev + " " +
				destination_encrypted_map_);
		if (r) return r;
		dev = "/dev/mapper/" + destination_encrypted_map_;
	}
	// mount
	r = Exec("mount " + dev + " " + destination_ + " 2>&1", stdout);
	switch (r) {
		case  0: output()->p("Backup device mounted", "ok"); break;
		case  1: output()->p("Backup device not mounted: incorrect permissions", "wrng"); return r + 20;
		case  2: output()->p("Backup device not mounted: system error", "wrng"); return r + 20;
		case  4: output()->p("Backup device not mounted: internal problem", "wrng"); return r + 20;
		case  8: output()->p("Backup device not mounted: user interrupt", "wrng"); return r + 20;
		case 16: output()->p("Backup device not mounted: problem locking", "wrng"); return r + 20;
		case 32: output()->p("Backup device not mounted: mount failure", "wrng"); return r + 20;
		case 64: output()->p("Backup device not mounted completely", "wrng"); break;
		default: output()->p("Backup device not mounted: multiple errors detected", "wrng"); return r + 20;
	}
	return r;
}

/**
 * Unmount filesystem
// umount
// luksclose
 */
void BackupApp::UnMount() {
	if (destination_type_ != "usb") return;
	// check mounted
	string stdout;
	Exec("mount | grep " + destination_, stdout);
	if (stdout.empty()) {
		output()->p("Backup device can be safely removed from usb", "ok");
		return;
	}
	if (Exec("umount " + destination_ + " 2>&1", stdout)) {
		output()->p("Backup device is busy and cannot be safely removed from usb", "note");
		return;
	}
	if (!destination_encrypted_key_.empty() &&
			Crypt("luksClose " + destination_encrypted_map_)) {
		return;
	}
	output()->p("Backup device can be safely removed from usb", "ok");
}

void BackupApp::TimeStat(time_t t_start) {
	time_t t_end = time(0), t_elapsed = t_end - t_start;
	int m = (t_elapsed / 60);
	int h = (m / 60);
	stringstream s, sm, sh;
	sm << m << " min ";
	sh << h << " h ";
	if (h || m) {
		s << "Started at " << output()->DateT("%H:%M", t_start) <<
			", finished at " + output()->DateT("%H:%M", t_end);
	}
	else {
		s << "Finished at " << output()->DateT("%H:%M", t_end);
	}
	s << 	" (" <<
			(h? sh.str(): "") <<
			(m? sm.str(): "") <<
			t_elapsed << " sec)";
	output()->p(s.str());
}

/**
 * Main function
 *
 * GENERAL
 * propage usb: push
 * propagate ssh: pull? not yet, push for now
 * pull function: reverse backup rsync!!! may be working, test
 * so push (check if exists, then rsync) for both, BEFORE frequency
 * make sure (if propagate ssh) that cron is not too frequent!
 *
 * USE CASES see backup-strategy.odt
 *
 * TODO index
 * Log files double endl: ok
 * Backup size: w*
 * MyqlDump ok
 * PostGresDump ok
 * app as lib
 * security program (logwatch: run as /usr/sbin/logwatch)
 * html mail*
 * compress rsync backup_dir/date! (new tar with time - append to tar.gz not possible).
 * purge
 * i18n (separate class child of output)
 * Restore
 * Debian package (see application.h)
 * Host on github (not launchpad because stackoverflow careers, drupal.org uses git too
 *
 * OPLOCKS
 *Tar fail options
Kernel oplocks = false: solves issue; creates problem if samba share is mixed shared with other clients (FTP, SFTP, NFS, OpenVPN etc).
Tar ignore ~! files: reduces possibility of issue and allows other clients, but issue is still there.
Tar schedule at other time: reduces possibility drastically, but still there and requires server to be ON all time. [***eupolis]
Rsync backup and then tar (2 jobs; a snapshot in time is essentially a MS shadow copy)

https://bugzilla.redhat.com/show_bug.cgi?id=839294
http://oreilly.com/openbook/samba/book/ch05_05.html

HTML mail
http://stackoverflow.com/questions/2591755/how-send-html-mail-using-linux-command-line
echo "<h1>TEST</h1><b>HTML Message goes here</b>" | mail -a "Content-type: text/html" -s "This is the subject" gkarak@forecasting.gr

Backup sizes:
drobopro: 10.86T total, 8.36T used, 1.2T for purge (3-day avg 13G = 90d) - with compression: 12G = 1.5T 120d
reports: device free, source device free, tar filesize, rsync size NO, incremental size
device: df -h | grep dev | awk '{print $3}', file: stat, rsync: du -ch --max-depth=1 dir OR du -ch dir | tail -n 1 | awk '{print $1}'
wait for security module, purge.

For external usb, instead of cron, you can use udev to notify for plug
http://ninetynine.be/blog/2009/03/ubuntu-backup-to-usb-drive-on-mount/
http://ubuntuforums.org/showthread.php?t=168221

Purge 2015:
- if tar:
-- delete all non-snar before 31 days: find ! -name '*snar*' (doesn't matter if incremental!, this will just not be good enough for full backups)
-- if still not enough, delete all but one from past 365 days (not good for eupolis)
-- if still not enough, start deleting one by one from past 31 days
-- if still not enough, msg
- if rsync incremental:
-- start deleting one by one

 */
int BackupApp::Main() {
	// initialization
	name_ = GetParamValue("name");
	if (name_.empty()) name_ = "sysadmin-backup";
	type_ = GetParamValue("type");
	incremental_ = GetParam("incremental")->value_bool();
	ignore_existing_ = GetParam("ignore-existing")->value_bool();
	mysqldump_ = GetParam("mysqldump")->value_bool();
	postgresdump_ = GetParam("postgresdump")->value_bool();
	output()->set_filename(GetParamValue("logfile"));
	output()->set_recipient(GetParamValue("recipient"));
	directories_ = GetParamValue("directories");
	excludes_ = GetParamValue("excludes");
	if (!excludes_.empty()) {
		excludes_ = output()->Replace(excludes_, ",", " --exclude=");
		excludes_ = "--exclude=" + excludes_;
	}
	destination_ = GetParamValue("destination");
	destination_type_ = GetParamValue("destination-type");
	destination_device_ = GetParamValue("destination-device");
	destination_encrypted_key_ = GetParamValue("destination-encrypted-key");
	destination_encrypted_map_ = GetParamValue("destination-encrypted-map");
	destination_backup_dir_ = GetParamValue("destination-backup-dir");
	destination_database_ = GetParamValue("destination-database");
	if (destination_database_.empty()) destination_database_ = destination_;
	destination_database_ +=  "/" + name_ + ".sql";
	destination_quota_ = GetParam("destination-quota")->value_int();
	verbose_ = GetParam("verbose")->value_bool();
	dry_run_ = GetParam("dry-run")->value_bool();
	time_t t_start = time(0);
	int r;
	output()->set_subject("Backup report");

	// Do not let double process in case cron.hourly and previous backup still executes
	r = 0;
	if (!Lock("sysadmin-backup")) {
		output()->p("Other backup still in progress", "fail");
		output()->set_filename("");
		output()->set_recipient("");
		return 0;
	}
	// Now mount the filesystem (if usb)
	r = Mount();
	if (r) {
		output()->set_recipient("");
		// return 0 or cron will complain.
		return 0;
	}
	MySqlDump();
	PostGresDump();
	if (type_ == "tar")	r = BackupTar();
	else r = BackupRsync();
	TimeStat(t_start);
	if (r) {
		output()->p("Backup not completed successfully", "fail");
		output()->set_subject("*** Backup report for " + name_ +": FAILED");
	}
	else {
		output()->p("Backup completed successfully", "ok");
		output()->set_subject("Backup report for " + name_ +": OK");
	}
	UnMount();
	Unlock();
	return r;
}
