#include <iostream>
#include <cstdlib>
#include <pthread.h>
#include <sys/mman.h>
using namespace std;

// stack class implementation
template <typename T>
class stack
{
  // private:
  struct node
  {
    T data;       // variable which holds the data
    size_t size;  // variable which holds the size of the data
    bool is_free; // variable which holds the status of the node
    node *next;   // variable which holds the address of the next node
  };
  node *head;           // variable which holds the address of the head node
  node *start;          // variable which holds the address of the start node
  node *end1;           // variable which holds the address of the end node
  pthread_mutex_t lock; // variable which holds the lock

public:
  // constructor of the stack class
  stack()
  {
    node *check = (node *)my_malloc(sizeof(node)); // allocate memory for the head node using my_malloc function
    if (check == NULL)                             // if the allocation fails
    {
      cout << "Memory allocation failed" << endl; // print the error message
    }

    head = NULL;
  }
  // destructor of the stack class
  ~stack()
  {
    while (head != NULL)
    { // while the head node is not NULL
      node *temp = head;
      head = head->next;
      // free(temp);
      my_free(temp);
    }
  }

  // this function pushes the data to the stack
  void push(const T &new_data)
  {

    // node *check= (node*)calloc(sizeof(node));
    node *check = (node *)my_calloc(1, sizeof(node)); // allocate memory for the new node using my_calloc function
    if (check == NULL)
    {
      cout << "Null pointer has been returned" << endl;
    }

    cout << "inside push-stack" << endl; // self-checking
    node *temp = new node;                // create a new node
    temp->data = new_data;                // assign the data to the new node
    temp->next = head;                    // assign the next node to the head node
    temp->is_free = false;                // assign the status of the node to false
    temp->size = sizeof(node);            // assign the size of the node to the size of the node
    head = temp;                          // assign the head node to the new node
  }

  // this function pops the data from the stack
  void pop()
  {

    cout << "inside pop-stack" << endl; // self-checking
    if (head != NULL)
    {
      node *temp = head; // create a new node
      head = head->next; // assign the next node to the head node
      // free(temp)
      my_free((void *)temp); // free the memory of the node using my_free function
    }
  }

  // this function returns the top element of the stack
  T top() const
  {
    if (head != NULL)
    {
      return head->data;
    }
    return NULL;
  }

  // this function returns if the stack is empty or not
  bool empty() const
  {
    return head == NULL;
  }

  // this function allocates memory for the new node using the previous functions
  void *my_malloc(unsigned long size)
  {
      int *temp;
      int length = size + sizeof( size ); // Add sizeof( size ) for holding length.
      cout << "working" << endl;
      temp = (int*)mmap( 0, length, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0 );

      *temp = length;                     // First 4 bytes contain length.
      return (void*)(&temp[1]);        // Memory that is after length variable.

  }

  // this function allocate memory for number of nodes using my_malloc function
  void *my_calloc(int number, unsigned long size)
  {
    return (my_malloc(number * size));
  }
 
  // this function frees the memory
  void my_free(void *ptr){
    cout<< "inside free" << endl;
    int *temp = (int*)ptr;
    int length;

    temp--;                          // Reach top of memory
    length = *temp;                     // Read length

    munmap( (void*)temp, length );
  }
  };
