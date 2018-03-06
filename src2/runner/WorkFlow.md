# Work flow of some actions

## Default reply handler

Reply message type:
```
int type; // the source type of the reply
```
The handler puts it into the reply-handler with the type.

By default, nothing will happen. 
In order to take some actions, one should be register the actions for this type in the reply-handler.

## Register worker

1. Master : when system starts,
	- send message "CRegister" to workers with its own network-id.
2. Worker : when receive "CRegister" from master, 
	- remember the network-id of master.
	- send message "CRegister" to master, whose content is ignored.
3. Master : when receive "CRegister" from a woker,
	- assign a worker-id for the network-id.
4. Master : when all workers are registered
	- send message "CWorkers" to all workers, containing (nid, wid) mapping for all workers.
5. Worker : when receive "CWorkers",
	- store them.
	- send a reply to master.
6. Master : when all workers replied
	- finish the registering.
	- if timeout, terminates the system.

## Progress report & Termination check

