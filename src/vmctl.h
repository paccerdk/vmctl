#ifndef __VMCTL_H__
#define __VMCTL_H__

extern bool flag_interactive;
extern bool flag_verbose;

void showHelp();

void WaitForUpdate();

int GetParameterFloat(char *param,float *cur);

int GetParameterString(char *param);

int SetParameterFloat(char *param,float val);

int SetParameters(char *param);

int runCommand(char *param);

int initVoicemeeter();

void RemoveNameInPath(char *szPath);

BOOL __cdecl RegistryGetVoicemeeterFolder(char *szDir);

long InitializeDLLInterfaces(void);

#endif /* ! defined __VMCTL_H__ */