/**
  * Copyright 2016 José Manuel Abuín Mosquera <josemanuel.abuin@usc.es>
  * 
  * This file is part of CPM.
  *
  * CPM is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * CPM is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  * 
  * You should have received a copy of the GNU General Public License
  * along with CPM. If not, see <http://www.gnu.org/licenses/>.
  */


#include <sys/procfs.h>

#include <proc/readproc.h>
#include <proc/sysinfo.h>

#include <pwd.h>
#include <proc/sysinfo.h>
#include <syslog.h>
#include <vector>
#include <signal.h>
#include <pthread.h>
#include <map>
#include <cstdlib>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "Network.h"
#include "PapiCounts.h"
#include "Energy.h"

void searchAndSendInfo(struct ProcessesInfo rxMsg);
