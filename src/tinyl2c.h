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
//L2C lua library / utility functions
//////////////////////////////////////////////////////////////////////////
int l2c_printstack(lua_State* L);
int l2c_printtable(lua_State* L);
int l2c_printtable(lua_State* L, int idx);
int l2c_printobject(lua_State* L);
int l2c_printobject(lua_State* L, int idx);

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

//place inside your class to specify you will be exposing private members to LUA
#define L2C_EXPOSE_PRIVATE(_ty)		friend class L2CTypeInterface<_ty>;

//place after your class declaration to link class to LUA
#define L2C_TYPEDECL(_ty)			L2CINTERNAL_TYPE_DECLARE(_ty)

//place inside cpp file to mark start/end of L2C definition macros (below)
#define L2C_TYPEDEF_BEGIN(_ty)		L2CINTERNAL_TYPE_DEFINITION_BEGIN(_ty)
#define L2C_TYPEDEF_END()			L2CINTERNAL_TYPE_DEFINITION_END()

//exactly the same as L2C_TYPEDEF_BEGIN, but inlined so can be used in a header file
#define L2C_TYPEDEF_BEGIN_INL(_ty)	L2CINTERNAL_TYPE_DEFINITION_BEGIN_INL(_ty)

//L2C definition macros
#define L2C_INHERITS(name)		l2cinternal_pushmetatable<name>(L);	lua_setfield(L,-2,"_l2c_inherits");
#define L2C_VARIABLE(name)		reg.m_variables.push_back(l2cinternal_buildmembervariable<ty, decltype(ty::name)>( L2CVariable::Config(#name) , offsetof(ty,name)));
#define L2C_FUNCTION(name)		reg.m_functions.push_back(l2cinternal_buildmemberfunc( L2CFunction::Config(#name) , &ty::name));
#define L2C_CONSTRUCTOR(...)	reg.m_constructors.push_back(l2cinternal_buildconstructorfunc(L2CFunction::Config("_ctor"), (ty*(*)(__VA_ARGS__))0 ));

//Definitions of operators. To keep it simple, each expects you to specify the return type and all argument types
#define L2C_OP_ADD(res_ty,a_ty,b_ty)	reg.m_add.push_back(new TL2CBinaryAdd<res_ty, a_ty, b_ty>(L2CFunction::Config("+")));
#define L2C_OP_SUB(res_ty,a_ty,b_ty)	reg.m_sub.push_back(new TL2CBinarySub<res_ty, a_ty, b_ty>(L2CFunction::Config("-")));
#define L2C_OP_MUL(res_ty,a_ty,b_ty)	reg.m_mul.push_back(new TL2CBinaryMul<res_ty, a_ty, b_ty>(L2CFunction::Config("*")));
#define L2C_OP_DIV(res_ty,a_ty,b_ty)	reg.m_div.push_back(new TL2CBinaryDiv<res_ty, a_ty, b_ty>(L2CFunction::Config("/")));
#define L2C_OP_UNM(res_ty,a_ty)			reg.m_unm.push_back(new TL2CUnaryNeg<res_ty, a_ty>(L2CFunction::Config("-")));

//Register global variables
#define L2C_ADD_GLOBAL_VARIABLE(_L,_var)		{ l2c_push(L,&_var); lua_setglobal(L,#_var); }

//Register a global function
#define L2C_ADD_GLOBAL_FUNCTION(_L,_function)	{ l2cinternal_create_function_invoke(L,l2cinternal_buildglobalfunc(L2CFunction::Config(#_function),&_function)); lua_setglobal(L,#_function); }

//to manually initialize L2C with knowledge of some types, call L2C_ADD_TYPE on each one
#define L2C_ADD_TYPE(_L,_ty)		{ l2cinternal_pushmetatable<_ty>(_L); lua_pop(_L,1); }


//////////////////////////////////////////////////////////////////////////
//Very rough example!
//////////////////////////////////////////////////////////////////////////
#if 0

//in header
struct MyStruct : public MyBaseClass
{
	MyStruct() { x=y=0;}
	MyStruct(int bla) { x=y=bla;}
	int x;
	int y;
	void Foo() { x=10; };
	MyStruct operator*(int c) { MyStruct res; res.x = x*c; res.y = y*c; return res; }
}
L2C_TYPEDECL(MyStruct)

//in cpp file
L2C_TYPEDEF_BEGIN(MyStruct)
	L2C_CONSTRUCTOR()					//default constructor
	L2C_CONSTRUCTOR(int)				//constructor that takes an int
	L2C_INHERITS(MyBaseClass)			//tell l2c who we inherit from
	L2C_VARIABLE(x)						//expose member variable
	L2C_VARIABLE(y)						//expose member variable
	L2C_FUNCTION(Foo)					//expose member function
	L2C_OP_MUL(MyStruct,MyStruct,int)	//expose multiply operator that returns MyStruct and takes MyStruct and int as parameters
L2C_TYPEDEF_END()

#endif