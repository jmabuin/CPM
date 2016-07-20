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
