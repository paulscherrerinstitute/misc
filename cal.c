#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "epicsString.h"
#include "epicsVersion.h"

#ifdef BASE_VERSION
#define EPICS_3_13
int epicsStrGlobMatch(const char *str, const char *pattern);
#define epicsMutexMustLock(lock) FASTLOCK(&lock)
#define epicsMutexUnlock(lock)   FASTUNLOCK(&lock)
#include <inetLib.h>
#define ipAddrToDottedIP(addr, buffer, size) inet_ntoa_b((addr)->sin_addr, buffer)
struct dbFldDes{
	char	*prompt;
	char	*name;
};
#else
#include "osiSock.h"
#include "dbBase.h"
#include "dbCommon.h"
#include "iocsh.h"
#include "epicsStdioRedirect.h"
#include "epicsExport.h"
#endif

#include "server.h"

#if EPICS_VERSION*10000+EPICS_REVISION*100+EPICS_MODIFICATION < 31412
#define chanListLock addrqLock
#define chanList     addrq
#endif

#if EPICS_VERSION*10000+EPICS_REVISION*100+EPICS_MODIFICATION > 31500
#define getAddr(pciu) pciu->dbch->addr
#else
#define getAddr(pciu) pciu->addr
#endif

long cal(const char* match)
{
    struct client *client;
    int matchfield;
    char clientIP[16];
    char fullname[PVNAME_STRINGSZ+5];
    char clientref[100];

    matchfield = match && strchr(match, '.');

    LOCK_CLIENTQ
    for (client = (struct client *)ellNext(&clientQ.node); client; client = (struct client *)ellNext(&client->node))
    {
        struct channel_in_use *pciu;
        epicsMutexMustLock(client->chanListLock);
        for (pciu = (struct channel_in_use *) ellFirst(&client->chanList); pciu; pciu = (struct channel_in_use *)ellNext(&pciu->node))
        {
            const char* recname = getAddr(pciu).precord->name;
            sprintf(fullname, "%s.%s", recname, ((struct dbFldDes*)getAddr(pciu).pfldDes)->name);
            sprintf(clientref, "%.36s%s%.*s:%i",
                client->pUserName ? client->pUserName : "",
                client->pUserName ? "@" : "",
                client->pHostName ? (int)strcspn(client->pHostName, "."): 63,
                client->pHostName ? client->pHostName : (ipAddrToDottedIP(&client->addr, clientIP, sizeof(clientIP)), (clientIP)),
                ntohs (client->addr.sin_port));
            if (!match || epicsStrGlobMatch(matchfield ? fullname : recname, match)
                || epicsStrGlobMatch(client->pUserName, match)
                || epicsStrGlobMatch(client->pHostName, match)
                || epicsStrGlobMatch(clientref, match))
            {
                printf("%s ==> %s\n", clientref, fullname);
            }
        }
        epicsMutexUnlock(client->chanListLock);
    }
    UNLOCK_CLIENTQ
    return 0;
}

#ifndef EPICS_3_13
static const iocshFuncDef calDef =
    { "cal", 1, (const iocshArg *[]) {
    &(iocshArg) { "record name pattern", iocshArgString },
}};

void calFunc(const iocshArgBuf *args)
{
    cal(args[0].sval);
}

static void calRegistrar(void)
{
    iocshRegister(&calDef, calFunc);
}

epicsExportRegistrar(calRegistrar);
#endif
