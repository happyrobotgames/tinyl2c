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

void l2cinternal_fillmetatable(lua_State* L, L2CTypeRegistry& reg)
{
	 //get metatable index (assumed on top of stack)
	int metatable_idx = lua_gettop(L);

	//load inheritance pointer idx
	lua_getfield(L,metatable_idx,"_l2c_inherits");
	int inherits_idx = lua_gettop(L);

	//create a table for members and a table for functions
	lua_newtable(L);
	int gettertable_idx = lua_gettop(L);
	lua_newtable(L);
	int settertable_idx = lua_gettop(L);
	lua_newtable(L);
	int functiontable_idx = lua_gettop(L);

	//fill them out
	for (int i = 0; i < reg.m_variables.size(); i++)
	{
		l2cinternal_create_variable_get(L,reg.m_variables[i]);
		lua_setfield(L,gettertable_idx,reg.m_variables[i]->m_config.m_name);
		l2cinternal_create_variable_set(L,reg.m_variables[i]);
		lua_setfield(L,settertable_idx,reg.m_variables[i]->m_config.m_name);
	}
	for (int i = 0; i < reg.m_functions.size(); i++)
	{
		l2cinternal_create_function_invoke(L,reg.m_functions[i]);
		lua_setfield(L,functiontable_idx,reg.m_functions[i]->m_config.m_name);
	}

	//store the tables in the metatable for safe keeping
	lua_pushvalue(L, gettertable_idx);
	lua_setfield(L, metatable_idx, "_l2c_getters");
	lua_pushvalue(L, settertable_idx);
	lua_setfield(L, metatable_idx, "_l2c_setters");
	lua_pushvalue(L, functiontable_idx);
	lua_setfield(L, metatable_idx, "_l2c_functions");

	//set index function
	if (!lua_isnil(L,inherits_idx))	lua_getfield(L,inherits_idx, "__index"); //the index function from the ancestor's metatable
	else lua_pushnil(L);
	lua_pushvalue(L, gettertable_idx);
	lua_pushvalue(L, functiontable_idx);
	lua_pushcclosure(L, l2cinternal_meta_index, 3);
	lua_setfield(L,metatable_idx,"__index");

	//set new index function
	if (!lua_isnil(L,inherits_idx))	lua_getfield(L,inherits_idx, "__newindex"); //the new index function from the ancestor's metatable
	else lua_pushnil(L);
	lua_pushvalue(L, settertable_idx);
	lua_pushvalue(L, functiontable_idx);
	lua_pushcclosure(L, l2cinternal_meta_newindex, 3);
	lua_setfield(L,metatable_idx,"__newindex");

	//set garbage collection function
	l2cinternal_create_function_invoke(L,reg.m_destructor);
	lua_setfield(L,metatable_idx,"__gc");

	//operators
	if (reg.m_add.size())
	{
		l2cinternal_create_function_invoke(L,reg.m_add);
		lua_setfield(L,metatable_idx,"__add");
	}
	if (reg.m_sub.size())
	{
		l2cinternal_create_function_invoke(L,reg.m_sub);
		lua_setfield(L,metatable_idx,"__sub");
	}
	if (reg.m_mul.size())
	{
		l2cinternal_create_function_invoke(L,reg.m_mul);
		lua_setfield(L,metatable_idx,"__mul");
	}
	if (reg.m_div.size())
	{
		l2cinternal_create_function_invoke(L,reg.m_div);
		lua_setfield(L,metatable_idx,"__div");
	}
	if (reg.m_unm.size())
	{
		l2cinternal_create_function_invoke(L,reg.m_unm);
		lua_setfield(L,metatable_idx,"__unm");
	}

	//restore stack to just the metatable
	lua_settop(L,metatable_idx);
}

//////////////////////////////////////////////////////////////////////////
//L2C lua library / utility functions
//////////////////////////////////////////////////////////////////////////
int l2c_printstack(lua_State* L)
{
	l2c_printf("----------------------------\n");
	l2c_printf("LUA Stack:\n");
	int top = lua_gettop(L);
	lua_getglobal(L, "tostring");
	for (int i = 1; i <= top; i++)
	{
	    lua_pushvalue(L, -1);	//tostring
		lua_pushvalue(L,i);		//stack[i]
	    lua_call(L, 1, 1);		//convert to string
		const char* str = lua_tostring(L,-1);
		if (!str) str = "<unprintable>";
		l2c_printf("[%d] %s\n",i,str);
		lua_pop(L,1);			//remove temp
	}
	lua_pop(L,1); //remove tostring
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
	lua_getglobal(L, "tostring");
	int tosidx = lua_gettop(L);

	lua_pushnil(L);
	int it = 0;
	while (lua_next(L, t) != 0) 
	{
		const char* str;

		l2c_printf("%d:",it);
		it++;

		lua_pushvalue(L,tosidx);
		lua_pushvalue(L,-3);
	    lua_call(L, 1, 1);		//convert to string
		str = lua_tostring(L,-1);
		if (!str)
			str = "<unprintable>";
		l2c_printf("\tKEY %s\n",str);
		lua_pop(L, 1);

		lua_pushvalue(L,tosidx);
		lua_pushvalue(L,-2);
	    lua_call(L, 1, 1);		//convert to string
		str = lua_tostring(L,-1);
		if (!str)
			str = "<unprintable>";
		l2c_printf("\tVAL %s\n",str);
		lua_pop(L, 1);

		lua_pop(L, 1);
	}
	lua_pop(L, 1);

	return 0;
}
int l2c_printobject(lua_State* L)
{
	return l2c_printobject(L,-1);
}

//expects:
//-2: user data object
//-1: metatable to use
int l2c_printobjectwithmetatable(lua_State* L)
{
	int object_idx = lua_absindex(L,-2);
	int metatable_idx = lua_absindex(L,-1);

	lua_getglobal(L, "tostring");
	int tostring_idx = lua_gettop(L);

	lua_getfield(L, metatable_idx, "_l2c_inherits");
	if (!lua_isnil(L, -1))
	{
		lua_pushvalue(L,object_idx);
		lua_pushvalue(L,-2);
		l2c_printobjectwithmetatable(L);
		lua_pop(L,2);
	}
	lua_pop(L,1);

	lua_getfield(L, metatable_idx, "_l2c_getters");
	int getters_idx = lua_absindex(L,-1);

	lua_pushnil(L);
	while (lua_next(L, getters_idx) != 0)
	{
		const char* name = lua_tostring(L,-2);
		lua_pushvalue(L,object_idx);
		lua_call(L,1,1); //call getter
		lua_pushvalue(L,tostring_idx);
		lua_pushvalue(L,-2);
		lua_call(L,1,1); //call tostring
		const char* val = lua_tostring(L,-1);
		
		l2c_printf("%s=%s\n",name,val);
		lua_pop(L,2);
	}

	lua_pop(L,2);

	return 0;
}

int l2c_printobject(lua_State* L, int idx)
{
	int top = lua_gettop(L);

	int object_idx = lua_absindex(L,idx);
	if(lua_getmetatable(L,object_idx) == 0)
		return luaL_error(L, "Not an object");

	lua_pushvalue(L,object_idx);
	lua_pushvalue(L,-2);
	l2c_printobjectwithmetatable(L);

	lua_pop(L,3);
	return 0;
}
