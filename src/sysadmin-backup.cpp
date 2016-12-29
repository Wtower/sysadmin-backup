/*
 * sysadmin-backup.cpp
 *
 *  Created on: 20 Ιαν 2014
 *      Author: gkarak
 */

#include "backupapp.h"

using namespace std;

int main(int argc, char **argv) {
	BackupApp app;
	return app.Init(argc, argv, true);
}
