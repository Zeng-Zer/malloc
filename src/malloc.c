/*
** malloc.c for malloc in /home/zeng_d
**
** Made by David Zeng
** Login   <zeng_d@epitech.net>
**
** Started on  Tue Jan 24 21:09:17 2017 David Zeng
** Last update Tue Jan 24 21:09:17 2017 David Zeng
*/

#include <stdio.h>
#include "malloc.h"

pthread_mutex_t	lock = PTHREAD_MUTEX_INITIALIZER;
t_node		*head = NULL;
t_node		*last = NULL;
t_node		*free_blocks = NULL;

// TODO REMOVE COMMENTS

static void	split_block(t_node *block, size_t size)
{
  t_node	*new_block;

  //get new block address
  new_block = get_block_at(block, size);
  new_block->size = block->size - size - HEADER_SIZE;
  new_block->is_free = true;
  new_block->next = block->next;
  new_block->prev = block->prev;
  if (block->prev)
    block->prev->next = new_block;
  if (block->next)
    block->next->prev = new_block;
  if (block == free_blocks)
    free_blocks = new_block;
  //set current block to real size
  block->size = size;
  block->is_free = false;
}

/**
 * Get a block from free_blocks
 */
static void	*get_free_block(size_t size)
{
  t_node	*node;

  node = free_blocks;
  while (node)
    {
      if (node->is_free && node->size >= size)
	{
	  // split block
	  if (node->size - size >= HEADER_SIZE + MIN_BLOCK_SIZE)
	    split_block(node, size);
	  else
	    {
	      node->is_free = false;
	      if (node->prev)
	      	node->prev->next = node->next;
	      if (node->next)
	      	node->next->prev = node->prev;
	      if (node == free_blocks)
		free_blocks = NULL;
	    }
	  return (node + 1);
	}
      node = node->next;
    }
  return (NULL);
}

static void	*get_internal_memory(size_t size)
{
  static size_t	page_size = 0;
  static size_t	cur_size = 0;
  static void	*cur_ptr = NULL;
  size_t	old_size;

  if (!head)
    page_size = getpagesize() * PAGE_NUMBER;
  if (cur_size < size)
    {
      old_size = cur_size;
      while (cur_size < size)
	cur_size += page_size;
      old_size = cur_size - old_size;
      if (!cur_ptr && (cur_ptr = sbrk(old_size)) == SBRK_FAILED)
	return (NULL);
      else if (sbrk(old_size) == SBRK_FAILED)
	return (NULL);
    }
  cur_size -= size;
  cur_ptr = (char*)cur_ptr + size;
  return ((char*)cur_ptr - size);
}

/**
 * Create a new block and add it to allocated_blocks
 */
static void	*get_new_block(size_t size)
{
  t_node	*block;

  // get memory by pages
  block = get_internal_memory(size + HEADER_SIZE);
  if (!block)
    return (NULL);
  if (!head)
    head = block;
  block->size = size;
  block->is_free = false;
  block->next = NULL;
  block->prev = NULL;
  last = block;
  // add sizeof(t_node) to block with + 1
  return (block + 1);
}

/**
 * Malloc function
 */
void	*malloc(size_t size)
{
  void	*ptr;

  // check if size is okay
  if ((int)size <= 0)
    return (NULL);
  // get next power of 2
  size = (size + 3) & ~3;
  // lock thread
  pthread_mutex_lock(&lock);
  // get block from free list or new one
  ptr = get_free_block(size);
  if (!ptr)
    ptr = get_new_block(size);
  // unlock thread
  pthread_mutex_unlock(&lock);
  return (ptr);
}
