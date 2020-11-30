#include "mem.h"

typedef struct __node_t node_t;
typedef struct __header_t header_t;

// header of each allocated space
struct __header_t
{
  int size;
  int magic;
};

// each node of free list
struct __node_t
{
  int size; // size available to allocate
  struct __node_t *next;
  struct __node_t *prev;
  int policy;
};

node_t *head = NULL; // head points to the start of the free list, may change when allocating memory
int calledNum = 0;
int const header_size = sizeof(header_t);

int Mem_Init(int region_size, int policy)
{
  printf("node_t:   %ld\n", sizeof(node_t));
  printf("header_t: %ld\n", sizeof(header_t));
  if (calledNum != 0)
  {
    printf("Mem_Init has been called\n");
    return -1;
  }
  if (region_size <= 0)
  {
    fprintf(stderr, "size initialized error\n");
    return -1;
  }
  int page_size = getpagesize();
  int extra_size = 0;
  extra_size = region_size % page_size;
  extra_size = page_size - extra_size;

  // calling mmap
  // open the /dev/zero device
  int fd = open("/dev/zero", O_RDWR);
  if (fd == -1)
  {
    printf("fd error\n");
  }
  // region_size (in bytes) needs to be evenly divisible by the page size
  void *ptr = mmap(NULL, region_size + extra_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  if (ptr == MAP_FAILED)
  {
    perror("mmap");
    return -1;
  }
  // close the device (don't worry, mapping is unaffected)
  close(fd);
  head = (node_t *)ptr;
  head->size = region_size + extra_size - sizeof(node_t);
  head->next = NULL;
  head->prev = NULL;
  head->policy = policy;
  return 0;
}

void *Mem_Alloc(int size)
{
  if (size <= 0)
  {
    return NULL;
  }
  int newSize = size;
  if (size % 4 != 0)
  {
    int extra_size = size % 4;
    newSize = size + 4 - extra_size;
  }
  // if the free list has only the head, then can only allocate memory to the head
  // need to make sure that there's space for the (node_t)
  if (head->next == NULL && newSize + sizeof(header_t) <= head->size)
  {
    // assign the memory starting at head to the header+size
    // add the header to the allocated space
    node_t *tmp = head;
    int old_size = head->size;
    head = (node_t *)((char *)head + sizeof(header_t) + newSize);

    head->size = old_size - sizeof(header_t) - newSize;
    head->next = NULL;
    head->prev = NULL;
    head->policy = 1;
    //*((node_t*)((char*)head+sizeof(header_t)+newSize)) = *head;
    header_t *header = (header_t *)tmp;
    header->size = newSize;
    header->magic = 111;
    void *return_ptr = (void *)((char *)header + sizeof(header_t));

    return return_ptr;
  }
  // if the free list is not NULL
  // best fit
  if (head->policy == 1)
  {
    // traverse the node, see if there's exactly-fit node
    node_t *tmp = head;
    while (tmp != NULL)
    {
      // the newSize wanted to be allocated equals to the size can be allocated of the node of the free list
      if (tmp->size + sizeof(node_t) == sizeof(header_t) + newSize)
      {
        void *return_ptr = (void *)((char *)tmp + sizeof(header_t));
        // store the header and size
        header_t *header = (header_t *)tmp;
        header->size = newSize;
        header->magic = 111;
        // if tmp is head
        if ((node_t *)tmp == (node_t *)head)
        {
          // manage the free list
          head->next->prev = NULL;
          head->next->policy = 1;
          head = head->next;
          head->policy = 1;
          return return_ptr;
        }
        // if tmp is not head
        if ((node_t *)tmp != (node_t *)head)
        {
          // manage the free list
          // if tmp is the last block: tmp->next == NULL
          if (tmp->next == NULL)
          {
            tmp->prev->next = NULL;
          }
          // if tmp is not the last block: tmp->next != NULL
          if (tmp->next != NULL)
          {
            tmp->prev->next = tmp->next;
            tmp->next->prev = tmp->prev;
          }
          return return_ptr;
        }
      }
      tmp = tmp->next;
    }

    // cannot find the exactly-fit node in the free list
    tmp = head; // reset tmp
    // in order to make sure there's still a node_t that can store the free memory, after allocating header and newSize, there should still be space for a node_t
    // need to find the smallest free node tmp s.t. sizeof(node_t) + tmp->size > sizeof(header_t) + newSize + sizeof(node_t)
    //                                         i.e. tmp->size > sizeof(header_t) + newSize
    node_t *node_fit = head;
    while (tmp->next != NULL)
    {
      if (tmp->size > tmp->next->size && tmp->next->size >= sizeof(header_t) + newSize)
      {
        node_fit = tmp->next;
      }
      tmp = tmp->next;
    }
    // now we have found the node_fit which is the smallest fit node
    // if node_fit is the head
    if (node_fit == head)
    {
      *((node_t *)((char *)head + sizeof(header_t) + newSize)) = *head;
      header_t *header = (header_t *)head;
      header->size = newSize;
      header->magic = 111;
      void *return_ptr = (void *)((char *)header + sizeof(header_t));
      head = (node_t *)((char *)head + sizeof(header_t) + newSize);
      int old_size = head->size;
      head->size = old_size - sizeof(header_t) - newSize;
      head->next = NULL;
      head->policy = 1;
      return return_ptr;
    }
    else
    {
      node_t *prevNode = node_fit->prev;
      node_t *nextNode = node_fit->next;
      node_t *thisNode = NULL;
      *((node_t *)((char *)node_fit + sizeof(header_t) + newSize)) = *node_fit;
      header_t *header = (header_t *)node_fit;
      header->size = newSize;
      header->magic = 111;
      void *return_ptr = (void *)((char *)node_fit + sizeof(header_t));
      int old_size = node_fit->size;
      thisNode = (node_t *)((char *)node_fit + sizeof(header_t) + newSize);
      thisNode->size = old_size - sizeof(header_t) - newSize;
      prevNode->next = thisNode;
      nextNode->prev = thisNode;
      return return_ptr;
    }
    // if node_fit is the last node

    // if node_fit is neither the last node nor the head
  }
  return NULL;
}

int Mem_Free(void *ptr)
{
  if (ptr == NULL)
  {
    return -1;
  }
  header_t *hptr = (header_t *)ptr - 1;
  // now, we have the header of the space we want to free
  // hptr->size
  assert(hptr->magic == 111);
  // manage the free list
  if ((char *)head > (char *)hptr)
  {
    node_t *tmp = head;
    head = (node_t *)hptr;
    head->size = hptr->size + sizeof(header_t) - sizeof(node_t);
    head->next = tmp;
    head->prev = NULL;
    head->policy = 1;
    tmp->prev = head;
  }
  if ((char *)head < (char *)hptr)
  {
    node_t *tmp = head;
    while (tmp != NULL)
    {
      if ((size_t *)tmp < (size_t *)hptr)
      {
        tmp = tmp->next;
        continue;
      }
      if ((size_t *)tmp > (size_t *)hptr)
      {
        break;
      }
    }
    // now, tmp points to the first free block right after hptr
    // tmp can be NULL, or NOT NULL
    if (tmp != NULL)
    { // tmp NOT NULL, then add a block between tmp and tmp->prev
      node_t *free_node = (node_t *)hptr;
      free_node->size = sizeof(header_t) + hptr->size - sizeof(node_t);
      free_node->next = tmp;
      free_node->prev = tmp->prev;
      tmp->prev->next = free_node;
      tmp->prev = free_node;
    }
    if (tmp == NULL)
    { // tmp NULL, then add a block to the last of free list
      // need to find the last block of free list
      node_t *x = head;
      while (x->next != NULL)
      {
        x = x->next;
      }
      // now, x is the last block
      node_t *free_node = (node_t *)hptr;
      free_node->size = sizeof(header_t) + hptr->size - sizeof(node_t);
      free_node->next = NULL;
      free_node->prev = x;
      x->next = free_node;
    }
  }
  // need to coallase the free list
  node_t *x = head;
  while (x->next != NULL)
  {
    if ((char *)x + sizeof(node_t) + x->size == (char *)x->next)
    {
      x->size = x->size + sizeof(node_t) + x->next->size;
      if (x->next->next == NULL)
      {
        x->next = NULL;
        break;
      }
      else
      {
        x->next->next->prev = x;
        x->next = x->next->next;
        printf("xxxxxxxxxxxxxxxxxxxxxxxx\n");
      }
    }
    x = x->next;
  }

  return 0;
}

//this routine simply prints which regions are currently free and should be used by you for debugging purposes.
void Mem_Dump()
{
  printf("free list:\n");
  node_t *tmp = head;
  while (tmp != NULL)
  {
    printf("address:%p, size:%d\n", (void *)tmp, tmp->size);
    tmp = tmp->next;
  }
}

#define P_BESTFIT (1)
#define P_WORSTFIT (2)
#define P_FIRSTFIT (3)