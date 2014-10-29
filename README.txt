General Information
===================
This package contains source code for the application that implements the
Ricart-Agrawala's algorithm [2] for distributed mutual exclusion with the
optimization suggested by Roucairol-Carvalho [1]. Also the client sends
data to the designated servers (indicated during the execution of the program).


The source code is POSIX compliant and uses the standard C99 data types.

The application is aptly named "rfs", short for replicated file system.

Building
========
1. % git clone https://github.com/corehacker/projects.git
2. Navigate to distributed-algorithms/ch-rfs.
   % cd distributed-algorithms/ch-rfs
3. Issue make command after issuing make clean.
   % ./configure
   % make clean
   % make
   After successful execution of the above commands, the executable "ch-rfs"
   will be created in ch-rfs directory.
4. Copy the aplication to the desired directoty.
   % cp ch-rfs <Desired Directory>
   
Execution
=========
1. Requirements:
   a. Current implementation supports execution of algorithm for arbitrary number 
      of nodes. But there is a cap put up so that total number os nodes does not
	  exceed 50. Although, changing a few macros will enable the application to 
	  support more nodes.
   b. All the nodes should reachable by each other and all firewall for incoming
      connections for the ports that will be used (refer below) should be
      disabled.
   c. The application accepts only DNS host names.

2. Application Usage:
   Usage: 
       ./ch-rfs <Total Nodes> <Node Index> <Act as Server> <Listen Port Range Start> 
		<Leader Host Name> <Unit Time> <Console Logging> [<Port No For Internal Use>]
   
   Parameter Details: 
        <Total Nodes, Range: 2 - 10> - Mandatory
        <Node Index, Range: 0 - 9; 0th index will mapped to leader.> - Mandatory
        <0 - Will act as a client, 1 - Will act as server> - Mandatory
        <Listen Port Range Start (> 15000 Preferred)> - Mandatory
        <Leader Host Name (Host Name of Node 0)> - Mandatory
        <Unit Time, in milliseconds. This will be used internally by algorithm for 
		 various waits conditions> - Mandatory
        <Console Logging, 1 - Enable, 0 - Disable> - Mandatory
        [<Port No For Internal Use (Defaults to 19000)>] - Optional. But required if 
		  bind fails for the internal port


3. Example:
   a. Catch hold of 10 machines named machine1-machine10. (10 nodes)
   b. Navigate to the directory where the "ch-rfs" executable is located.
      Copy the executable to each of the machines. Recompilation may be necessary
	  for each of the machine.
   c. Starting from machine1, execute the following commands, one on
      each node, in sequence. (This sequence is not necessary, but to make the
      executions simpler.). Here machine1 will be used as the leader
      node. Also 100ms will be used for unit time. Also nodes 0, 1 and 2 are acting
	  as the server, which is being specified by the fourth command line argument.
      % ./ch-rfs 10 0 1 37001 machine1 100 0
      % ./ch-rfs 10 1 1 37001 machine2 100 0
      % ./ch-rfs 10 2 1 37001 machine3 100 0
      % ./ch-rfs 10 3 0 37001 machine4 100 0
      % ./ch-rfs 10 4 0 37001 machine5 100 0
      % ./ch-rfs 10 5 0 37001 machine6 100 0
      % ./ch-rfs 10 6 0 37001 machine7 100 0
      % ./ch-rfs 10 7 0 37001 machine8 100 0
      % ./ch-rfs 10 8 0 37001 machine9 100 0
      % ./ch-rfs 10 9 0 37001 machine10 100 0
   d. Wait till all the nodes enter and exit the critical section 40 times. At 
      the end of 40 executions, a summary of all the executions and statistics
      will be printed on the console of the leader (machine1).
   e. In case of any node going down (crash/network failure) please restart the
      application as described in c above.

4. Data Verification
   a. Each server node writes a file as follows:
      dimutex_server_data_<Node Index>_<Server DNS Hostname>.txt
   a. The leader node which is also acting as a server will check for the 
      consistency of all the files written by different servers. This is done
	  by executing the "diff" command using the "system" system call. This is
	  feasable due to the fact that when we execute on the machine1-10 machines, 
	  the NFS mounted home folder of the user is visible to all the servers.
	  
5. Miscellaneous
	a. The program has been check for memory leaks and found to be none in the
	   "happy path" case, i.e., when the algorithm executes and finishes till
	   the end.
   
Known Issues
============
1. During the execution of the algorithm, if any of the nodes goes down due to
   a crash or network error, the error cases are not handles in the other 
   nodes. Meaning, the socket/task clean-up parts of the code will not be 
   executed, which will lead to memory leaks.
   
Copyright
=========
Copyright Sandeep Prakash (c), 2012
Sandeep Prakash - 123sandy@gmail.com

References
==========
[1] O.S.F. Carvalho and G. Roucairol. On Mutual Exclusion in Computer Networks 
(Technical Correspondence). Communications of the ACM, February 1983.

[2] G. Ricart and A. K. Agrawala. An Optimal Algorithm for Mutual Exclusion in
Computer Networks. Communications of the ACM, 24(1):9{17, January 1981.