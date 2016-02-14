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
//The LUA hooks that invoke the getter or setter for a variable via
//a variable functor. The functions expect the first up value to be
//a pointer to the c++ functor.
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

//////////////////////////////////////////////////////////////////////////
//Similar but a bit more advanced than variables, these invoke functions.
//Each expects a user data structure with a header followed by n functors
//as the upvalue. This allows for handling of overloaded functions with
//different signatures
//////////////////////////////////////////////////////////////////////////
struct L2CInvokeHeader
{
	int32_t m_num_functions;
	L2CFunction* m_functions[1];
};
int l2cinternal_function_invoke(lua_State* L)
{
	L2CInvokeHeader* functions = (L2CInvokeHeader*)lua_touserdata(L,lua_upvalueindex(1));
	if (!functions)
		luaL_error(L, "Invalid variable access");
	for (int i = 0; i < functions->m_num_functions; i++)
	{
		if (functions->m_functions[i]->CheckSig(L))
		{
			return functions->m_functions[i]->Invoke(L);
		}
	}
	{
		char buff[1024]; buff[0]=0;
		int top = lua_gettop(L);
		for (int i = 1; i <= top; i++)
		{
			const char* tname = lua_typename(L,lua_type(L,i));
			strcat_s(buff,tname);
			strcat_s(buff," ");
		}
		luaL_error(L, "No overload of %s matches argument list [%s]\n",functions->m_functions[0]->m_config.m_name,buff);
	}
	return 0;
}
int l2cinternal_create_function_invoke(lua_State* L, L2CFunction* function)
{
	L2CInvokeHeader* header = (L2CInvokeHeader*)lua_newuserdata(L,sizeof(L2CInvokeHeader));
	header->m_num_functions = 1;
	header->m_functions[0] = function;
	lua_pushcclosure(L,l2cinternal_function_invoke,1);
	return 1;
}
int l2cinternal_create_function_invoke(lua_State* L, std::vector<L2CFunction*>& functions)
{
	L2CInvokeHeader* header = (L2CInvokeHeader*)lua_newuserdata(L,sizeof(L2CInvokeHeader)+sizeof(L2CFunction*)*(functions.size()-1));
	header->m_num_functions = (int32_t)functions.size();
	for (int i = 0; i < header->m_num_functions; i++)
		header->m_functions[i] = functions[i];
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

//////////////////////////////////////////////////////////////////////////
//L2C lua library / utility functions
//////////////////////////////////////////////////////////////////////////
int l2c_printstack(lua_State* L)
{
	l2c_printf("----------------------------\n");
	l2c_printf("LUA Stack:\n");
	int top = lua_gettop(L);
	for (int i = 1; i <= top; i++)
	{
		lua_pushvalue(L,i);
		const char* tname = lua_typename(L,lua_type(L,-1));
		const char* str = lua_tostring(L,-1);
		if (!str)
			str = "<unprintable>";
		l2c_printf("[%d] type=%s, value=%s\n",i,tname,str);
		lua_pop(L,1);
	}
	l2c_printf("----------------------------\n");
	return 0;
}
int l2c_printtable(lua_State* L)
{
	return l2c_printtable(L,-1);
}
int l2c_printtable(lua_State* L, int idx)
{
	int t = lua_absindex(L,idx);
	lua_pushnil(L);
	int it = 0;
	while (lua_next(L, t) != 0) 
	{
		const char* tname;
		const char* str;

		l2c_printf("%d:",it);
		it++;

		lua_pushvalue(L,-2);
		tname = lua_typename(L,lua_type(L,-2));
		str = lua_tostring(L,-1);
		if (!str)
			str = "<unprintable>";
		l2c_printf("\tKEY type=%s, value=%s\n",tname,str);
		lua_pop(L, 1);

		lua_pushvalue(L,-1);
		tname = lua_typename(L,lua_type(L,-1));
		str = lua_tostring(L,-1);
		if (!str)
			str = "<unprintable>";
		l2c_printf("\tVAL type=%s, value=%s\n",tname,str);
		lua_pop(L, 1);

		lua_pop(L, 1);
	}
	return 0;
}
int l2c_printobject(lua_State* L)
{
	return l2c_printobject(L,-1);
}
int l2c_printobject(lua_State* L, int idx)
{
	return 0;
}
