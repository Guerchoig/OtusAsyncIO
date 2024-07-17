## Outline
A command processor, receive commands from external network connections and send them 
in an asynchroneous & multithread way to files and to console

## Server run
   bulk_server <port> <block_size> <ip_address>
or bulk_server <port> <block_size>
or bulk_server <port>
or bulk_server 
CTRL+C - stop operation

## Client run
   client <commands start_number>
or client
disconnect in automatic mode - automatically
disconnect in manual mode - CTRL+D

## Archtecture and operation
Server implements receiving text commands over network and outputing them whith help of 'libasync.so' library
The library maintains one 'dynamic' commands input queue per connection, which store commands enclosed into figure brackets,
and one input queue for 'static' commands, common for all the connections.
The commands from both input queues come into output block queue whith blocks formed  'on the fly'.
A block from the static queue forms when number of commands atteign block_size or when the server is being shutdowned
A block from one of dynamic queues is formed and output when a 'close bracket' arrives.

The server disconnect client when accepts from it a special DISCONNECT symbol.
The server accurately shuts down on CTRL+C, producing output of commands, being still in buffers.




