#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "VoicemeeterRemote.h"
#include "vmctl.h"

static T_VBVMR_INTERFACE iVMR;

bool flag_verbose = false;
bool flag_interactive = false;
bool flag_debug = false;

int main(int argc, char * argv[]) {
    int rep = initVoicemeeter();
    if (!rep) {
        return -1;
    }
    if (argc == 1) {
        showHelp();
        return 0;
    }
    for (int i = 1; i < argc; i++) {
        char * arg = argv[i];

        if (arg[0] == '-' || arg[0] == '/') {
            if (strcmp(arg, "-v") == 0 || strcmp(arg, "--verbose") == 0) {
                flag_verbose = true;
            } else if (strcmp(arg, "-i") == 0 || strcmp(arg, "--") == 0 || strcmp(arg, "--interactive") == 0) {
                flag_interactive = true;
            } else if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
                showHelp();
                return 0;
            } else {
                printf("Unknown option: %s\n", arg);
                showHelp();
                return -1;
            }
        }
    }
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            runCommand(argv[i]);
        }
    }

    if (flag_interactive) {
        char input[512];
        while (fgets(input, 512, stdin) != NULL) {
            if (strlen(input) > 0) {
                input[strlen(input) - 1] = '\0';
            }
            char * pos;
            char * arg = strtok_r(input, " ", & pos);
            while (arg != NULL) {
                if (strcmp(arg, "exit") == 0 || strcmp(arg, "quit") == 0) {
                    return 0;
                }
                else if (strcmp(arg, "help") == 0) {
                    showHelp();
                } 
                else if (strcmp(arg, "verbose") == 0) {
                    flag_verbose = !flag_verbose;
                    printf("Verbose mode %s\n", flag_verbose ? "enabled" : "disabled");
                }
                else if (strcmp(arg, "debug") == 0) {
                    flag_debug = !flag_debug;
                    printf("Debug mode %s\n", flag_debug ? "enabled" : "disabled");
                }
                else if (strlen(arg) > 8) { //shortest command (Option.sr) is 9 characters, 
                    runCommand(arg);
                } else {
                    printf("Unknown command: %s\n", arg);
                }

                arg = strtok_r(NULL, " ", & pos);
            }
        }
    }
    iVMR.VBVMR_Logout();
    return 0;
}

void showHelp() {
    if (flag_interactive) {
        printf("Commands:\n");
        printf("  exit, quit\t\t\tExit the program\n");
        printf("  help\t\t\t\tShow this help\n");
        printf("  verbose\t\t\tToggle verbose mode\n");
        printf("  debug\t\t\t\tToggle debug mode\n");
        printf("  interactive\t\t\tEnter interactive mode\n");
        printf("  <API call>[=value]\t\tGet/Set the specified API entry\n\n");
        printf("  Examples:\n");
        printf("   Strip[0].Gain=-1\t\tSet the gain of the first strip to -1\n");
        printf("   Strip[0].Label\t\tGet the label of the first strip\n");
        printf("   Strip[0].Label=MyStrip\tSet the label of the first strip to MyStrip\n");
    }
    else {
        printf("Usage: vmctl [options] [API call] [--]\n");
        printf("Options:\n");
        printf("  -v, --verbose\t\t\tVerbose mode: Useful for debugging and seeing what's going on\n");
        printf("  --, -i, --interactive\t\tInteractive mode: Read commands from standard input until it's closed\n");
        printf("  -h, --help\t\t\tShow this help\n");
    }
}

void WaitForUpdate() { //TODO: figure out how to do this properly, seems to be reliable for now
    int rep = iVMR.VBVMR_IsParametersDirty();
    while (rep != 0) {
        Sleep(30);
        rep = iVMR.VBVMR_IsParametersDirty();
    }
    Sleep(5);
}

int GetParameterFloat(char * param, float * cur) {
    return iVMR.VBVMR_GetParameterFloat(param, cur);
}

int GetParameterString(char * param) {
    wchar_t output[512] = {'\0'};

    int ret = iVMR.VBVMR_GetParameterStringW(param, output); //GetParameterStringA appears to exit the main loop if the parameter is not found
    WaitForUpdate();
    if (ret == 0) {
        printf("%ls\n", output);
    }
    return ret;
}

int SetParameterFloat(char * param, float val) {
    int ret = iVMR.VBVMR_SetParameterFloat(param, val);
    WaitForUpdate();
    return ret;
}

int SetParameters(char * param) {
    int ret = iVMR.VBVMR_SetParameters(param);
    WaitForUpdate();
    return ret;
}

int runCommand(char * param) {
    int ret = -1; //TODO: probably not the best way figuring out what to return

    char * tokens = strdup(param);
    char * cmd = NULL;
    char * val = NULL;

    cmd = strtok(tokens, "=");
    val = strtok(NULL, "");
    if (val == NULL) {
        // no value given, GET the parameter value
        if (flag_verbose) printf("Getting %s\n", cmd);
        if (flag_debug) printf("GetParameterString cmd: %s\n", cmd);
        ret = GetParameterString(cmd);
        if (flag_debug) printf("GetParameterString ret: %d\n", ret);

        if (ret != 0) {
            printf("Error: %d\n", ret);
            return ret;
        }
    } else {
        // value given, SET the parameter to the value
        if (flag_verbose) printf("Setting %s\n", param);
        if (flag_debug) printf("SetParameters cmd: %s\n", cmd);
        ret = SetParameters(param);
        if (flag_debug) printf("SetParameters ret: %d\n", ret);

        if (ret != 0) {
            printf("Error: %d\n", ret);
            return ret;
        }
    }

    return ret;
}

int initVoicemeeter() {
    int rep = InitializeDLLInterfaces();
    if (rep < 0) {
        if (rep == -100) {
            printf("Voicemeeter is not installed\n");
        } else {
            printf("Error loading Voicemeeter dll\n");
        }
        return FALSE;
    }
    rep = iVMR.VBVMR_Login();
    if (rep != 0) {
        printf("Error logging into Voicemeeter\n");
        return FALSE;
    }
    return TRUE;
    WaitForUpdate();
}

/*******************************************************************************/
/**                           GET VOICEMEETER DIRECTORY                       **/
/*******************************************************************************/

static char uninstDirKey[] = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";

#define INSTALLER_UNINST_KEY "VB:Voicemeeter {17359A74-1236-5467}"

void RemoveNameInPath(char * szPath) {
    long ll;
    ll = (long) strlen(szPath);
    while ((ll > 0) && (szPath[ll] != '\\')) ll--;
    if (szPath[ll] == '\\') szPath[ll] = 0;
}

#ifndef KEY_WOW64_32KEY
#define KEY_WOW64_32KEY 0x0200
#endif

BOOL __cdecl RegistryGetVoicemeeterFolder(char * szDir) {
    char szKey[256];
    char sss[1024];
    DWORD nnsize = 1024;
    HKEY hkResult;
    LONG rep;
    DWORD pptype = REG_SZ;
    sss[0] = 0;

    // build Voicemeeter uninstallation key
    strcpy(szKey, uninstDirKey);
    strcat(szKey, "\\");
    strcat(szKey, INSTALLER_UNINST_KEY);

    // open key
    rep = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey, 0, KEY_READ, & hkResult);
    if (rep != ERROR_SUCCESS) {
        // if not present we consider running in 64bit mode and force to read 32bit registry
        rep = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey, 0, KEY_READ | KEY_WOW64_32KEY, & hkResult);
    }
    if (rep != ERROR_SUCCESS) return FALSE;
    // read uninstall profram path
    rep = RegQueryValueEx(hkResult, "UninstallString", 0, & pptype, (unsigned char * ) sss, & nnsize);
    RegCloseKey(hkResult);

    if (pptype != REG_SZ) return FALSE;
    if (rep != ERROR_SUCCESS) return FALSE;
    // remove name to get the path only
    RemoveNameInPath(sss);
    if (nnsize > 512) nnsize = 512;
    strncpy(szDir, sss, nnsize);

    return TRUE;
}

/*******************************************************************************/
/**                                GET DLL INTERFACE                          **/
/*******************************************************************************/

static HMODULE G_H_Module = NULL;
static T_VBVMR_INTERFACE iVMR;

//if we directly link source code (for development only)
#ifdef VBUSE_LOCALLIB

long InitializeDLLInterfaces(void) {
    iVMR.VBVMR_Login = VBVMR_Login;
    iVMR.VBVMR_Logout = VBVMR_Logout;
    iVMR.VBVMR_RunVoicemeeter = VBVMR_RunVoicemeeter;
    iVMR.VBVMR_GetVoicemeeterType = VBVMR_GetVoicemeeterType;
    iVMR.VBVMR_GetVoicemeeterVersion = VBVMR_GetVoicemeeterVersion;
    iVMR.VBVMR_IsParametersDirty = VBVMR_IsParametersDirty;
    iVMR.VBVMR_GetParameterFloat = VBVMR_GetParameterFloat;
    iVMR.VBVMR_GetParameterStringA = VBVMR_GetParameterStringA;
    iVMR.VBVMR_GetParameterStringW = VBVMR_GetParameterStringW;

    iVMR.VBVMR_GetLevel = VBVMR_GetLevel;
    iVMR.VBVMR_GetMidiMessage = VBVMR_GetMidiMessage;
    iVMR.VBVMR_SetParameterFloat = VBVMR_SetParameterFloat;
    iVMR.VBVMR_SetParameters = VBVMR_SetParameters;
    iVMR.VBVMR_SetParametersW = VBVMR_SetParametersW;
    iVMR.VBVMR_SetParameterStringA = VBVMR_SetParameterStringA;
    iVMR.VBVMR_SetParameterStringW = VBVMR_SetParameterStringW;

    iVMR.VBVMR_Output_GetDeviceNumber = VBVMR_Output_GetDeviceNumber;
    iVMR.VBVMR_Output_GetDeviceDescA = VBVMR_Output_GetDeviceDescA;
    iVMR.VBVMR_Output_GetDeviceDescW = VBVMR_Output_GetDeviceDescW;
    iVMR.VBVMR_Input_GetDeviceNumber = VBVMR_Input_GetDeviceNumber;
    iVMR.VBVMR_Input_GetDeviceDescA = VBVMR_Input_GetDeviceDescA;
    iVMR.VBVMR_Input_GetDeviceDescW = VBVMR_Input_GetDeviceDescW;

    #ifdef VMR_INCLUDE_AUDIO_PROCESSING_EXAMPLE
    iVMR.VBVMR_AudioCallbackRegister = VBVMR_AudioCallbackRegister;
    iVMR.VBVMR_AudioCallbackStart = VBVMR_AudioCallbackStart;
    iVMR.VBVMR_AudioCallbackStop = VBVMR_AudioCallbackStop;
    iVMR.VBVMR_AudioCallbackUnregister = VBVMR_AudioCallbackUnregister;
    #endif
    #ifdef VMR_INCLUDE_MACROBUTTONS_REMOTING
    iVMR.VBVMR_MacroButton_IsDirty = VBVMR_MacroButton_IsDirty;
    iVMR.VBVMR_MacroButton_GetStatus = VBVMR_MacroButton_GetStatus;
    iVMR.VBVMR_MacroButton_SetStatus = VBVMR_MacroButton_SetStatus;

    #endif
    return 0;
}

//Dynamic link to DLL in 'C' (regular use)
#else

long InitializeDLLInterfaces(void) {
    char szDllName[1024];
    memset( & iVMR, 0, sizeof(T_VBVMR_INTERFACE));

    //get folder where is installed Voicemeeter
    if (RegistryGetVoicemeeterFolder(szDllName) == FALSE) {
        // voicemeeter not installed
        return -100;
    }
    //use right dll according O/S type
    if (sizeof(void * ) == 8) strcat(szDllName, "\\VoicemeeterRemote64.dll");
    else strcat(szDllName, "\\VoicemeeterRemote.dll");

    // Load Dll
    G_H_Module = LoadLibrary(szDllName);
    if (G_H_Module == NULL) return -101;

    // Get function pointers
    iVMR.VBVMR_Login = (T_VBVMR_Login) GetProcAddress(G_H_Module, "VBVMR_Login");
    iVMR.VBVMR_Logout = (T_VBVMR_Logout) GetProcAddress(G_H_Module, "VBVMR_Logout");
    iVMR.VBVMR_RunVoicemeeter = (T_VBVMR_RunVoicemeeter) GetProcAddress(G_H_Module, "VBVMR_RunVoicemeeter");
    iVMR.VBVMR_GetVoicemeeterType = (T_VBVMR_GetVoicemeeterType) GetProcAddress(G_H_Module, "VBVMR_GetVoicemeeterType");
    iVMR.VBVMR_GetVoicemeeterVersion = (T_VBVMR_GetVoicemeeterVersion) GetProcAddress(G_H_Module, "VBVMR_GetVoicemeeterVersion");

    iVMR.VBVMR_IsParametersDirty = (T_VBVMR_IsParametersDirty) GetProcAddress(G_H_Module, "VBVMR_IsParametersDirty");
    iVMR.VBVMR_GetParameterFloat = (T_VBVMR_GetParameterFloat) GetProcAddress(G_H_Module, "VBVMR_GetParameterFloat");
    iVMR.VBVMR_GetParameterStringA = (T_VBVMR_GetParameterStringA) GetProcAddress(G_H_Module, "VBVMR_GetParameterStringA");
    iVMR.VBVMR_GetParameterStringW = (T_VBVMR_GetParameterStringW) GetProcAddress(G_H_Module, "VBVMR_GetParameterStringW");
    iVMR.VBVMR_GetLevel = (T_VBVMR_GetLevel) GetProcAddress(G_H_Module, "VBVMR_GetLevel");
    iVMR.VBVMR_GetMidiMessage = (T_VBVMR_GetMidiMessage) GetProcAddress(G_H_Module, "VBVMR_GetMidiMessage");

    iVMR.VBVMR_SetParameterFloat = (T_VBVMR_SetParameterFloat) GetProcAddress(G_H_Module, "VBVMR_SetParameterFloat");
    iVMR.VBVMR_SetParameters = (T_VBVMR_SetParameters) GetProcAddress(G_H_Module, "VBVMR_SetParameters");
    iVMR.VBVMR_SetParametersW = (T_VBVMR_SetParametersW) GetProcAddress(G_H_Module, "VBVMR_SetParametersW");
    iVMR.VBVMR_SetParameterStringA = (T_VBVMR_SetParameterStringA) GetProcAddress(G_H_Module, "VBVMR_SetParameterStringA");
    iVMR.VBVMR_SetParameterStringW = (T_VBVMR_SetParameterStringW) GetProcAddress(G_H_Module, "VBVMR_SetParameterStringW");

    iVMR.VBVMR_Output_GetDeviceNumber = (T_VBVMR_Output_GetDeviceNumber) GetProcAddress(G_H_Module, "VBVMR_Output_GetDeviceNumber");
    iVMR.VBVMR_Output_GetDeviceDescA = (T_VBVMR_Output_GetDeviceDescA) GetProcAddress(G_H_Module, "VBVMR_Output_GetDeviceDescA");
    iVMR.VBVMR_Output_GetDeviceDescW = (T_VBVMR_Output_GetDeviceDescW) GetProcAddress(G_H_Module, "VBVMR_Output_GetDeviceDescW");
    iVMR.VBVMR_Input_GetDeviceNumber = (T_VBVMR_Input_GetDeviceNumber) GetProcAddress(G_H_Module, "VBVMR_Input_GetDeviceNumber");
    iVMR.VBVMR_Input_GetDeviceDescA = (T_VBVMR_Input_GetDeviceDescA) GetProcAddress(G_H_Module, "VBVMR_Input_GetDeviceDescA");
    iVMR.VBVMR_Input_GetDeviceDescW = (T_VBVMR_Input_GetDeviceDescW) GetProcAddress(G_H_Module, "VBVMR_Input_GetDeviceDescW");

    #ifdef VMR_INCLUDE_AUDIO_PROCESSING_EXAMPLE
    iVMR.VBVMR_AudioCallbackRegister = (T_VBVMR_AudioCallbackRegister) GetProcAddress(G_H_Module, "VBVMR_AudioCallbackRegister");
    iVMR.VBVMR_AudioCallbackStart = (T_VBVMR_AudioCallbackStart) GetProcAddress(G_H_Module, "VBVMR_AudioCallbackStart");
    iVMR.VBVMR_AudioCallbackStop = (T_VBVMR_AudioCallbackStop) GetProcAddress(G_H_Module, "VBVMR_AudioCallbackStop");
    iVMR.VBVMR_AudioCallbackUnregister = (T_VBVMR_AudioCallbackUnregister) GetProcAddress(G_H_Module, "VBVMR_AudioCallbackUnregister");
    #endif
    #ifdef VMR_INCLUDE_MACROBUTTONS_REMOTING
    iVMR.VBVMR_MacroButton_IsDirty = (T_VBVMR_MacroButton_IsDirty) GetProcAddress(G_H_Module, "VBVMR_MacroButton_IsDirty");
    iVMR.VBVMR_MacroButton_GetStatus = (T_VBVMR_MacroButton_GetStatus) GetProcAddress(G_H_Module, "VBVMR_MacroButton_GetStatus");
    iVMR.VBVMR_MacroButton_SetStatus = (T_VBVMR_MacroButton_SetStatus) GetProcAddress(G_H_Module, "VBVMR_MacroButton_SetStatus");
    #endif

    // check pointers are valid
    if (iVMR.VBVMR_Login == NULL) return -1;
    if (iVMR.VBVMR_Logout == NULL) return -2;
    if (iVMR.VBVMR_RunVoicemeeter == NULL) return -2;
    if (iVMR.VBVMR_GetVoicemeeterType == NULL) return -3;
    if (iVMR.VBVMR_GetVoicemeeterVersion == NULL) return -4;
    if (iVMR.VBVMR_IsParametersDirty == NULL) return -5;
    if (iVMR.VBVMR_GetParameterFloat == NULL) return -6;
    if (iVMR.VBVMR_GetParameterStringA == NULL) return -7;
    if (iVMR.VBVMR_GetParameterStringW == NULL) return -8;
    if (iVMR.VBVMR_GetLevel == NULL) return -9;
    if (iVMR.VBVMR_SetParameterFloat == NULL) return -10;
    if (iVMR.VBVMR_SetParameters == NULL) return -11;
    if (iVMR.VBVMR_SetParametersW == NULL) return -12;
    if (iVMR.VBVMR_SetParameterStringA == NULL) return -13;
    if (iVMR.VBVMR_SetParameterStringW == NULL) return -14;
    if (iVMR.VBVMR_GetMidiMessage == NULL) return -15;

    if (iVMR.VBVMR_Output_GetDeviceNumber == NULL) return -30;
    if (iVMR.VBVMR_Output_GetDeviceDescA == NULL) return -31;
    if (iVMR.VBVMR_Output_GetDeviceDescW == NULL) return -32;
    if (iVMR.VBVMR_Input_GetDeviceNumber == NULL) return -33;
    if (iVMR.VBVMR_Input_GetDeviceDescA == NULL) return -34;
    if (iVMR.VBVMR_Input_GetDeviceDescW == NULL) return -35;

    #ifdef VMR_INCLUDE_AUDIO_PROCESSING_EXAMPLE
    if (iVMR.VBVMR_AudioCallbackRegister == NULL) return -40;
    if (iVMR.VBVMR_AudioCallbackStart == NULL) return -41;
    if (iVMR.VBVMR_AudioCallbackStop == NULL) return -42;
    if (iVMR.VBVMR_AudioCallbackUnregister == NULL) return -43;
    #endif
    #ifdef VMR_INCLUDE_MACROBUTTONS_REMOTING
    if (iVMR.VBVMR_MacroButton_IsDirty == NULL) return -50;
    if (iVMR.VBVMR_MacroButton_GetStatus == NULL) return -51;
    if (iVMR.VBVMR_MacroButton_SetStatus == NULL) return -52;
    #endif
    return 0;
}

#endif