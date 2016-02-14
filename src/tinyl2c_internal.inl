

//////////////////////////////////////////////////////////////////////////
//Base classes for variable access and function invocation. This are
//overridden using template specialization in the 2 glue files below
//////////////////////////////////////////////////////////////////////////
class L2CVariable
{
public:

	struct Config
	{
		const char* m_name;

		Config(const char* name) : m_name(name) {}
	};
	Config m_config;

	L2CVariable(const Config& config) : m_config(config) {}

	virtual int Set(lua_State* L) { return 0; }
	virtual int Get(lua_State* L) { return 0; }
};
class L2CFunction
{
public:
	struct Config
	{
		const char* m_name;

		Config(const char* name) : m_name(name) {}
	};
	Config m_config;

	L2CFunction(const Config& config) : m_config(config) {}

	virtual int Invoke(lua_State* L) { return 0; }
};

//////////////////////////////////////////////////////////////////////////
//The LUA hooks that invoke a variable or function class. They expect the
//first up value of the call to be a pointer to the class, stored as
//light user data. Below are the functions used to register them with LUA
//////////////////////////////////////////////////////////////////////////
int l2cinternal_variable_get(lua_State* L);
int l2cinternal_variable_set(lua_State* L);
int l2cinternal_function_invoke(lua_State* L);
int l2cinternal_create_variable_get(lua_State* L, L2CVariable* variable);
int l2cinternal_create_variable_set(lua_State* L, L2CVariable* variable);
int l2cinternal_create_function_invoke(lua_State* L, L2CFunction* function);

//////////////////////////////////////////////////////////////////////////
//Include the overly wordy glue code for members and functions
//////////////////////////////////////////////////////////////////////////
#include "tinyl2c_variableglue.inl"
#include "tinyl2c_functionglue.inl"

//////////////////////////////////////////////////////////////////////////
//Register core numeric types
//////////////////////////////////////////////////////////////////////////
#define L2CNUMERICTYPE(ty)																			\
template<> class L2CTypeInterface<ty>																\
{																									\
public:																								\
	L2CINTERFACE_ID()																				\
	static ty			Default()						{ return 0; }								\
	static const char*	MTName()						{ return "l2c."#ty; }						\
	static void			Push(lua_State* L, ty value)	{ lua_pushnumber(L,(lua_Number)value); }	\
	static bool			Is(lua_State* L, int idx)		{ return lua_isnumber(L,idx) != 0; }		\
	static ty			To(lua_State* L, int idx)		{ return (ty)lua_tonumber(L, idx); }		\
}
L2CNUMERICTYPE(int8_t);
L2CNUMERICTYPE(uint8_t);
L2CNUMERICTYPE(int16_t);
L2CNUMERICTYPE(uint16_t);
L2CNUMERICTYPE(int32_t);
L2CNUMERICTYPE(uint32_t);
L2CNUMERICTYPE(int64_t);
L2CNUMERICTYPE(uint64_t);
L2CNUMERICTYPE(float32_t);
L2CNUMERICTYPE(float64_t);

//////////////////////////////////////////////////////////////////////////
//Register a const char* as a string type
//////////////////////////////////////////////////////////////////////////
template<> class L2CTypeInterface<const char*>																
{																									
public:																								
	L2CINTERFACE_ID()
	static const char*	Default()								{ return ""; }								
	static const char*	MTName()								{ return "l2c.charptr"; }
	static void			Push(lua_State* L, const char* value)	{ lua_pushstring(L,value); }	
	static bool			Is(lua_State* L, int idx)				{ return lua_isstring(L,idx) != 0; }		
	static const char*	To(lua_State* L, int idx)				{ return lua_tostring(L, idx); }		
};


//////////////////////////////////////////////////////////////////////////
//Header for any L2C data stored inside LUA user data
//////////////////////////////////////////////////////////////////////////
struct L2CHeader
{
	uint64_t	m_type_id;
	void*		m_data;
	bool		m_is_reference;
};

//////////////////////////////////////////////////////////////////////////
//Gets/creates the meta table for a type and pushes onto stack
//////////////////////////////////////////////////////////////////////////
template<typename ty>
inline int l2cinternal_pushmetatable(lua_State* L)
{
	//attempt to get the metatable for the type
	luaL_getmetatable(L, L2CTypeInterface<ty>::MTName());
	if (lua_isnil(L, -1))
	{
		//metatable is nil, so create it and call the register function to set it up
		lua_pop(L,1);
		luaL_newmetatable(L,L2CTypeInterface<ty>::MTName());
		L2CTypeInterface<ty>::Register(L);
	}
	return 1;
}

//////////////////////////////////////////////////////////////////////////
//Creates the user data to represent a C++ structure in LUA. This can
//either reference the C++ data externally, or copy it in
//////////////////////////////////////////////////////////////////////////
template<typename ty>
inline L2CHeader* l2cinternal_createuserdatavalue(lua_State* L, ty& data, bool is_reference)
{
	//create header
	L2CHeader* header;
	if (is_reference)
	{
		//if reference, just allocate a header and set the data ptr
		header = (L2CHeader*)lua_newuserdata(L, sizeof(L2CHeader));
		header->m_data = &data;
	}
	else
	{
		//if reference, allocate a block for header+data, 
		int headersz = L2C_ALIGNED_SIZE(sizeof(L2CHeader), 16);
		int sz = headersz + sizeof(ty) + 16;
		void* buffer = lua_newuserdata(L, sz);
		header = (L2CHeader*)buffer;

		//copy the data in using placement new and copy constructor
		ty* target = (ty*)l2c_addandalignptr(buffer,headersz, 16);
		new (target)ty(data);

		//set data ptr
		header->m_data = target;
	}

	//store type id and stuff in header
	header->m_type_id = L2CTypeInterface<ty>::Id();
	header->m_is_reference = is_reference;

	//get the metatable for this type
	l2cinternal_pushmetatable<ty>(L);

	//set metatable on user data
	lua_setmetatable(L,-2);

	//return the header
	return header;
}

//////////////////////////////////////////////////////////////////////////
//Checks/casts user data to a given c++ type
//////////////////////////////////////////////////////////////////////////
template<typename ty>
inline bool l2cinternal_isuserdatavalue(lua_State* L, int idx)
{
	return luaL_checkudata(L,idx,L2CTypeInterface<ty>::MTName()) != NULL;
}

template<typename ty>
inline ty& l2cinternal_touserdatavalue(lua_State* L, int idx)
{
	L2CHeader* header = (L2CHeader*)luaL_checkudata(L,idx,L2CTypeInterface<ty>::MTName());
	if (!header)
		luaL_error(L, "Incorrect type");
	return *(ty*)header->m_data;
}


//////////////////////////////////////////////////////////////////////////
//Fill out metatable from list of members and functions
//////////////////////////////////////////////////////////////////////////
int l2cinternal_meta_index(lua_State* L);
int l2cinternal_meta_newindex(lua_State* L);

struct L2CTypeRegistry
{
	std::vector<L2CVariable*> m_variables;
	std::vector<L2CFunction*> m_functions;
};

template<typename ty>
inline void l2cinternal_fillmetatable(lua_State* L, L2CTypeRegistry& reg)
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

	//restore stack to just the metatable
	lua_settop(L,metatable_idx);
}

//////////////////////////////////////////////////////////////////////////
//Internal macros to construct types
//////////////////////////////////////////////////////////////////////////

//Place this macros below your class or structure definition, typically in a header file
#define L2CINTERNAL_TYPE_DECLARE(_ty)																						\
template<> class L2CTypeInterface<_ty>																						\
{																															\
public:																														\
	typedef _ty ty;																											\
	L2CINTERFACE_ID()																										\
	static ty			Default()								{ return ty(); }											\
	static const char*	MTName()								{ return "l2c."#_ty; }										\
	static void			Push(lua_State* L, ty value)			{ l2cinternal_createuserdatavalue<ty>(L,value,false); }		\
	static bool			Is(lua_State* L, int idx)				{ return l2cinternal_isuserdatavalue<ty>(L,idx); }			\
	static ty			To(lua_State* L, int idx)				{ return l2cinternal_touserdatavalue<ty>(L,idx); }			\
	static void			Register(lua_State* L);																				\
};																															\
template<> class L2CTypeInterface<_ty*>																						\
{																															\
public:																														\
	typedef _ty ty;																											\
	L2CINTERFACE_ID()																										\
	static ty*			Default()								{ return NULL; }											\
	static const char*	MTName()								{ return "l2c."#_ty"*"; }									\
	static void			Push(lua_State* L, ty* value)			{ l2cinternal_createuserdatavalue<ty>(L,*value,true); }		\
	static bool			Is(lua_State* L, int idx)				{ return l2cinternal_isuserdatavalue<ty>(L,idx); }			\
	static ty*			To(lua_State* L, int idx)				{ return &l2cinternal_touserdatavalue<ty>(L,idx); }			\
};

//Start and end of the code blocks for a type definition function
#define L2CINTERNAL_TYPE_DEFINITION_BLOCK_BEGIN()		\
{														\
	L2CTypeRegistry reg;
#define L2CINTERNAL_TYPE_DEFINITION_BLOCK_END()			\
	l2cinternal_fillmetatable<ty>(L,reg);				\
}


//Defines the registration function, usually inside the c++ file for your class. 
#define L2CINTERNAL_TYPE_DEFINITION_BEGIN(_ty)		void L2CTypeInterface<_ty>::Register(lua_State* L)	L2CINTERNAL_TYPE_DEFINITION_BLOCK_BEGIN()
#define L2CINTERNAL_TYPE_DEFINITION_BEGIN_INL(_ty)	inline L2CINTERNAL_TYPE_DEFINITION_BEGIN(_ty)
#define L2CINTERNAL_TYPE_DEFINITION_END()			L2CINTERNAL_TYPE_DEFINITION_BLOCK_END()