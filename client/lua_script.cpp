/*
Copyright (c) Sam Jansen and Jesse Baker.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

3. The end-user documentation included with the redistribution, if
any, must include the following acknowledgment: "This product includes
software developed by Sam Jansen and Jesse Baker 
(see http://www.wand.net.nz/~stj2/bung)."
Alternately, this acknowledgment may appear in the software itself, if
and wherever such third-party acknowledgments normally appear.

4. The hosted project names must not be used to endorse or promote
products derived from this software without prior written
permission. For written permission, please contact sam@wand.net.nz or
jtb5@cs.waikato.ac.nz.

5. Products derived from this software may not use the "Bung" name
nor may "Bung" appear in their names without prior written
permission of Sam Jansen or Jesse Baker.

THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL COLLABNET OR ITS CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
// $Id$
#include "stdafx.h"
#include "script.h"
#include "exception.h"
#include "misc.h"

#include <stack>
#include <errno.h>

extern "C" {
	#include "lua40/lua.h"
}

class CLuaScript : public CScript
{
public:
	virtual void ExecuteString(const string &s);
	virtual void ExecuteFile(const string &filename);

	virtual void Begin();
	virtual void End();

	virtual bool GetGlobal(const string &name, int *val);
	virtual bool GetGlobal(const string &name, float *val);
	virtual bool GetGlobal(const string &name, string *val);
	virtual bool GetGlobal(const string &name, Vector3f *val);
	virtual bool GetGlobal(const string &name, bool *val);

	void Init();
	void Deinit();

	CLuaScript() { Init(); }
	virtual ~CLuaScript() {}
private:
	lua_State *L;
	stack<int> stacksize;
};

CScript * CScript::Create()
{
	return new CLuaScript();
}

void CLuaScript::Init()
{
	L = lua_open(1024); // stack size=1024 for now

	ASSERT(L);

	// Init std. libraries (do we need these?)
	//lua_baselibopen(L); lua_iolibopen(L); lua_strlibopen(L); lua_mathlibopen(L);
}

void CLuaScript::Deinit()
{
	lua_close(L);
}

void CLuaScript::Begin()
{
	stacksize.push(lua_gettop(L));
}

void CLuaScript::End()
{
	int stacktop = stacksize.top(); stacksize.pop();

	lua_settop(L, stacktop);
}

void CLuaScript::ExecuteFile(const string &filename)
{
	int ret;

	if((ret = lua_dofile(L, filename.c_str())) != 0) {
		switch(ret) {
case LUA_ERRRUN: throw CException(" - error while running the chunk.");
case LUA_ERRSYNTAX: throw CException(" - syntax error during pre-compilation.");
case LUA_ERRMEM: throw CException(" - memory allocation error. For such errors, Lua does not call _ERRORMESSAGE (see Section 4.7).");
case LUA_ERRERR: throw CException(" - error while running _ERRORMESSAGE. For such errors, Lua does not call _ERRORMESSAGE again, to avoid loops.");
case LUA_ERRFILE: throw CException(bsprintf("CLuaScript::ExecuteFile : %s", strerror( errno )));
		};
		throw CException("lua_dofile broke");
	}
}

void CLuaScript::ExecuteString(const string &s)
{
	int ret;

	if((ret = lua_dostring(L, s.c_str())) != 0) {
		switch(ret) {
case LUA_ERRRUN: throw CException(" - error while running the chunk.");
case LUA_ERRSYNTAX: throw CException(" - syntax error during pre-compilation.");
case LUA_ERRMEM: throw CException(" - memory allocation error. For such errors, Lua does not call _ERRORMESSAGE (see Section 4.7).");
case LUA_ERRERR: throw CException(" - error while running _ERRORMESSAGE. For such errors, Lua does not call _ERRORMESSAGE again, to avoid loops.");
case LUA_ERRFILE: throw CException(" - error opening the file (only for lua_dofile). In this case, you may want to check errno, call strerror, or call perror to tell the user what went wrong.");
		};
		throw CException("lua_dostring broke");
	}
}

bool CLuaScript::GetGlobal(const string &name, int *val)
{
	int stacktop = lua_gettop(L), newtop = 0;

	lua_getglobal(L, name.c_str());
	newtop = lua_gettop(L);
	
	if(stacktop < newtop) {
		*val = (int)lua_tonumber(L, newtop);
		lua_settop(L, stacktop);
		return true;
	}

	return false;
}

bool CLuaScript::GetGlobal(const string &name, float *val)
{
	int stacktop = lua_gettop(L), newtop = 0;

	lua_getglobal(L, name.c_str());
	newtop = lua_gettop(L);
	
	if(stacktop < newtop) {
		*val = (float)lua_tonumber(L, newtop);
		lua_settop(L, stacktop);
		return true;
	}

	return false;
}

bool CLuaScript::GetGlobal(const string &name, string *val)
{
	int stacktop = lua_gettop(L), newtop = 0;

	lua_getglobal(L, name.c_str());
	newtop = lua_gettop(L);
	
	if(stacktop < newtop) {
		const char *s = lua_tostring(L, newtop);
		*val = string(s);
		lua_settop(L, stacktop);
		return true;
	}

	return false;
}

bool CLuaScript::GetGlobal(const string &name, Vector3f *val)
{
	int stacktop = lua_gettop(L), newtop = 0;

	lua_getglobal(L, name.c_str());
	newtop = lua_gettop(L);

	ASSERT( lua_istable(L, newtop) );

	if(stacktop < newtop) {
		lua_rawgeti(L, newtop, 1);
		val->x = (float)lua_tonumber(L, -1);

		lua_rawgeti(L, newtop, 2);
		val->y = (float)lua_tonumber(L, -1);

		lua_rawgeti(L, newtop, 3);
		val->z = (float)lua_tonumber(L, -1);

		lua_settop(L, stacktop);

		return true;
	}

	return false;
}

bool CLuaScript::GetGlobal(const string &name, bool *val)
{
	int stacktop = lua_gettop(L), newtop = 0;

	lua_getglobal(L, name.c_str());
	newtop = lua_gettop(L);

	if(stacktop < newtop) {
		if(lua_isnumber(L, newtop)) {
			if((int)lua_tonumber(L, newtop) == 0)
				*val = false;
			else
				*val = true;

			lua_settop(L, stacktop);
			return true;
		} else if(lua_isstring(L, newtop)) {
			string str(lua_tostring(L, newtop));

			if(str == "yes" || str == "true") // case sensitive???
				*val = true;
			else
				*val = false;

			lua_settop(L, stacktop);
			return true;
		}
	}

	return false;
}
