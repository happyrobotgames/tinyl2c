#include "tinyl2c.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

//////////////////////////////////////////////////////////////////////////
//Print function that outputs to debug console in windows in addition 
//to std out
//////////////////////////////////////////////////////////////////////////
void l2c_printf(const char * format, ...)
{
	char buffer[1024];
	va_list args;
	va_start(args, format);
	vsprintf_s(buffer, format, args);
#ifdef _WIN32
	OutputDebugStringA(buffer);
#endif
	printf(buffer);
	va_end(args);
}

//////////////////////////////////////////////////////////////////////////
//The LUA hooks that invoke a variable or function class. They expect the
//first up value of the call to be a pointer to the class, stored as
//light user data. Below are the functions used to register them with LUA
//////////////////////////////////////////////////////////////////////////
int l2cinternal_variable_get(lua_State* L)
{
	L2CVariable* variable = (L2CVariable*)lua_touserdata(L,lua_upvalueindex(1));
	if (!variable)
		luaL_error(L, "Invalid variable access");
	return variable->Get(L);
}
int l2cinternal_variable_set(lua_State* L)
{
	L2CVariable* variable = (L2CVariable*)lua_touserdata(L,lua_upvalueindex(1));
	if (!variable)
		luaL_error(L, "Invalid variable access");
	return variable->Set(L);
}
int l2cinternal_function_invoke(lua_State* L)
{
	L2CFunction* func = (L2CFunction*)lua_touserdata(L,lua_upvalueindex(1));
	if (!func)
		luaL_error(L, "Invalid variable access");
	return func->Invoke(L);
}

int l2cinternal_create_variable_get(lua_State* L, L2CVariable* variable)
{
	lua_pushlightuserdata(L,variable);
	lua_pushcclosure(L,l2cinternal_variable_get,1);
	return 1;
}
int l2cinternal_create_variable_set(lua_State* L, L2CVariable* variable)
{
	lua_pushlightuserdata(L,variable);
	lua_pushcclosure(L,l2cinternal_variable_set,1);
	return 1;
}
int l2cinternal_create_function_invoke(lua_State* L, L2CFunction* function)
{
	lua_pushlightuserdata(L,function);
	lua_pushcclosure(L,l2cinternal_function_invoke,1);
	return 1;
}

//////////////////////////////////////////////////////////////////////////
//Core table get/set functions. Each expects up values to be:
//1: inherits from (or nil)
//2: member table (getters for index, setters for new index)
//3: function table
//////////////////////////////////////////////////////////////////////////
int l2cinternal_meta_index(lua_State* L)
{
	//read out all the shizzle
	int inherits_idx = lua_upvalueindex(1);
	int membertable_idx = lua_upvalueindex(2);
	int functiontable_idx = lua_upvalueindex(3);
	int instance_idx = lua_absindex(L,-2);
	int key_idx = lua_absindex(L,-1);

	//first look for a member
	lua_pushvalue(L,key_idx);
	lua_gettable(L,membertable_idx);
	if (!lua_isnil(L, -1))
	{
		//got one, so push instance onto stack and invoke it
		lua_pushvalue(L,instance_idx);
		lua_call(L,1,1);
		return 1;
	}
	lua_pop(L,1);

	//now look for a function
	lua_pushvalue(L,key_idx);
	lua_gettable(L,functiontable_idx);
	if (!lua_isnil(L, -1))
	{
		//got one, so just return it 
		return 1;
	}
	lua_pop(L,1);

	//if the inherits value is not nil, call its index function
	if (!lua_isnil(L, inherits_idx))
	{
		lua_pushvalue(L,inherits_idx);
		lua_pushvalue(L, instance_idx);
		lua_pushvalue(L, key_idx);
		lua_call(L,2,1);
		return 1;
	}

	//couldn't find it at all, so fail
	return luaL_error(L, "Member not found");
}
int l2cinternal_meta_newindex(lua_State* L)
{
	//read out all the shizzle
	int inherits_idx = lua_upvalueindex(1);
	int membertable_idx = lua_upvalueindex(2);
	int functiontable_idx = lua_upvalueindex(3);
	int instance_idx = lua_absindex(L,-3);
	int key_idx = lua_absindex(L,-2);
	int value_idx = lua_absindex(L,-1);

	//first look for a member
	lua_pushvalue(L,key_idx);
	lua_gettable(L,membertable_idx);
	if (!lua_isnil(L, -1))
	{
		//got one, so push instance and value onto stack and invoke it
		lua_pushvalue(L,instance_idx);
		lua_pushvalue(L,value_idx);
		lua_call(L,2,0);
		return 0;
	}
	lua_pop(L,1);

	//now look for a function
	lua_pushvalue(L,key_idx);
	lua_gettable(L,functiontable_idx);
	if (!lua_isnil(L, -1))
	{
		//got one - can't overwrite functions, so error!
		return luaL_error(L,"Cant overwrite functions");
	}
	lua_pop(L,1);

	//if the inherits value is not nil, call its index function
	if (!lua_isnil(L, inherits_idx))
	{
		lua_pushvalue(L,inherits_idx);
		lua_pushvalue(L, instance_idx);
		lua_pushvalue(L, key_idx);
		lua_pushvalue(L, value_idx);
		lua_call(L,3,0);
		return 0;
	}

	//couldn't find it at all, so fail
	return luaL_error(L, "Member not found");
}
