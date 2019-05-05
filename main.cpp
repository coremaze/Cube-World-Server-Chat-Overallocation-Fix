#include <windows.h>
#include <iostream>
unsigned int base;
unsigned int invalid_JMP_back;
unsigned int valid_JMP_back;
void __declspec(naked) __declspec(noinline) MaliciousChatChecker(){
    asm("cmp eax, 512"); //limit chat length to 512 bytes
    asm("ja 0f"); //end communication with client

    asm("push 0"); //original code
    asm("push eax");
    asm("lea ecx, [ebp-0x28]");

    asm("jmp [_valid_JMP_back]"); //continue as normal

    asm("0:");
    asm("push eax"); //length
    asm("call [_warn_ptr]");
    asm("jmp [_invalid_JMP_back]"); //impossible to deref in jg

}
void WriteJMP(BYTE* location, BYTE* newFunction){
    DWORD dwOldProtection;
    VirtualProtect(location, 5, PAGE_EXECUTE_READWRITE, &dwOldProtection);
    location[0] = 0xE9; //jmp
    *((DWORD*)(location + 1)) = (DWORD)(( (unsigned INT32)newFunction - (unsigned INT32)location ) - 5);
    VirtualProtect(location, 5, dwOldProtection, &dwOldProtection);
}

void __stdcall warn(unsigned int length){
    printf("[Warning] A player tried to send a chat packet using a length of 0x%X, but the attempt was blocked.\n", length);
    return;
}
void* warn_ptr = (void*)&warn;

extern "C" __declspec(dllexport) bool APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            base = (unsigned int)GetModuleHandle(NULL);
            invalid_JMP_back = base + 0x266EF; //end communication with client
            valid_JMP_back = base + 0x26352; //continue as normal
            WriteJMP((BYTE*)(base + 0x2634C), (BYTE*)&MaliciousChatChecker);
            break;

    }
    return true;
}
