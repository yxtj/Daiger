# Work flow of some network actions

## Default reply handler

Reply message type:
```
int type; // the source type of the reply
```
The handler puts it into the reply-handler with the type.

By default, nothing will happen. 
In order to take some actions, one should be register the actions for this type in the reply-handler.

## System start

1. Master : when boot,
	- send message "COnline" to all workers.
2. Worker : when receive "COnline",
	- store the sender's net-id as master's net-id.
	- send message "CRegister" to master AS REPLY
	- waiting for reply with a timeout
3. Master : when receive "CRegister" from a worker,
	- assign a worker-id for the network-id.
	- reply.
4. Master : when all "CRegister" are received,
	- finish.


## Initialization

1. Master :
	- send message "CWorkers" to all workers, containing the assigned (net-id, worker-id) mapping of all workers.
2. Worker : when receive "CWorkers" from master,
	- store it.
	- do local initialization
	- send a reply to master.
3. Master : when all workers replied,
	- finish

## Run a procedure

### Start a procedure

1. Master : 
	- send message "CClear" to all workers.
2. Worker : when receive "CClear" from master,
	- flush out all messages.
	- wait until all incomming messages are processed or ignored.
	- reply to master.
3. Master : when all workers replied,
	- send message "CProcedure" to workers with the id of the procedure to run.
4. Worker : when receive "CProcedure" from master,
	- start a new thread running specific procedure
	- reply to master

### Finish a procedure

- Type 1: worker initiated (e.g. loading, outputing)

	1. Worker : when the local part of a procedure finishes,
		- clear the resources for this procedure.
		- send message "CFinish" to master with procedure-id.
	2. Master : when receive "CFinish" from a worker,
		- check the procedure id.
		- reply it.
	3. Master : when receive "CFinish" from all workers,
		- send message "CFinish" to workers with procedure-id.
	4. Worker : when receive "CFinish" from master,
		- flush out generated but unsent messages.
		- abandon unprocessed procedure-related messages.
		- clear the resources for this procedure (if something left).
		- reply it.
	5. Master : when receive replies from all workers,
		- clear the resources for this procedure.

- Type 2: master initiated (e.g. processing)
	
	1. Master : when decide to finish,
		- send message "CFinish" to workers with procedure-id.
	2. Worker : when receive "CFinish" from master,
		- flush out generated but unsent messages.
		- abandon unprocessed procedure-related messages.
		- clear the resources for this procedure (if something left).
		- reply it.
	3. Master : when receive replies from all workers,
		- clear the resources for this procedure.

	(This is actually the step 3, 4, 5 of Type 1.)

## Progress report & Termination check

