/*
 *
 *  Created on: 28 Jan 2019
 *      Author: jcalv
 */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <execinfo.h>
#include <sys/types.h>
#include <fcntl.h>
#include <gnu/libc-version.h>
#include <sys/utsname.h>
#include <ucontext.h> 
#include "libapiinc.h"

struct sigaction act;
char wrkdir[256];

#define EXCEPTION_FILE_NAME "exception.log"

static void sighandler(int signum, siginfo_t *info, void *ptr)
{
	if (signum == SIGTERM)
	{
		// restore default handlers - we get a lot of exceptions during shut down
		signal(SIGSEGV, SIG_DFL);
		signal(SIGBUS,  SIG_DFL);
		signal(SIGFPE,  SIG_DFL);
		signal(SIGILL,  SIG_DFL);
		signal(SIGABRT, SIG_DFL);	// Bug 3725

		fprintf(stdout, "Received signal %d\n", signum);
		fprintf(stdout, "Signal originates from process %lu\n", (unsigned long)info->si_pid);
		exit(0);
	}
	else if (  (signum == SIGABRT) || (signum == SIGSEGV) || (signum == SIGBUS)
			|| (signum == SIGFPE)  || (signum == SIGILL)
			)
	{
		int output_fd;

		fprintf(stdout, "Received signal %d\n", signum);

		chdir(wrkdir);

		output_fd = open(EXCEPTION_FILE_NAME, O_WRONLY | O_CREAT, 0644);
		if (output_fd == -1)
		{
			// error
			fprintf(stdout, "Failed to create %s\n", EXCEPTION_FILE_NAME );
		}
		else
		{
			void *btbuffer[256];
			char fbuffer[128];
			int nptrs;
			time_t rawtime;
			struct tm * timeinfo;
			char timeBuf[24];

			sprintf(fbuffer, "An error has occurred and the\n" );
			write (output_fd, fbuffer, strlen(fbuffer));
			sprintf(fbuffer, "Payment Application software\n" );
			write (output_fd, fbuffer, strlen(fbuffer));
			sprintf(fbuffer, "has automatically restarted.\n" );
			write (output_fd, fbuffer, strlen(fbuffer));
			sprintf(fbuffer, "Please scan this report and\n" );
			write (output_fd, fbuffer, strlen(fbuffer));
			sprintf(fbuffer, "contact your help-desk for advice.\n\n" );
			write (output_fd, fbuffer, strlen(fbuffer));

			sprintf(fbuffer, "Exception report:\n" );
			write (output_fd, fbuffer, strlen(fbuffer));

#ifndef __DEBUG__
			strcpy(fbuffer, "Build mode (Release)\n");
#else
			strcpy(fbuffer, "Build mode (Debug)\n");
#endif
			write (output_fd, fbuffer, strlen(fbuffer));

			// Print the working directory at the time exception occurred
			sprintf(fbuffer, "Dir:  %.33s\n", wrkdir );
			write (output_fd, fbuffer, strlen(fbuffer));

			// timestamp
			time(&rawtime);
			timeinfo = localtime(&rawtime);
			strftime((char *) timeBuf, sizeof(timeBuf), "%d-%m-%y %H:%M:%S", timeinfo);
			sprintf(fbuffer, "Time: %s\n", timeBuf);
			write (output_fd, fbuffer, strlen(fbuffer));

			// signal number, faulting ins/mem addr
			sprintf(fbuffer, "Signal: %02d, faulting ins/mem  =%08lx\n", signum, (unsigned long)info->si_addr);
			write (output_fd, fbuffer, strlen(fbuffer));

			// Registers
			ucontext_t *u = (ucontext_t *)ptr;
			unsigned long pc = u->uc_mcontext.arm_pc;
			unsigned long fa = u->uc_mcontext.fault_address;
			unsigned long r0 = u->uc_mcontext.arm_r0;
			unsigned long r1 = u->uc_mcontext.arm_r1;
			unsigned long r2 = u->uc_mcontext.arm_r2;
			unsigned long r3 = u->uc_mcontext.arm_r3;
			unsigned long r4 = u->uc_mcontext.arm_r4;
			unsigned long r5 = u->uc_mcontext.arm_r5;
			unsigned long r6 = u->uc_mcontext.arm_r6;
			unsigned long r7 = u->uc_mcontext.arm_r7;
			unsigned long r8 = u->uc_mcontext.arm_r8;
			unsigned long r9 = u->uc_mcontext.arm_r9;
			unsigned long r10= u->uc_mcontext.arm_r10;
			unsigned long fp = u->uc_mcontext.arm_fp;
			unsigned long ip = u->uc_mcontext.arm_ip;
			unsigned long sp = u->uc_mcontext.arm_sp;
			unsigned long lr = u->uc_mcontext.arm_lr;
			sprintf(fbuffer, "ctx: pc= %08lx, fa= %08lx\n", pc, fa);
			write (output_fd, fbuffer, strlen(fbuffer));
			sprintf(fbuffer, "    r0=%08lx r1=%08lx r2=%08lx\n", r0, r1, r2);
			write (output_fd, fbuffer, strlen(fbuffer));
			sprintf(fbuffer, "    r3=%08lx r4=%08lx r5=%08lx\n", r3, r4, r5);
			write (output_fd, fbuffer, strlen(fbuffer));
			sprintf(fbuffer, "    r6=%08lx r7=%08lx r8=%08lx\n", r6, r7, r8);
			write (output_fd, fbuffer, strlen(fbuffer));
			sprintf(fbuffer, "    r9=%08lx 10=%08lx fp=%08lx\n", r9, r10, fp);
			write (output_fd, fbuffer, strlen(fbuffer));
			sprintf(fbuffer, "    ip=%08lx sp=%08lx lr=%08lx\n", ip, sp, lr);
			write (output_fd, fbuffer, strlen(fbuffer));

			sprintf(fbuffer, "Built-in addr = %p (%p)\n",
					__builtin_extract_return_addr(__builtin_return_address(0)),
					__builtin_return_address(0));
			write (output_fd, fbuffer, strlen(fbuffer));
			nptrs = backtrace(btbuffer, 16);
			sprintf(fbuffer, "bt: %d addresses (max 16)\n", nptrs);
			write (output_fd, fbuffer, strlen(fbuffer));
			backtrace_symbols_fd(btbuffer, nptrs, output_fd); // async signal safe

			sprintf(fbuffer, "End of report\n" );
			write (output_fd, fbuffer, strlen(fbuffer));

			close (output_fd);
		}

		// restore default handle for this signal
		signal(signum, SIG_DFL);
		NAPI_SysReboot();
		while(1) sleep(1);
		kill(getpid(), signum);
	}
	else
	{
		fprintf(stdout, "Received unexpected signal %d\n", signum);
		fprintf(stdout, "Signal originates from process %lu\n", (unsigned long)info->si_pid);
	}
}

void InitException(void)
{
//	sigset_t set;
	struct utsname unameData;
	pid_t pid;

	// Save application's home directory
	getcwd(wrkdir, sizeof(wrkdir));
	fprintf(stdout, "Apps home directory: %s\n", wrkdir);

/*
	// Block all signals before creating application threads
	// so that each thread will inherit the mask
	// we will then unblock signals in main() thread to ensure
	// at least one thread will receive signals
	sigfillset(&set);

	// we override handler for SIGTERM, so don't include it in the set(?)
	sigdelset(&set, SIGTERM);
	sigdelset(&set, SIGSEGV);
	sigdelset(&set, SIGBUS);
	sigdelset(&set, SIGFPE);
	sigdelset(&set, SIGILL);
	sigdelset(&set, SIGABRT);

	if ( pthread_sigmask(SIG_SETMASK, &set, NULL) )
	{
		fprintf(stdout, "can't block signals, terminating main()\n" );
		exit(-1);
	}
*/
    // register signal handler
	memset(&act, 0, sizeof(act));
	act.sa_sigaction = sighandler;
	act.sa_flags = SA_SIGINFO;
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGSEGV, &act, NULL);
	sigaction(SIGBUS,  &act, NULL);
	sigaction(SIGFPE,  &act, NULL);
	sigaction(SIGILL,  &act, NULL);
	sigaction(SIGABRT, &act, NULL);

	if ( uname(&unameData) == 0 )
	{
		fprintf(stdout, "OS name:    %s\n", unameData.sysname);
		fprintf(stdout, "OS release: %s\n", unameData.release);
		fprintf(stdout, "OS version: %s\n", unameData.version);
		fprintf(stdout, "Hardware identifier: %s\n", unameData.machine);
	}
	fprintf(stdout, "libc version: %s\n", gnu_get_libc_version());
	fprintf(stdout, "libc release: %s\n", gnu_get_libc_release());

	pid = getpid();
	fprintf(stdout, "PID = %d\n", pid);
}

