//=============================================================================
// Copyright (C) 2011 Athip Rooprayochsilp <athipr@gmail.com>
//=============================================================================

#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#endif

#include <stdio.h>
#include "Daemonizer.h"

Daemonizer::Daemonizer()
{

}

Daemonizer::~Daemonizer()
{
}

bool
Daemonizer::initialize()
{
#ifndef _WIN32
  pid_t pid;

  if ((pid = fork()) < 0)
  {
    fprintf(stderr, "Failed to fork the process.\n");
    return false;
  }
  else if (pid)
  {
    // Parent
    return false;    
  }

  if (setsid() < 0)
  {
    fprintf(stderr, "Failed to setsid to the process.\n");
    return false;
  }

  signal(SIGHUP, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);

  if ((pid = fork()) < 0)
  {
    fprintf(stderr, "Failed to fork the process.\n");
    return false;
  }
  else if (pid)
  {
    // First child
    return false;    
  }

  uint16_t MAXFD = getdtablesize();
  for (uint16_t i = 0; i < MAXFD; i += 1)
  {
    if (i != 1 && i != 2) // we still need stdout and stderr
      close(i);
  }
  // redirect stdin to /dev/null
  open("/dev/null", O_RDONLY);
#endif

  return true;
}

