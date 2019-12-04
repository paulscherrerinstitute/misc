#define _GNU_SOURCE 1
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include "errlog.h"
#include "epicsExit.h"
#include "epicsExport.h"

int termSigDebug = 0;
epicsExportAddress(int, termSigDebug);

#ifndef vxWorks
struct sigaction old_handler_SIGHUP;
struct sigaction old_handler_SIGINT;
struct sigaction old_handler_SIGPIPE;
struct sigaction old_handler_SIGALRM;
struct sigaction old_handler_SIGTERM;
struct sigaction old_handler_SIGUSR1;
struct sigaction old_handler_SIGUSR2;

void termSigHandler(int sig, siginfo_t* info , void* ctx)
{
    /* try to clean up before exit */
    if (termSigDebug)
        errlogPrintf("\nSignal %s (%d)\n", strsignal(sig), sig);

    /* restore original handlers */
    sigaction(SIGHUP,  &old_handler_SIGHUP,  NULL);
    sigaction(SIGINT,  &old_handler_SIGINT,  NULL);
    sigaction(SIGPIPE, &old_handler_SIGPIPE, NULL);
    sigaction(SIGALRM, &old_handler_SIGALRM, NULL);
    sigaction(SIGTERM, &old_handler_SIGTERM, NULL);
    sigaction(SIGUSR1, &old_handler_SIGUSR1, NULL);
    sigaction(SIGUSR2, &old_handler_SIGUSR2, NULL);

    epicsExitCallAtExits(); /* will only start executing handlers once */
    /* send the same signal again */
    raise(sig);
}
#endif

static void termSigExitFunc(void* arg)
{
    if (termSigDebug)
        errlogPrintf("Exit handlers executing\n");
}

static void termSigRegistrar (void)
{
#ifndef vxWorks
    struct sigaction sa = {{0}};

    /* make sure that exit handlers are called even at signal termination (e,g CTRL-C) */
    sa.sa_sigaction = termSigHandler;
    sa.sa_flags = SA_SIGINFO;

    sigaction(SIGHUP,  &sa, &old_handler_SIGHUP);
    sigaction(SIGINT,  &sa, &old_handler_SIGINT);
    sigaction(SIGPIPE, &sa, &old_handler_SIGPIPE);
    sigaction(SIGALRM, &sa, &old_handler_SIGALRM);
    sigaction(SIGTERM, &sa, &old_handler_SIGTERM);
    sigaction(SIGUSR1, &sa, &old_handler_SIGUSR1);
    sigaction(SIGUSR2, &sa, &old_handler_SIGUSR2);
#endif
    epicsAtExit(termSigExitFunc, NULL);
}

epicsExportRegistrar(termSigRegistrar);
