# Qme (Queue me)

A simple and fast task queuing system

The basic approach is that a task can hold four states, which are
INIT -> QUEUED -> RUNNING -> FINISHED

by submitting a task, it will enter the INIT state.
The Queue deamon (openrc) immediately picks it up and changes the state to QUEUED.
If the needed resources are available and the said task is next in line
the task will enter the RUNNING state, with the associated execution of the task.
When the execution is completed the task reaches the final FINISHED state.

(c) christoph irrenfried, chi86


# Install

```console
foo@bar:~$ make -B
foo@bar:~$ sudo make install
```

Creates the spool environment

```
/var/spool/Qme
|  
|─── info
|       Qme.log (general Qme logging file)
|       taskid  (id of last task)
|       version (verions file)
|
|─── init
|─── queued
|─── running
|─── finished
```

## Init service
The init script will be copied automatically to:
```
/etc/init.d
|  
|   Qme
```

### Starting of init service

```console
foo@bar:~$ sudo /etc/init.d/Qme start
```

### Adding to runlevel

```console
foo@bar:~$ sudo rc-update add Qme
```

### Checking up on service
```console
foo@bar:~$ /etc/init.d/Qme status
```

## systemd
The systemd script needs to be copied to:
```
/etc/systemd/system
|  
|   Qme.service
```

### Starting the systemd service

```console
foo@bar:~$ systemctl start Qme.service
```

### Starting adding to runlevel
```console
foo@bar:~$ systemctl enable Qme.service
```

### Checking up on service
```console
foo@bar:~$ systemctl status Qme.service
```


# Usage

Usage: Qme [FLAG] [ARGUMENTS] \

|FLAG  |   interpretation                 |
|------|----------------------------------|
|  -h  |   Help (this!)                   |
|  -Y  |   run Queue                      |
|  -X  |   Demonize the Queue             |
|  -a  |   submit add new task to queue   |
|      |   Arguments:                     |
|      |   NAME SCRIPT.sh NPROC           |
|      |   Default:                       |
|      |   RANSrun Allrun.sh 1            |
|  -d  |   delete task from queue         |
|      |   Arguments:                     |
|      |   ID                             |
|  -l  |   list all tasks                 |


## Example

Startring a OpenFoam case

```console
foo@bar:~$ Qme -a motorbike Allrun 6

Adding task to queue
Task id             : 0
Task name           : motorbike
Script              : Allrun
Current working dir : /home/foo/OpenFOAM/motorbike/
nproc               : 6
owner               : chi
env                 : TERM=xterm
env                 : USER=foo
env                 : HOME=/home/foo
env                 : SHELL=/bin/bash
env                 : LOGNAME=foo
env                 : PATH=/usr/bin:/bin:/opt/bin:/usr/local/bin

```

Status of queued tasks (highlighted in fancy colors)

```console
foo@bar:~$ Qme

Qme Version  0.9
------------------

    ID      pid            Name   Status   proc   Dt/min                      Date
----------------------------------------------------------------------------------
    19        0           test2   queued      6        4  Tue Mar  2 13:57:08 2021
    21        0           test4   queued      6        4  Tue Mar  2 13:57:08 2021
    22        0           test5   queued      6        4  Tue Mar  2 13:57:08 2021
    18        0           test1   queued      6        4  Tue Mar  2 13:57:08 2021
    20        0           test3   queued      6        4  Tue Mar  2 13:57:08 2021
     7    20705       motorbike  running      6       17  Tue Mar  2 13:43:39 2021
    13    23574           test1  running      6        0  Tue Mar  2 14:00:52 2021


```

# Changelog

- version 1.0: Basic functions
- version 2.0: Bugfix, extended functionality
