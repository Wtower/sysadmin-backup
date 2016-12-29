sysadmin-backup
===============

Console backup app

Features
--------

- Backup methods: tar, rsync
- Backup modes: normal, incremental
- Email notification
- Mysqldump/postgres dump
- Log file
- Exclude file list
- Destination types: local, ssh, usb
- Automatic usb media mount and decryption
- Size quota (soon)
- Configuration files

How to use
----------
::

    $ ./sysadmin-backup
    SYSadmin Backup
    (c) 2011-2017 George Karakostas <gkarak@9-dev.com>
    Version 1.20
    Missing parameter [destination]                                          [fail]
    Missing parameter [directories]                                          [fail]
    Usage: ./sysadmin-backup (options) (configuration file)

    Options:
            --destination: Where to place backup files.
            --destination-backup-dir: Where to keep incremental backups (rsync only).
            --destination-database: Specify where to output database dumps (if enabled).
            --destination-device: If usb, it is the device to be mounted (eg /dev/sdd1).
            --destination-encrypted-key: If encrypted usb, set to pathfilename of keyfile.
            --destination-encrypted-map: If encrypted usb, set to mappoint.
            --destination-quota: Up to which size to allow old backups (in GB).
            --forward: Specify type of backup destination (local, ssh, usb).
            --directories: Which directories to include.
            -n, --dry-run: Simulate backup.
            --excludes: Which files or directories to exclude.
            -h, --help: This help screen.
            --ignore-existing: Ignores existing files on destination (rsync only).
            -i, --incremental: If on then increment period is *month* (every month one full backup).
            --logfile: Where to keep a log file.
            -d, --mysqldump: Run a mysqldump before the backup.
            --name: Backup job name.
            -p, --postgresdump: Run a postgresdump before the backup.
            --recipient: Where to send mail to notify. If empty no email will be sent.
            --type: Specify the backup method (tar, rsync).
            -v, --verbose: Print additional information.

See also the provided configuration examples under ``/etc``.
