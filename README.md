hermes_c
========

A HTTP-like messaging library that allows to build no-frills servers.
See tests/copy_server.c and tests/copy_client.c for details.

READ THIS FILE IN RAW MODE. THE HTML FORMATTING MESSES UP THE LAYOUT.

Basic Message Format
---------------------

VERB [un-named headers]
[named headers]
.

[un-named headers] are strings separated by spaces
[named headers] are key value pairs of type 
<key>:<value>
If the message contains a payload then there is a special
content-length: <num>
Message header ends with a ".\n" (a line with just a period)

Example: (copy_client)

COPY
Filename:blah
Offset:0
Content-Length:12
.

The above command has:
VERB =              COPY
Un-named headers=   None
Named-headers=      Filename:blah
                    Offset:0
                    Content-Length:12 (indicates there is a payload)
(The ".\n" ends the message header. The payload follows the dot)


Default Message Handlers
-------------------------

Hermes has a set of default message handlers. These can be overridden to something you want.
Here's an example of talking with the Copy server

[bash]$ telnet localhost 61182
Trying 127.0.0.1...
Connected to localhost.
Escape character is '^]'.
INFO .
INFO
Localtime:Sun Jun  3 09:40:21 2012

.
PING .
PONG
.
BYE .
Connection closed by foreign host.


INFO .
A diagnostics command that gets the status of the server

PING .
Server returns a PONG (a heartbeat message)

BYE .
Closes the conection

Things to note
---------------

* The parsing is very strict. It will shutdown the connection to prevent memory leaks.
* Rationale: If broken connections are annoying then you will fix the messages. :-)
* Server side code uses thread pools to manage connections


