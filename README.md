# Synchronized-Process

* Written by Guy Azoulay - 207709734 
             Tal Malchi -  208278556
             
             
In this assigment we asked to build our own stack using MUTEX and multiThreading, we wrote this assigment in the C++ language.

## Main goal of this project
The main goal of this project is deep understanding about the synchronized multiThreading world and communication
between number of clients and one server.  
In addition, in this project we were facing with some issues of how to let few Client preform action in parallel.  
 Each Client ask from the server one of the standard stack's actions (PUSH , POP , TOP) -without interupt of other client.    
Each action is critical section in the code, and this is where the Mutex takes place.  
Each time one Client is asking from the server some action, we use Mutex lock and in that way we ensure that no other client will interupt to his action.

We were implemnted commends such as:
* PUSH - insert to our stack new string
* POP - remove the upper string 
* TOP - introduce to the client the upper string

In addition we were implemented the Malloc, Calloc and free commends which we will explain about later.

## Explanation about the Server class:

In our server class we use most of our code from Ex3 and made the necessary changes in aim to 
adjust our code to this assigment.
First of all, in this class we created our server socket using all the known commends (create socket ,bind , listen and accept)
and when the client ask from the server an action, the server do it job depands on the key words which stack have (PUSH, POP, TOP).
Each one of the commands are surrounded by pthread mutex lock and unlock on the cretical section, in aim to prevent a
case which more than one client trying to reach to the same function.

## Explanation about the Client class:

In our client class we use most of our code from Ex3 and made the necessary changes in aim to 
adjust our code to this assigment.
First of all, in this class we created our client socket using all the known commends (create socket ,connect and send)
and For each action, the client will send one of the following messages (PUSH, POP, TOP) to the server, 
If the commend is PUSH so the client also send the string he wants to add.


## Explanation about the Stack class:

We built node struct in order to implement our Stack Class.  
These struct containf 4 field: 
     
    
* T data - which store the data inside the node
* size_t size - which store the size of our node
* bool is_free - a boolean variable that check if the place in memory we allocate to it was free or not.
* node * next - pointer to the next node in our stack.
* 

Stack class contains main 3 methods:    
In stack class there are the main three function of stack:
* PUSH
* POP
* TOP

And as we asked for, we were implemented 3 function which related to memory allocation:
* Malloc - function gets variable's size and returns a pointer to the new place in memory which 
           it now allocated to, when we are creating new stack we are using this function.
           
* Free - free the memory space that was given by the malloc or calloc functions, using in the "POP" method.

* Calloc - function gets number of arguments and the size of them and allocating a new place in the heap memory,
           when we want to use the "PUSH" commend we are using this function.
           
           
           
           
## The My_malloc function:

In aim to achive the main goal of the malloc function, we write two auxiliary functions:

* look_for_memory function - this function duty is to allocate new place in our memory.
* update_node function - this function update all the stack fields.

In the my_malloc function we used "sbrk" which used to change the space allocated for the calling process,
and again we lock our function using mutex in aim that we will allocate only the necessary place And more than one client can not 
access the allocation of new space in memory. 


## The My_calloc funtion:

In this function we used our my_malloc function, the only change in it is that we send also the numbers
of variables we want to allocate memory.

## The my_free function:

In this function we release the space which we have allocated to the specific variable, 
we use this function when we perform the "POP" operation.


## How to run ?  
Open two terminal. In the first one write `make server` and then `./server`   
In the second terminal write write `make client`  and then `./client 127.0.0.1`           
In order to clean all the files write `make clean`

We attaching you this video to see how the project work, enjoy!



https://user-images.githubusercontent.com/87694635/164465263-a8370551-583b-41cc-88c9-801ad42a8fa5.mp4









             
