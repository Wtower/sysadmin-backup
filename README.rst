sysadmin-backup
===============

Console backup app version 2

GKlib4
------

Initially the ``gklib4`` directory was placed out of the root of the project.

Notes on Eclipse IDE
--------------------

Install with::

    sudo apt-get install eclipse-cdt g++

http://stackoverflow.com/questions/14721514/how-to-install-eclipse-with-c-in-ubuntu-12-10

New project
^^^^^^^^^^^

Use Linux gcc (not cross) toolchain (compiler, linker etc).

If issue with includes:

- http://askubuntu.com/questions/150139/how-to-make-eclipse-cdts-linux-gcc-toolchain-resolve-c-standard-library-heade

Execute shell command
---------------------

``popen``

- http://stackoverflow.com/questions/478898/how-to-execute-a-command-and-get-output-of-command-within-c
- http://www.sw-at.com/blog/2011/03/23/popen-execute-shell-command-from-cc/

String class
------------

Use ``c_str()`` to output to ``printf``

- http://stackoverflow.com/questions/10865957/c-printf-with-stdstring

Arguments
---------

- http://www.cplusplus.com/articles/DEN36Up4/
