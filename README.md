# CPM (Cluster Processes Monitor)
**CPM**, or Cluster Processes Monitor, is a tool to measure the performance of a job processes inside a computing cluster.

This tool composed by three programs:

* **MonitoringProcesses**. The client. This program runs locally in the user computer.
* **MonitoringMaster**. The daemon running in the cluster front/master node.
* **MonitoringAgent**. The daemon running in the cluster computing/worker nodes.

# What's CPM about? #
Typically, in a computing cluster, the task of monitorize a parallel job and get statictics, such as CPU consumption, used memory or other kind of data, is a tedious job that implies visualize large logs or modify the application source code. There are some solutions that monitorize all the cluster, such as [Nagios][1] or [Zabbix][2], but they monitorize the nodes total resources, and not per process of a launched job. **CPM** allows the user to monitorize data from individual processes in real time and get plots and statictics from them in a very easy way. It is composed by three programs that communicate among each other and work togheter.

In order to explain how **CPM** works, is needed to talk about how a computing cluster typically works. In almost all the supercomputers or clusters, the user connects throught SSH to what is called the **master or front node**. In this master node the user can compile his/her program that uses MPI, OpenMP, Hadoop, or others. After that, the user launchs the job by using a queue or resources manager, and after to wait for available resources, the job runs in the **worker or computing nodes**, this is, the rest of nodes in the cluster that make the real computation.

The general process of how **CPM** works can be seen at the next figure, and this process is explained next.![Figure 1](/doc/Images/Diagrama2.png) 

Typically, only the master node has direct acces to the outside world. In this master node is where the **MonitoringMaster** is going to run. This daemon acts as a bridge between the computing nodes and the user PC. It is going to open the port 8000 and waits for messages from **MonitoringProcesses**, this is, the client. This client is the visual program that runs in the user PC. The **MonitoringProcesses** program opens the port 10000, where it receives packages from the master node with data that contains the information from the computing nodes. In the computing nodes is where the **MonitorinsAgent** runs. This daemon opens the port 20000.

When the **MonitoringProcesses** client wants to start taking measures from a job, it sends a package to the **MonitoringMaster**. This daemon creates an image from itself by using the *fork()* function and listens in the port 8000 + N, where N is the number of **MonitoringMaster** processes that are already running in the master node. This forked process sends an ACK to the client. In this way, the client knows that the program is running in the master node and at wich port. After that, the client sends to this new process a package pear each one of the agents it wants to have data, and a package to indicate that also wants data from this master node. Then, the **MonitoringMaster** sends a package to each one of the computing nodes, where the **MonitoringAgent** is running. This daemon forks and launchs a **MonitoringAgent** process in each one of the computing nodes. This forked processes are the ones that actually get the information from given processes and send it back to the **MonitoringMaster**, who is going to re-send it to the client.

## Resume ##
**MonitoringProcesses**
It is implemented in C++ using the Qt libraries. It provides a user-friendly interface to visualize the data from the cluster processes. Listen at port 10000, where it receives data from **MonitoringAgent** across the **MonitoringMaster**.

**MonitoringMaster**
It is a program that runs in the cluster computing master node. It works as a bridge between the client program, and the agents running in each one of the computing nodes of the cluster. It listens at port 8000 + N.

**MonitoringAgent**
It runs in the computing nodes and is the responsible of collecting data of the processes and send them to the client by means of the **MonitoringMaster** process.

# Structure #
In this GitHub repository you can find the following directories:

* doc - Documentation.
* script - Scripts that are going to be used to start or stop the daemons.
* src - **CPM** source code. It includes the source of the three programs.
* build - Directory where the final compiled programs are stored.

# Getting started #

## Requirements ##
Common requirements for the three programs are a Linux 64 bit distribution. Individual requirements to build **CPM** are the next ones.

### MonitoringClient ###

* Qt libraries.
* C++11.

### MonitoringMaster ###

* C++11.

### MonitoringAgent ###

* *procps* development libraries. In Debian, *libprocps4-dev*, in CentOs, *procps-devel.x86_64*.
* C++11.


## Building
The default way to build **CPM** is:

	git clone https://github.com/jmabuin/CPM.git
	cd CPM
	make
		
This will create the *build* folder, which will contain three files:

* **MonitoringAgent**
* **MonitoringMaster**
* **MonitoringProcesses**

## Running CPM ##
**CPM** requires a working cluster. In each one of the cluster computing node the **MonitoringAgent** has to be running. For that, in each one of these nodes, the user has to execute `./MonitoringAgent`. In the master node, the **MonitoringAgent** and the **MonitoringMaster** have to be running. In the case of the Agent, the command is the same. For the Master the command is `./MonitoringMaster`. And finally, in the user's PC, the command to execute is `./MonitoringProcesses`. This last command will open the graphic interface made with Qt.

## Configuring CPM ##

There are some parameters that the user needs to configure in order to execute **CPM** in a correct way. Some of these parameters need to be configured in the source code. It is the case of the network interfaces to use in the Master Daemon and the Agent Daemon.

Inside the src directory we can see the MonitoringAgent and MonitoringMaster folders. Inside each one of them, in the file *Network.h*, the user needs to modify the value of the defines NETWORK\_INTERFACE (in the case of MonitoringAgent), and NETWORK\_INTERFACE and NETWORK\_INTERFACE\_INTERNAL (in the case of MonitoringMaster). In the master node is typicall to have two network interfaces, one to comunicate with the outside world(NETWORK\_INTERFACE), and another one to communicate with the computing nodes(NETWORK\_INTERFACE\_INTERNAL).

In case of neccesity, the user can also change the default ports at which the tool operates, also in the Network.h file inside each one of the folders.

##Configuring the MonitoringProcesses program##
At the **MonitoringProcesses** main window, the user can configure basic parameters by doing click in *File* and *Configure*. This window can be seen here. ![Figure 2](/doc/Images/Configuring.png)
The basic parameters that the user needs to configure are:

* **Cluster node list - Usual Nodes** - Here the user has to put the Master Node name or Ip address. Here is where the **MonitoringMaster** and one of the **MonitoringAgent** are going to run.
* **Cluster node list - Nodes Behind Master Node** - Here the user has to put the Computing Nodes names or Ip addresses, one per line. In these nodes the **MonitoringAgent** is going to be running.
* **Process Settings - Process owner** - The owner of the processes running in the cluster from which the user wants to have data. In the case of the image, the owner is the user *hadoop*.
* **Process Settings - Process name** - The name, or a string that contains part of the name, of the processes running in the cluster from which the user wants to have data. In the case of the image, the name contains the string *container*, as it is a hadoop process.
* **Process Settings - Process starts with** - The starting string of the processes running in the cluster from which the user wants to have data. In the case of the image, the processes starts with the string */usr/lib/jvm/java*.
* **Network** - Here the user has to select the network interface to communicate with the cluster Master Node.

The rest of the options are not currently working, because they are planned to work in future versions with future features.

##Frequently asked questions (FAQs)

1. [I can not build the tool because I can not find the procps library.](#building1)

####<a name="building1"></a>1. I can not build the tool because I can not find the procps library.
Depending on the Linux distribution the name of the library can change. For example, in Debian the library is linked with the *-lprocps* argument, and in CentOs with *-lproc* argument. Check in your library folder for the libproc library and link it properly in your Makefile.

[1]: https://www.nagios.org/
[2]: http://www.zabbix.com/
