#pragma once

//////////////////////////////////////////////////////////////////////////
//Standard includes
//////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include <stdint.h>
#include <vector>

//////////////////////////////////////////////////////////////////////////
//THIS SHOULD POINT TO YOUR LUA INCLUDE
//////////////////////////////////////////////////////////////////////////
#include "../lua-5.3.2/src/lua.hpp"

//////////////////////////////////////////////////////////////////////////
//Utility functions / macros
//////////////////////////////////////////////////////////////////////////
#define L2C_ALIGNED_SIZE(size, alignment) (((size) + ((alignment)-1)) & ~((alignment)-1))

template<typename ty>
inline ty* l2c_addptr(ty* src, intptr_t amount) 
{ 
	return reinterpret_cast<ty*>(reinterpret_cast<intptr_t>(src)+amount); 
}
template<typename ty>
inline ty* l2c_addandalignptr(ty* src, intptr_t amount, intptr_t alignment) 
{ 
	intptr_t unal = reinterpret_cast<intptr_t>(src)+amount;
	return reinterpret_cast<ty*>(L2C_ALIGNED_SIZE(unal,alignment)); 
}
void l2c_printf(const char * format, ...);

//////////////////////////////////////////////////////////////////////////
//type definitions 
//////////////////////////////////////////////////////////////////////////
typedef float float32_t;
typedef double float64_t;

//////////////////////////////////////////////////////////////////////////
//Base template that describes a type to L2C. Types compatible with
//L2C will define their own specializations of this
//////////////////////////////////////////////////////////////////////////
#define L2CINTERFACE_ID() static uint64_t Id() { static int idloc = 0; return (uint64_t)&idloc; }
template<typename ty> class L2CTypeInterface
{
public:
	L2CINTERFACE_ID()
	static ty			Default()						{ return ty(); }
	static const char*	MTName()						{ return "l2c.unknown"; }
	static void			Push(lua_State* L, ty value)	{ luaL_error(L, "Unknown type"); }
	static bool			Is(lua_State* L, int idx)		{ luaL_error(L, "Unknown type"); return false; }
	static ty			To(lua_State* L, int idx)		{ luaL_error(L, "Unknown type"); return Default(); }
};

//////////////////////////////////////////////////////////////////////////
//Core functions to achieve push/is/to functionality on any
//type registered with L2C
//////////////////////////////////////////////////////////////////////////
template<typename ty> inline void l2c_push(lua_State* L, ty value)
{
	L2CTypeInterface<ty>::Push(L, value);
}
template<typename ty> inline bool l2c_is(lua_State* L, int idx)
{
	return L2CTypeInterface<ty>::Is(L, idx);
}
template<typename ty> inline ty l2c_to(lua_State* L, int idx)
{
	return L2CTypeInterface<ty>::To(L, idx);
}

//////////////////////////////////////////////////////////////////////////
//Pull in the internal systems
//////////////////////////////////////////////////////////////////////////
#include "tinyl2c_internal.inl"

//////////////////////////////////////////////////////////////////////////
//Type definition macros
//////////////////////////////////////////////////////////////////////////

//place after your class declaration to link class to LUA
#define L2C_TYPEDECL(_ty)			L2CINTERNAL_TYPE_DECLARE(_ty)

//place inside cpp file to mark start/end of L2C definition macros (below)
#define L2C_TYPEDEF_BEGIN(_ty)		L2CINTERNAL_TYPE_DEFINITION_BEGIN(_ty)
#define L2C_TYPEDEF_END()			L2CINTERNAL_TYPE_DEFINITION_END()

//exactly the same as L2C_TYPEDEF_BEGIN, but inlined so can be used in a header file
#define L2C_TYPEDEF_BEGIN_INL(_ty)	L2CINTERNAL_TYPE_DEFINITION_BEGIN_INL(_ty)

//L2C definition macros
#define L2C_INHERITS(name)		l2cinternal_pushmetatable<name>(L);	lua_setfield(L,-2,"_l2c_inherits");
#define L2C_VARIABLE(name)		reg.m_variables.push_back(l2cinternal_buildvariable<ty, int>( L2CVariable::Config(#name) , offsetof(ty,name)));
#define L2C_FUNCTION(name)		reg.m_functions.push_back(l2cinternal_buildfunc( L2CFunction::Config(#name) , &ty::name));


//////////////////////////////////////////////////////////////////////////
//Very rough example!
//////////////////////////////////////////////////////////////////////////
#if 0

//in header
struct MyStruct
{
	int x;
	int y;
	void Foo() { x=10; };
}
L2C_TYPEDECL(MyStruct)

//in cpp file
L2C_TYPEDEF_BEGIN(MyStruct)
	L2C_VARIABLE(x)
	L2C_VARIABLE(y)
	L2C_FUNCTION(Foo)
L2C_TYPEDEF_END()

#endif