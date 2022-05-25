#include <iostream>
#include <cstdlib>
#include <pthread.h>
#include <sys/mman.h>
#include <vector>
#include <fcntl.h>
#include <arpa/inet.h>
using namespace std;

// stack class implementation

int fd;
struct flock fl;

void fileBlock()
{

  fd = open("stack.txt", O_RDWR, S_IRUSR | S_IWUSR);
  if (fd == -1)
  {
    perror("Error opening file for writing");
  }
  memset(&fl, 0, sizeof(fl));
}

typedef struct stack
{
  char arr[1024];
  int top_counter = 0;
  int size = 0;

} stack, *stackptr;

//this function pushes the data to the stack
void *push(string &new_data, stackptr ss)
{
  memset(&fl, 0, sizeof(fl));
  fl.l_type = F_WRLCK;
  fcntl(fd, F_SETLKW, &fl);
  cout << "inside push stack" << endl;
  strcpy(ss[ss->top_counter++].arr, new_data.c_str());
  ss->size++;
  cout << "size of stack is " << ss->size << endl;
  cout << "top counter is " << ss->top_counter << endl;
  cout << "pushed" << endl;
  fl.l_type = F_UNLCK;
  fcntl(fd, F_SETLKW, &fl);
  close(fd);
  return NULL;
}

//this function pops the data from the stack
void pop(stackptr &ss)
{

  if (ss->top_counter == 0)
  {
    cout << "stack is empty" << endl;
    return;
  }
  else
  {
    fl.l_type = F_WRLCK;
    fcntl(fd, F_GETLK, &fl);
    char *out = ss[ss->top_counter - 1].arr;
    cout << "out:  " << out << endl;
    strcpy(ss[ss->top_counter - 1].arr, "");
    ss->size--;
    fl.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &fl);

    return;
  }
}

//this function returns the top element of the stack
string top(stackptr &ss)
{
  if (ss->top_counter == 0)
  {
    return "stack is empty";
  }
  else
  {
    fl.l_type = F_WRLCK;
    fcntl(fd, F_GETLK, &fl);
    cout << "inside top" << endl;
    char *temp = ss[ss->top_counter - 1].arr;

    char out[1024];
    bzero(out, 1024);
    strcat(out, temp);
    cout << "out:  " << out << endl;

    cout << "inside top stack" << endl;
    fl.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &fl);
    close(fd);
    return out;
  }
}
