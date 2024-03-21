// dllmain.cpp : Define o ponto de entrada para o aplicativo DLL.
#include "pch.h"
#include <Windows.h>

HMODULE hndOpengl = NULL;
unsigned char *hookLocation;

void(__stdcall* glDepthFunc)(unsigned int) = NULL; // GlEnum
DWORD returnAddress = 0;

__declspec(naked) void codecave() {
    __asm {
        pushad
    }
    (*glDepthFunc)(0x0207); // GL_ALWAYS

    __asm {
        popad
        mov esi, dword ptr ds : [esi + 0xA18] // Funcao para jmp que achamos no x86
        jmp returnAddress
    }
}

void threadWh() {
    while (true) {
        if (hndOpengl == NULL) {
            hndOpengl = GetModuleHandle(L"opengl32.dll");

            glDepthFunc = (void(__stdcall*)(unsigned int))GetProcAddress(hndOpengl, "glDepthFunc");
        }
        if (hndOpengl != NULL) {
            hookLocation = (unsigned char*)GetProcAddress(hndOpengl, "glDrawElements");
            hookLocation += 0x16; // endereço da funcao mov que vamos substituir pelo jmp para nossa code cave
        
            DWORD oldProtect;
            VirtualProtect((void*)hookLocation, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
            *hookLocation = 0xE9; // opcode do jmp (so pesquisar)
            *(DWORD*)(hookLocation + 1) = (DWORD)&codecave - ((DWORD)hookLocation + 5);
            *(hookLocation + 5) = 0x90; // NOP offset

            returnAddress = (DWORD)(hookLocation + 0x6);
        }
        Sleep(1);
    
    }
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH: 
        CloseHandle(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadWh, NULL, 0, NULL));
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

