

//////////////////////////////////////////////////////////////////////////
//Base classes for variable access and function invocation. This are
//overridden using template specialization below
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

	virtual bool CheckSig(lua_State* L) { return false; }
	virtual int Invoke(lua_State* L) { return 0; }
};

//////////////////////////////////////////////////////////////////////////
//The LUA hooks that invoke the getter or setter for a variable via
//a variable functor. The functions expect the first up value to be
//a pointer to the c++ functor.
//////////////////////////////////////////////////////////////////////////
int l2cinternal_variable_get(lua_State* L);
int l2cinternal_variable_set(lua_State* L);
int l2cinternal_create_variable_get(lua_State* L, L2CVariable* variable);
int l2cinternal_create_variable_set(lua_State* L, L2CVariable* variable);

//////////////////////////////////////////////////////////////////////////
//Similar but a bit more advanced than variables, these invoke functions.
//Each expects a user data structure with a header followed by n functors
//as the upvalue. This allows for handling of overloaded functions with
//different signatures
//////////////////////////////////////////////////////////////////////////
int l2cinternal_function_invoke(lua_State* L);
int l2cinternal_create_function_invoke(lua_State* L, L2CFunction* function);
int l2cinternal_create_function_invoke(lua_State* L, std::vector<L2CFunction*>& functions);

//////////////////////////////////////////////////////////////////////////
//Register core numeric types
//////////////////////////////////////////////////////////////////////////
#define L2CNUMERICTYPE(ty)																				\
template<> class L2CTypeInterface<ty>																	\
{																										\
public:																									\
	typedef ty reference_ty;																			\
	L2CINTERFACE_ID()																					\
	static ty			Default()							{ return 0; }								\
	static const char*	MTName()							{ return "l2c."#ty; }						\
	static void			PushVal(lua_State* L, ty value)		{ lua_pushnumber(L,(lua_Number)value); }	\
	static void			PushRef(lua_State* L, ty& value)	{ lua_pushnumber(L,(lua_Number)value); }	\
	static bool			Is(lua_State* L, int idx)			{ return lua_isnumber(L,idx) != 0; }		\
	static ty			To(lua_State* L, int idx)			{ return (ty)lua_tonumber(L, idx); }		\
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
	typedef const char* reference_ty;
	static const char*	Default()									{ return ""; }								
	static const char*	MTName()									{ return "l2c.charptr"; }
	static void			PushVal(lua_State* L, const char* value)	{ lua_pushstring(L,value); }	
	static void			PushRef(lua_State* L, const char* value)	{ lua_pushstring(L,value); }	
	static bool			Is(lua_State* L, int idx)					{ return lua_isstring(L,idx) != 0; }		
	static const char*	To(lua_State* L, int idx)					{ return lua_tostring(L, idx); }		
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
	//create the table that will hold this user data
	lua_newtable(L);

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

	//store the user data as a member of the table
	lua_setfield(L,-2,"_l2c_data");

	//get the metatable for this type
	l2cinternal_pushmetatable<ty>(L);

	//set metatable on the table
	lua_setmetatable(L,-2);

	//return the header
	return header;
}

//////////////////////////////////////////////////////////////////////////
//Similar to above, but just allocates a buffer without filling it in
//////////////////////////////////////////////////////////////////////////
template<typename ty>
inline L2CHeader* l2cinternal_allocuserdatavalue(lua_State* L)
{
	//create the table that will hold this user data
	lua_newtable(L);

	//create header
	L2CHeader* header;

	//allocate a block for header+data, 
	int headersz = L2C_ALIGNED_SIZE(sizeof(L2CHeader), 16);
	int sz = headersz + sizeof(ty) + 16;
	void* buffer = lua_newuserdata(L, sz);
	header = (L2CHeader*)buffer;

	//set data ptr
	header->m_data = (ty*)l2c_addandalignptr(buffer,headersz, 16);

	//store type id and stuff in header
	header->m_type_id = L2CTypeInterface<ty>::Id();
	header->m_is_reference = false;

	//store the user data as a member of the table
	lua_setfield(L,-2,"_l2c_data");

	//get the metatable for this type
	l2cinternal_pushmetatable<ty>(L);

	//set metatable on the table
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
	idx = lua_absindex(L,idx);

	if (lua_istable(L,idx)) 
	{  
		luaL_getmetatable(L, L2CTypeInterface<ty>::MTName());
		if (lua_getmetatable(L, idx)) 
		{  
			while (!lua_isnil(L, -1))
			{
				if (lua_rawequal(L, -2, -1))
				{
					lua_pop(L,2);
					return true;
				}
				lua_getfield(L,-1,"_l2c_inherits");
				lua_copy(L,-1,-2);
				lua_pop(L,1);
			}
			lua_pop(L,1);
		}
		lua_pop(L,1);
	}
	return false;
}

template<typename ty>
inline L2CHeader* l2cinternal_touserdataheader(lua_State* L, int idx)
{
	idx = lua_absindex(L,idx);

	if (lua_istable(L,idx)) 
	{  
		luaL_getmetatable(L, L2CTypeInterface<ty>::MTName());
		if (lua_getmetatable(L, idx)) 
		{  
			while (!lua_isnil(L, -1))
			{
				if (lua_rawequal(L, -2, -1))
				{
					lua_getfield(L,idx,"_l2c_data");
					L2CHeader* header = (L2CHeader*)lua_touserdata(L,-1);
					lua_pop(L,3);
					return header;
				}
				lua_getfield(L,-1,"_l2c_inherits");
				lua_copy(L,-1,-2);
				lua_pop(L,1);
			}
			lua_pop(L,1);
		}
		lua_pop(L,1);
	}

	luaL_error(L, "Incorrect type");
	return NULL;
}
template<typename ty>
inline ty& l2cinternal_touserdatavalue(lua_State* L, int idx)
{
	return *(ty*)l2cinternal_touserdataheader<ty>(L,idx)->m_data;
}

//////////////////////////////////////////////////////////////////////////
//Meta table setup and callbacks
//////////////////////////////////////////////////////////////////////////
struct L2CTypeRegistry
{
	const char* m_name;
	std::vector<L2CVariable*> m_variables;
	std::vector<L2CVariable*> m_static_variables;
	std::vector<L2CFunction*> m_functions;
	std::vector<L2CFunction*> m_static_functions;
	std::vector<L2CFunction*> m_constructors;
	std::vector<L2CFunction*> m_add;
	std::vector<L2CFunction*> m_sub;
	std::vector<L2CFunction*> m_mul;
	std::vector<L2CFunction*> m_div;
	std::vector<L2CFunction*> m_unm;
	L2CFunction* m_destructor;
};
int l2cinternal_meta_index(lua_State* L);
int l2cinternal_meta_newindex(lua_State* L);
void l2cinternal_fillmetatable(lua_State* L, L2CTypeRegistry& reg);

//////////////////////////////////////////////////////////////////////////
//Include the overly wordy glue code for variables and functions
//////////////////////////////////////////////////////////////////////////
//member variable
template<typename ty, typename var_ty> class TL2CMemberVariable : public L2CVariable
{
public:
	int m_offset;

	TL2CMemberVariable(Config config, int offset) : L2CVariable(config), m_offset(offset) {}

	virtual int Set(lua_State* L)
	{
		ty* instance		= l2c_to<ty*>(L,-2);
		var_ty* member		= (var_ty*)l2c_addptr(instance,m_offset);
		*member				= l2c_to<var_ty>(L,-1);
		lua_pop(L,2);
		return 0;
	}

	virtual int Get(lua_State* L)
	{
		ty* instance		= l2c_to<ty*>(L,-1);
		var_ty* member		= (var_ty*)l2c_addptr(instance,m_offset);
		l2c_pushref(L,*member);
		if (lua_istable(L,-1))
		{
			lua_pushstring(L, "_l2c_lifetime");
			lua_pushvalue(L, -3);
			lua_rawset(L, -3);
		}
		lua_copy(L,-1,-2);
		lua_pop(L,1);
		return 1;
	}
};
template<typename ty, typename var_ty> 
inline L2CVariable* l2cinternal_buildmembervariable(L2CVariable::Config config, int offset)
{
	return new TL2CMemberVariable<ty, var_ty>(config,offset);
}

//////////////////////////////////////////////////////////////////////////
//Unary operator functions
//////////////////////////////////////////////////////////////////////////
template<typename res_ty, typename a_ty> class TL2CUnaryOperator : public L2CFunction
{
public:
	virtual res_ty Op(a_ty a) = 0;

	TL2CUnaryOperator(Config config) : L2CFunction(config) {}

	virtual bool CheckSig(lua_State* L)
	{
		return lua_gettop(L) == 1 && l2c_is<a_ty>(L,-1);
	}
	virtual int Invoke(lua_State* L)
	{
		a_ty a = l2c_to<a_ty>(L,-1);
		res_ty res = Op(a);
		lua_pop(L,2);
		l2c_pushval(L,res);
		return 1;
	}
};
template<typename res_ty, typename a_ty> class TL2CUnaryNeg : public TL2CUnaryOperator<res_ty, a_ty>
{
public:
	TL2CUnaryNeg(Config config) : TL2CUnaryOperator<res_ty, a_ty>(config) {}
	virtual res_ty Op(a_ty a) { return -a; }
};

//////////////////////////////////////////////////////////////////////////
//Binary operator functions
//////////////////////////////////////////////////////////////////////////
template<typename res_ty, typename a_ty, typename b_ty> class TL2CBinaryOperator : public L2CFunction
{
public:
	virtual res_ty Op(a_ty a, b_ty b) = 0;

	TL2CBinaryOperator(Config config) : L2CFunction(config) {}

	virtual bool CheckSig(lua_State* L)
	{
		return lua_gettop(L) == 2 && l2c_is<a_ty>(L,-2) && l2c_is<b_ty>(L,-1);
	}
	virtual int Invoke(lua_State* L)
	{
		a_ty a = l2c_to<a_ty>(L,-2);
		b_ty b = l2c_to<b_ty>(L,-1);
		res_ty res = Op(a,b);
		lua_pop(L,2);
		l2c_pushval(L,res);
		return 1;
	}
};
template<typename res_ty, typename a_ty, typename b_ty> class TL2CBinaryAdd : public TL2CBinaryOperator<res_ty, a_ty, b_ty>
{
public:
	TL2CBinaryAdd(Config config) : TL2CBinaryOperator<res_ty, a_ty, b_ty>(config) {}
	virtual res_ty Op(a_ty a, b_ty b) { return a + b; }
};
template<typename res_ty, typename a_ty, typename b_ty> class TL2CBinarySub : public TL2CBinaryOperator<res_ty, a_ty, b_ty>
{
public:
	TL2CBinarySub(Config config) : TL2CBinaryOperator<res_ty, a_ty, b_ty>(config) {}
	virtual res_ty Op(a_ty a, b_ty b) { return a - b; }
};
template<typename res_ty, typename a_ty, typename b_ty> class TL2CBinaryMul : public TL2CBinaryOperator<res_ty, a_ty, b_ty>
{
public:
	TL2CBinaryMul(Config config) : TL2CBinaryOperator<res_ty, a_ty, b_ty>(config) {}
	virtual res_ty Op(a_ty a, b_ty b) { return a * b; }
};
template<typename res_ty, typename a_ty, typename b_ty> class TL2CBinaryDiv : public TL2CBinaryOperator<res_ty, a_ty, b_ty>
{
public:
	TL2CBinaryDiv(Config config) : TL2CBinaryOperator<res_ty, a_ty, b_ty>(config) {}
	virtual res_ty Op(a_ty a, b_ty b) { return a / b; }
};

//////////////////////////////////////////////////////////////////////////
//Use the function glue generator to build the different functors
//these build member, global and constructor functions for between 0 and 15 arguments
//////////////////////////////////////////////////////////////////////////
#define FDEF_ARG_COUNT 0
#include "tinyl2c_functionglue_preproc.inl"
#define FDEF_ARG_COUNT 1
#include "tinyl2c_functionglue_preproc.inl"
#define FDEF_ARG_COUNT 2
#include "tinyl2c_functionglue_preproc.inl"
#define FDEF_ARG_COUNT 3
#include "tinyl2c_functionglue_preproc.inl"
#define FDEF_ARG_COUNT 4
#include "tinyl2c_functionglue_preproc.inl"
#define FDEF_ARG_COUNT 5
#include "tinyl2c_functionglue_preproc.inl"
#define FDEF_ARG_COUNT 6
#include "tinyl2c_functionglue_preproc.inl"
#define FDEF_ARG_COUNT 7
#include "tinyl2c_functionglue_preproc.inl"
#define FDEF_ARG_COUNT 8
#include "tinyl2c_functionglue_preproc.inl"
#define FDEF_ARG_COUNT 9
#include "tinyl2c_functionglue_preproc.inl"
#define FDEF_ARG_COUNT 10
#include "tinyl2c_functionglue_preproc.inl"
#define FDEF_ARG_COUNT 11
#include "tinyl2c_functionglue_preproc.inl"
#define FDEF_ARG_COUNT 12
#include "tinyl2c_functionglue_preproc.inl"
#define FDEF_ARG_COUNT 13
#include "tinyl2c_functionglue_preproc.inl"
#define FDEF_ARG_COUNT 14
#include "tinyl2c_functionglue_preproc.inl"
#define FDEF_ARG_COUNT 15
#include "tinyl2c_functionglue_preproc.inl"

//And an extra bit of glue to define a destructor function
template<typename ty> class TL2CDestructor : public L2CFunction
{
public:
	TL2CDestructor(Config config) : L2CFunction(config) {}
	virtual bool CheckSig(lua_State* L)
	{
		return lua_gettop(L) == 1 && l2c_is<ty*>(L,-1);
	}
	virtual int Invoke(lua_State* L)
	{
		L2CHeader* header = l2cinternal_touserdataheader<ty>(L,-1);
		if (!header->m_is_reference)
		{
			ty& instance = *(ty*)header->m_data;
			instance.~ty();
		}
		lua_pop(L,1);
		return 0;
	}
};
template<typename ty> 
inline L2CFunction* l2cinternal_builddestructor(L2CFunction::Config config)
{
	return new TL2CDestructor<ty>(config);
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
	typedef ty& reference_ty;																								\
	L2CINTERFACE_ID()																										\
	static ty			Default()								{ return ty(); }											\
	static const char*	Name()									{ return #_ty; }											\
	static const char*	MTName()								{ return "l2c."#_ty; }										\
	static void			PushVal(lua_State* L, ty value)			{ l2cinternal_createuserdatavalue<ty>(L,value,false); }		\
	static void			PushRef(lua_State* L, ty& value)		{ l2cinternal_createuserdatavalue<ty>(L,value,true); }		\
	static bool			Is(lua_State* L, int idx)				{ return l2cinternal_isuserdatavalue<ty>(L,idx); }			\
	static ty			To(lua_State* L, int idx)				{ return l2cinternal_touserdatavalue<ty>(L,idx); }			\
	static void			Register(lua_State* L);																				\
};																															\
template<> class L2CTypeInterface<_ty*>																						\
{																															\
public:																														\
	typedef _ty ty;																											\
	typedef ty* reference_ty;																								\
	L2CINTERFACE_ID()																										\
	static ty*			Default()								{ return NULL; }											\
	static const char*	Name()									{ return #_ty"*"; }											\
	static const char*	MTName()								{ return "l2c."#_ty"*"; }									\
	static void			PushVal(lua_State* L, ty* value)		{ l2cinternal_createuserdatavalue<ty>(L,*value,true); }		\
	static void			PushRef(lua_State* L, ty* value)		{ l2cinternal_createuserdatavalue<ty>(L,*value,true); }		\
	static bool			Is(lua_State* L, int idx)				{ return l2cinternal_isuserdatavalue<ty>(L,idx); }			\
	static ty*			To(lua_State* L, int idx)				{ return &l2cinternal_touserdatavalue<ty>(L,idx); }			\
};

//Start and end of the code blocks for a type definition function
#define L2CINTERNAL_TYPE_DEFINITION_BLOCK_BEGIN()		\
{														\
	L2CTypeRegistry reg;

#define L2CINTERNAL_TYPE_DEFINITION_BLOCK_END()											\
	reg.m_destructor = l2cinternal_builddestructor<ty>(L2CFunction::Config("_dtor"));	\
	reg.m_name = L2CTypeInterface<ty>::Name();											\
	l2cinternal_fillmetatable(L,reg);													\
	if (reg.m_constructors.size())														\
	{																					\
		l2cinternal_create_function_invoke(L, reg.m_constructors);						\
		lua_setglobal(L, L2CTypeInterface<ty>::Name());									\
	}																					\
}


//Defines the registration function, usually inside the c++ file for your class. 
#define L2CINTERNAL_TYPE_DEFINITION_BEGIN(_ty)		void L2CTypeInterface<_ty>::Register(lua_State* L)	L2CINTERNAL_TYPE_DEFINITION_BLOCK_BEGIN()
#define L2CINTERNAL_TYPE_DEFINITION_BEGIN_INL(_ty)	inline L2CINTERNAL_TYPE_DEFINITION_BEGIN(_ty)
#define L2CINTERNAL_TYPE_DEFINITION_END()			L2CINTERNAL_TYPE_DEFINITION_BLOCK_END()


