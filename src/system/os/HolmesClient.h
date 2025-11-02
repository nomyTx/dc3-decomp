#pragma once
#include "os/NetworkSocket.h"
#include "os/File.h"

bool UsingHolmes(int);
int HolmesClientSysExec(const char *);
void HolmesClientReInit();
void HolmesClientPrint(const char *);
void HolmesClientTerminate();
void HolmesClientInit();
NetAddress HolmesResolveIP();
void HolmesClientPollKeyboard();
int HolmesClientGetStat(const char *, FileStat &);
int HolmesClientDelete(const char *);
int HolmesClientMkDir(const char *);
const char *HolmesFileShare();
void HolmesClientEnumerate(
    const char *, void (*)(const char *, const char *), bool, const char *, bool
);
void HolmesClientStackTrace(const char *, struct StackData *, int, String &);
void HolmesClientTruncate(int, int);
bool HolmesClientOpen(const char *, int, unsigned int &, int &);
void HolmesClientWrite(int, int, int, const void *);
void HolmesClientRead(int, int, int, void *, File *);
bool HolmesClientReadDone(File *);
void HolmesClientClose(File *, int);
