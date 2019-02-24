#pragma once
#include <Windows.h>
#include "globals.h"
#include "memory.h"
#include "retcheck.h"

Ret ret;

#define r_lua_tostring(rL,i)	rlua_tolstring(rL, (i), NULL)
#define r_lua_pop(rL,n)		rlua_settop(rL, -(n)-1)
#define r_lua_getglobal(rL,s)	rlua_getfield(rL, LUA_GLOBALSINDEX, (s))
#define r_lua_newtable(rL) rlua_createtable(rL, 0, 0)

DWORD unprotect(DWORD addr)
{
	BYTE* tAddr = (BYTE*)addr;
	do
	{
		tAddr += 16;
	} while (!(tAddr[0] == 0x55 && tAddr[1] == 0x8B && tAddr[2] == 0xEC));

	DWORD funcSz = tAddr - (BYTE*)addr;

	PVOID nFunc = VirtualAlloc(NULL, funcSz, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (nFunc == NULL)
		return addr;

	memcpy(nFunc, (void*)addr, funcSz);

	BYTE* pos = (BYTE*)nFunc;
	BOOL valid = false;
	do
	{
		if (pos[0] == 0x72 && pos[2] == 0xA1 && pos[7] == 0x8B) {
			*(BYTE*)pos = 0xEB;

			DWORD cByte = (DWORD)nFunc;
			do
			{
				if (*(BYTE*)cByte == 0xE8)
				{
					DWORD oFuncPos = addr + (cByte - (DWORD)nFunc);
					DWORD oFuncAddr = (oFuncPos + *(DWORD*)(oFuncPos + 1)) + 5;

					if (oFuncAddr % 16 == 0)
					{
						DWORD relativeAddr = oFuncAddr - cByte - 5;
						*(DWORD*)(cByte + 1) = relativeAddr;

						cByte += 4;
					}
				}

				cByte += 1;
			} while (cByte - (DWORD)nFunc < funcSz);

			valid = true;
		}
		pos += 1;
	} while ((DWORD)pos < (DWORD)nFunc + funcSz);

	if (!valid)
	{
		VirtualFree(nFunc, funcSz, MEM_RELEASE);
		return addr;
	}

	return (DWORD)nFunc;
}

#define ASLR(asd)(unprotect(asd - 0x400000 + (int)GetModuleHandleA(0)))

typedef void(__cdecl *luaL_getmetafield_Def)(uintptr_t, int, const char*);
const auto rluaL_getmetafield = reinterpret_cast<luaL_getmetafield_Def>(ASLR(0x007EA860));
typedef void(__fastcall *lua_getfield_Def)(uintptr_t, int, const char*);
const auto rlua_getfield = reinterpret_cast<lua_getfield_Def>(ASLR(0x007EFC80));
typedef void(__stdcall *lua_setfield_Def)(uintptr_t, int, const char*);
const auto rlua_setfield = reinterpret_cast<lua_setfield_Def>(ASLR(0x007F1A90));
typedef void(__cdecl *lua_settable_Def)(uintptr_t, int);
const auto rlua_settable = reinterpret_cast<lua_settable_Def>(ASLR(0x007F1DF0));
typedef void(__stdcall *lua_settop_Def)(uintptr_t rL, int);
const auto rlua_settop = reinterpret_cast<lua_settop_Def>(ASLR(0x007F1E80));
typedef void(__stdcall *lua_pushnumber_Def)(uintptr_t, double);
const auto rlua_pushnumber = reinterpret_cast<lua_pushnumber_Def>(ASLR(0x007F0FD0));
typedef void(__cdecl *lua_pushnil_Def)(uintptr_t);
const auto rlua_pushnil = reinterpret_cast<lua_pushnil_Def>(ASLR(0x007F0F60));
typedef void(__cdecl *lua_pushstring_Def)(uintptr_t, const char*);
const auto rlua_pushstring = reinterpret_cast<lua_pushstring_Def>(ASLR(0x007F1060));
typedef void(__cdecl *lua_pushthread_Def)(uintptr_t);
const auto rlua_pushthread = reinterpret_cast<lua_pushthread_Def>(ASLR(0x007F10A0));
typedef void(__stdcall *lua_pushvalue_Def)(uintptr_t rL, int);
const auto rlua_pushvalue = reinterpret_cast<lua_pushvalue_Def>(ASLR(0x007F1130));
typedef int(__cdecl *lua_tothread_Def)(uintptr_t rL, int);
const auto rlua_tothread = reinterpret_cast<lua_tothread_Def>(ASLR(0x007F24C0));
typedef bool(__cdecl *lua_toboolean_Def)(uintptr_t, bool);
const auto rlua_toboolean = reinterpret_cast<lua_toboolean_Def>(ASLR(0x007F2000));
typedef double(__cdecl *lua_tonumber_Def)(uintptr_t, int);
const auto rlua_tonumber = reinterpret_cast<lua_tonumber_Def>(ASLR(0x007F23B0));
typedef int(__cdecl *lua_type_Def)(uintptr_t, int);
const auto rlua_type = reinterpret_cast<lua_type_Def>(ASLR(0x007F2510));
typedef int(__cdecl *lua_pcall_Def)(uintptr_t, int, int, int);
const auto rlua_pcall = reinterpret_cast<lua_pcall_Def>(ASLR(0x007F1860));
typedef void(__cdecl *lua_pushcclosure_Def)(DWORD, DWORD, DWORD);
const auto rlua_pushcclosure = reinterpret_cast<lua_pushcclosure_Def>(ASLR(0x007F0BD0));
typedef const char*(__stdcall *lua_tolstring_Def)(uintptr_t, size_t osize, size_t ksize);
const auto rlua_tolstring = reinterpret_cast<lua_tolstring_Def>(ASLR(0x007F2200));
typedef int(__cdecl *lua_next_Def)(uintptr_t rL, int);
const auto rlua_next = reinterpret_cast<lua_next_Def>(ASLR(0x007F0900));
typedef void(__cdecl *lua_createtable_Def)(uintptr_t, int, int);
const auto rlua_createtable = reinterpret_cast<lua_createtable_Def>(ASLR(0x007EF940));
typedef int(__cdecl *luaL_ref_def)(uintptr_t, int);
const auto rluaL_ref = reinterpret_cast<luaL_ref_def>(ret.unprotect<DWORD>((BYTE*)(x(0x007EACB0))));
typedef bool(__cdecl *lua_pushboolean_def)(uintptr_t, bool);
const auto rlua_pushboolean = reinterpret_cast<lua_pushboolean_def>(ASLR(0x007F0B50));
typedef void(__cdecl *lua_rawgeti_def)(uintptr_t, int, int);
const auto rlua_rawgeti = reinterpret_cast<lua_rawgeti_def>(ASLR(0x007F13F0));
typedef int(__cdecl *lua_newthread_def)(uintptr_t);
const auto rlua_newthread = reinterpret_cast<lua_newthread_def>(ASLR(0x007F0750));
typedef int(__cdecl *gettop_def)(uintptr_t);
const auto rlua_gettop = reinterpret_cast<gettop_def>(ASLR(0x007F0190));
typedef int(__fastcall *rlua_getmetatable_def)(uintptr_t, int);
const auto rlua_getmetatable = reinterpret_cast<rlua_getmetatable_def>(ASLR(0x007EFF60));
typedef void(__cdecl *newud_def)(uintptr_t, int);
const auto rlua_newuserdata = reinterpret_cast<newud_def>(ASLR(0x007F0840));

/*
Sehchainfaker
Eternals sehchain fixed for windows7, windows 8,...
*/
void fakeChain(DWORD* chain)
{
	chain[1] = 0x1555555;
	if ((((DWORD*)chain[0])[1]) != NULL) {
		((DWORD*)chain[0])[1] = 0x1555555;
	}
}
void restoreChain(DWORD* chain, DWORD unk, DWORD nextUnk)
{
	chain[1] = unk;
	if ((((DWORD*)chain[0])[1]) != NULL) {
		((DWORD*)chain[0])[1] = nextUnk;
	}
}

/*
r_lua_pcall
r_lua_pcall we use this to see what error roblox returns if it returns the error we want we put a breakpoint on it
and rewrite it to our own functions sadly r_lua_pcall has a check sehchaincheck which i already bypassed!
*/

