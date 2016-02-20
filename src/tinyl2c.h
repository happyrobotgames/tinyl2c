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
	typedef ty reference_ty;
	L2CINTERFACE_ID()
	static ty			Default()							{ return ty(); }
	static const char*	MTName()							{ return "l2c.unknown"; }
	static void			PushVal(lua_State* L, ty value)		{ luaL_error(L, "Unknown type"); }
	static void			PushRef(lua_State* L, ty& value)	{ luaL_error(L, "Unknown type"); }
	static bool			Is(lua_State* L, int idx)			{ luaL_error(L, "Unknown type"); return false; }
	static ty			To(lua_State* L, int idx)			{ luaL_error(L, "Unknown type"); return Default(); }
};
#define l2c_refty(_ty) typename L2CTypeInterface<_ty>::reference_ty

//////////////////////////////////////////////////////////////////////////
//Core functions to achieve push/is/to functionality on any
//type registered with L2C
//////////////////////////////////////////////////////////////////////////

//push a copy of a value into lua
//eg l2c_pushval(L,myvalue)
template<typename ty> inline void l2c_pushval(lua_State* L, ty value)
{
	L2CTypeInterface<ty>::PushVal(L, value);
}

//push a reference to a value (just its address is stored) into lua
//eg l2c_pushval(L,myvalue)
template<typename ty> inline void l2c_pushref(lua_State* L, ty& value)
{
	L2CTypeInterface<ty>::PushRef(L, value);
}

//check if a value at a given point on the stack is of a given type
//eg if(l2c_is<MyType>(L,-1)) {}
template<typename ty> inline bool l2c_is(lua_State* L, int idx)
{
	return L2CTypeInterface<ty>::Is(L, idx);
}

//copy a value out of lua
template<typename ty> inline l2c_refty(ty) l2c_to(lua_State* L, int idx)
{
	return L2CTypeInterface<ty>::To(L, idx);
}

//////////////////////////////////////////////////////////////////////////
//Some extra utility functions
//////////////////////////////////////////////////////////////////////////

//to notify l2c about any types without registering them through pushing global variables/functions
//use l2c_addtype<type> to add them to the registry in advance. this allows their constructors to
//be called by lua code
template<typename ty> 
inline void l2c_addtype(lua_State* L) {	l2cinternal_pushmetatable<ty>(L); lua_pop(L,1); }

//helper to register a global variable
#define l2c_addglobal(_L, _variable) { l2c_pushref(_L,_variable); lua_setglobal(_L,#_variable); }

//generate the functor for a c++ function and push into lua
#define l2c_pushfunction(_L, _function) l2cinternal_create_function_invoke(L,l2cinternal_buildglobalfunc(L2CFunction::Config(#_function),&_function))

//same as above, but also registers it as a global function call
#define l2c_addglobalfunction(_L, _function) { l2cinternal_create_function_invoke(L,l2cinternal_buildglobalfunc(L2CFunction::Config(#_function),&_function)); lua_setglobal(_L,#_function); }

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

//////////////////////////////////////////////////////////////////////////
//Very rough example!
//////////////////////////////////////////////////////////////////////////
#if 0

//in header
#include "tinyl2cinclude.h"
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
#include "tinyl2c.h"
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