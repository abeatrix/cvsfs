/***************************************************************************
                          cache.c  -  description
                             -------------------
    begin                : Thu May 17 2001
    copyright            : (C) 2001 by Petric Frank
    email                : pfrank@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cvsfs_config.h"
#include "cache.h"

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/malloc.h>

#include "inode.h"
#include "proc.h"

#define CVSFS_CACHE_LEN		5
#define CVSFS_CACHE_HASH	5
#define	CVSFS_CACHE_TTL		30



struct cvsfs_hashlist_node
{
  struct cvsfs_hashlist_node	*prev;
  struct cvsfs_hashlist_node	*next;
  struct cvsfs_directory	directory;
};

struct cvsfs_dir_cache
{
  struct cvsfs_hashlist_node	*hash[CVSFS_CACHE_HASH];
  unsigned			len[CVSFS_CACHE_HASH];
};



/* local forward references */
int cvsfs_cache_deldir (struct cvsfs_hashlist_node *);
unsigned long cvsfs_cache_hash (char *);
int cvsfs_cache_infront (unsigned long, struct cvsfs_hashlist_node *);
int cvsfs_cache_shrink (unsigned long);
struct cvsfs_directory * cvsfs_cache_add (struct cvsfs_sb_info *, char *, char *);



/* local variables */
static struct cvsfs_dir_cache dir_cache;



int cvsfs_cache_init ()
{
  memset (&dir_cache, 0, sizeof (dir_cache));

  return 0;
}



int cvsfs_cache_empty ()
{
  int i;
  int j;
  struct cvsfs_hashlist_node *p;

  for (i = 0; i < CVSFS_CACHE_HASH; ++i)
    for (j = 0; j < dir_cache.len[i]; ++j)
    {
      p = dir_cache.hash[i];
      dir_cache.hash[i] = p->next;
      cvsfs_cache_deldir (p);
    }

  return 0;
}



unsigned long cvsfs_cache_hash (char * name)
{
  unsigned long hash = 0;
  int i;

  for (i = 0; i < strlen (name); ++i)
    hash += name[i];

  return hash % CVSFS_CACHE_HASH;
}



int cvsfs_cache_deldir (struct cvsfs_hashlist_node * node)
{
  struct cvsfs_dirlist_node *p;
  struct cvsfs_dirlist_node *q;

  if (!node)
    return -1;

  for (p = node->directory.head; p != NULL; p = q)
  {
    q = p->next;

    if (p->entry.name != NULL)
    {
      kfree (p->entry.name);
      p->entry.name = NULL;
      if (p->entry.version != NULL)
      {
        kfree (p->entry.version);
	p->entry.version = NULL;
      }
    }

    kfree (p);
  }

  if (node->directory.name != NULL)
    kfree (node->directory.name);

  if (node->prev != NULL)
    node->prev->next = node->next;

  if (node->next != NULL)
    node->next->prev = node->prev;

  return 0;
}



int cvsfs_cache_infront (unsigned long hash, struct cvsfs_hashlist_node *node)
{
  unsigned long h;

  if (node->prev)
  {
    h = cvsfs_cache_hash (node->directory.name);

    if (h != hash)
      ;               /* oops */

    node->prev->next = node->next;

    if (node->next)
      node->next->prev = node->prev;

    if (dir_cache.hash[h])
      dir_cache.hash[h]->prev = node;

    node->prev = NULL;
    node->next = dir_cache.hash[h];
    dir_cache.hash[h] = node;
  }

  return 0;
}



int cvsfs_cache_shrink (unsigned long hash)
{
  struct cvsfs_hashlist_node *p;

  if (dir_cache.len[hash] == 0)
    return -1;

  for (p = dir_cache.hash[hash]; p->next != NULL; p = p->next);

  cvsfs_cache_deldir (p);
  dir_cache.len[hash]--;

  return 0;
}



struct cvsfs_directory * cvsfs_cache_add (struct cvsfs_sb_info * info, char * name, char * version)
{
  unsigned long hash;
  struct cvsfs_hashlist_node *node;
#ifdef __DEBUG
  if (version == NULL)
    printk (KERN_DEBUG "cvsfs: cvsfs_cache_add - name %s\n", name);
  else
    printk (KERN_DEBUG "cvsfs: cvsfs_cache_add - name %s (version %s)\n", name, version);
#endif
  hash = cvsfs_cache_hash (name);

  while (dir_cache.len[hash] >= CVSFS_CACHE_LEN)
    cvsfs_cache_shrink (hash);

  node = (struct cvsfs_hashlist_node *) kmalloc (sizeof (struct cvsfs_hashlist_node), GFP_KERNEL);

  if (!node)
    return NULL;

  memset (node, 0, sizeof (struct cvsfs_hashlist_node));
  node->directory.valid = 1;
  node->directory.time = CURRENT_TIME;
  node->directory.name = (char *) kmalloc (strlen (name) + 1, GFP_KERNEL);

  if (node->directory.name)
  {
    strcpy (node->directory.name, name);
#ifdef __DEBUG__
    if (version == NULL)
      printk (KERN_DEBUG "cvsfs: cvsfs_cache_add - new node allocated for %s\n", name);
    else
      printk (KERN_DEBUG "cvsfs: cvsfs_cache_add - new node allocated for %s (version %s)\n", name, version);
#endif
    if (cvsfs_loaddir (info, name, &node->directory, version) >= 0)
    {
      node->prev = NULL;
      node->next = dir_cache.hash[hash];

      if (node->next)
        node->next->prev = node;

      dir_cache.hash[hash] = node;

      dir_cache.len[hash]++;

      return &node->directory;
    }

    kfree (node->directory.name);
  }

  kfree (node);

  return NULL;
}



struct cvsfs_directory * cvsfs_cache_get (struct cvsfs_sb_info * info, char * name, char * version)
{
  struct cvsfs_hashlist_node *node;
  unsigned long hash;
#ifdef __DEBUG__
  if (version == NULL)
    printk (KERN_DEBUG "cvsfs: cvsfs_cache_get - name %s\n", name);
  else
    printk (KERN_DEBUG "cvsfs: cvsfs_cache_get - name %s (version %s)\n", name, version);
#endif
  hash = cvsfs_cache_hash (name);

  for (node = dir_cache.hash[hash]; node != NULL; node = node->next)
    if ((strcmp (name, node->directory.name) == 0) && (node->directory.valid))
    {
      if ((CURRENT_TIME - node->directory.time) > CVSFS_CACHE_TTL)
      {
#ifdef __DEBUG__
        printk (KERN_DEBUG "cvsfs: cvsfs_cache_get - cache add name %s\n", name);
#endif
        return cvsfs_cache_add (info, name, version);
      }

      cvsfs_cache_infront (hash, node);
#ifdef __DEBUG__
      printk (KERN_DEBUG "cvsfs: cvsfs_cache_get - return dir entry for %s\n", name);
#endif
      return &node->directory;
    }
#ifdef __DEBUG__
  printk (KERN_DEBUG "cvsfs: cvsfs_cache_get - default add cache name %s\n", name);
#endif
  return cvsfs_cache_add (info, name, version);
}
