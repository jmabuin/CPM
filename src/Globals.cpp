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

#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>
#include "Globals.h"

int DEBUG_MODE = 0;


void setDebugMode(int new_mode) {

	DEBUG_MODE = new_mode;

}

int getDebugMode() {
	return DEBUG_MODE;
}

void printFunction (int error, const char* format, ...) {

	va_list argptr;
	va_start(argptr, format);

	if (DEBUG_MODE) {
		vfprintf(stderr, format, argptr);
	}
	else if (error) {
		syslog(LOG_ERR, format, argptr);
	}
	else{
		syslog(LOG_INFO, format, argptr);
	}

}

