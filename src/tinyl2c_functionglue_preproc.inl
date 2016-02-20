//////////////////////////////////////////////////////////////////////////
//This is horrific preprocessor magic for which I am sorry, but there's
//just no nice way of generically calling functions with different 
//argument counts/types and return types in c++.
//this file is recursive, in that it is included once with an 'arg count',
//which internally sets up a load of other defines then re-includes itself
//////////////////////////////////////////////////////////////////////////

//check if this is a 'recursive' include (i.e. the file including itself)
#ifndef _FDEF_RECURSING

	//its not, so verify we have an argument count
	#ifndef FDEF_ARG_COUNT
	#error "You must specify an arg count"
	#endif

	//set the define that says we're recursing
	#define _FDEF_RECURSING

	//define a few constants
	#define FTYPE_MEMBER 1
	#define FTYPE_GLOBAL 2
	#define FTYPE_CONSTRUCTOR 3
	#define FDEF_BASE_CLASS L2CFunction
	#define FDEF_TYPE 

	//specify 'generate member functions', then recurse
	#undef FDEF_TYPE
	#define FDEF_TYPE FTYPE_MEMBER
	#include "tinyl2c_functionglue_preproc.inl"

	//specify 'generate global functions', then recurse
	#undef FDEF_TYPE
	#define FDEF_TYPE FTYPE_GLOBAL
	#include "tinyl2c_functionglue_preproc.inl"

	//enable return values (earlier ones returned void)
	#define FDEF_RETURN

	//specify 'generate member functions', then recurse
	#undef FDEF_TYPE
	#define FDEF_TYPE FTYPE_MEMBER
	#include "tinyl2c_functionglue_preproc.inl"

	//specify 'generate global functions', then recurse
	#undef FDEF_TYPE
	#define FDEF_TYPE FTYPE_GLOBAL
	#include "tinyl2c_functionglue_preproc.inl"

	//specify 'generate constructors', then recurse
	#undef FDEF_TYPE
	#define FDEF_TYPE FTYPE_CONSTRUCTOR
	#include "tinyl2c_functionglue_preproc.inl"

	//clear out preprocessor
	#undef FDEF_RETURN
	#undef FTYPE_MEMBER
	#undef FTYPE_GLOBAL
	#undef FTYPE_CONSTRUCTOR
	#undef FDEF_BASE_CLASS
	#undef FDEF_TYPE
	#undef FDEF_ARG_COUNT
	#undef _FDEF_RECURSING

#else

//This is the main preprocessor magic
//It works primarily by defining VA ARG macros that conditionally output their arguments. Using this, with a bit of extra
//preprocessor logic, we can generate all the different possible combinations of function for different argument sets

//Some conditionals, and a few defines based off whether we return a value
#ifdef FDEF_RETURN
	#define IF_RETURN(...) __VA_ARGS__
	#define IF_VOID(...)
	#define RET_NAME Ret
	#define RET_TYPE ret_ty
#else
	#define IF_RETURN(...)
	#define IF_VOID(...) __VA_ARGS__
	#define RET_NAME Void
	#define RET_TYPE void
#endif

//Big list of conditionals based on how many arguments have been provided.
//eg 'IF_ARG3(bla)' will output 'bla' if arg count is >= 3 
#if FDEF_ARG_COUNT > 0
	#define IF_ARG0(...) __VA_ARGS__
#else
	#define IF_ARG0(...)
#endif
#if FDEF_ARG_COUNT > 1
	#define IF_ARG1(...) __VA_ARGS__
#else
	#define IF_ARG1(...)
#endif
#if FDEF_ARG_COUNT > 2
	#define IF_ARG2(...) __VA_ARGS__
#else
	#define IF_ARG2(...)
#endif
#if FDEF_ARG_COUNT > 3
	#define IF_ARG3(...) __VA_ARGS__
#else
	#define IF_ARG3(...)
#endif
#if FDEF_ARG_COUNT > 4
	#define IF_ARG4(...) __VA_ARGS__
#else
	#define IF_ARG4(...)
#endif
#if FDEF_ARG_COUNT > 5
	#define IF_ARG5(...) __VA_ARGS__
#else
	#define IF_ARG5(...)
#endif
#if FDEF_ARG_COUNT > 6
	#define IF_ARG6(...) __VA_ARGS__
#else
	#define IF_ARG6(...)
#endif
#if FDEF_ARG_COUNT > 7
	#define IF_ARG7(...) __VA_ARGS__
#else
	#define IF_ARG7(...)
#endif
#if FDEF_ARG_COUNT > 8
	#define IF_ARG8(...) __VA_ARGS__
#else
	#define IF_ARG8(...)
#endif
#if FDEF_ARG_COUNT > 9
	#define IF_ARG9(...) __VA_ARGS__
#else
	#define IF_ARG9(...)
#endif
#if FDEF_ARG_COUNT > 10
	#define IF_ARGA(...) __VA_ARGS__
#else
	#define IF_ARGA(...)
#endif
#if FDEF_ARG_COUNT > 11
	#define IF_ARGB(...) __VA_ARGS__
#else
	#define IF_ARGB(...)
#endif
#if FDEF_ARG_COUNT > 12
	#define IF_ARGC(...) __VA_ARGS__
#else
	#define IF_ARGC(...)
#endif
#if FDEF_ARG_COUNT > 13
	#define IF_ARGD(...) __VA_ARGS__
#else
	#define IF_ARGD(...)
#endif
#if FDEF_ARG_COUNT > 14
	#define IF_ARGE(...) __VA_ARGS__
#else
	#define IF_ARGE(...)
#endif
#if FDEF_ARG_COUNT > 15
	#define IF_ARGF(...) __VA_ARGS__
#else
	#define IF_ARGF(...)
#endif

//generate the name of the functor class. this is actually just combining a name with some of the
//arguments, but has to use the 2 step preprocessor trick to force evaluation of the arguments
//before using them to generate a name
#if FDEF_TYPE == FTYPE_MEMBER
	#define _FUNCTOR_NAME_FORMAT(_nm,_args) TL2CMemberFunc_##_nm##_##_args
#elif FDEF_TYPE == FTYPE_GLOBAL
	#define _FUNCTOR_NAME_FORMAT(_nm,_args) TL2CGlobalFunc_##_nm##_##_args
#elif FDEF_TYPE == FTYPE_CONSTRUCTOR
	#define _FUNCTOR_NAME_FORMAT(_nm,_args) TL2CConstructorFunc_##_args
#endif
#define FUNCTOR_NAME_FORMAT(_nm,_args) _FUNCTOR_NAME_FORMAT(_nm,_args)
#define FUNCTOR_NAME FUNCTOR_NAME_FORMAT(RET_NAME,FDEF_ARG_COUNT)

//pick the name of the build function
#if FDEF_TYPE == FTYPE_MEMBER
	#define FUNC_NAME l2cinternal_buildmemberfunc
#elif FDEF_TYPE == FTYPE_GLOBAL
	#define FUNC_NAME l2cinternal_buildglobalfunc
#elif FDEF_TYPE == FTYPE_CONSTRUCTOR
	#define FUNC_NAME l2cinternal_buildconstructorfunc
#endif

//more handy conditionals based off function type
#if FDEF_TYPE == FTYPE_CONSTRUCTOR || FDEF_TYPE == FTYPE_MEMBER
	#define IF_CONSTRUCTOR_OR_MEMBER(...) __VA_ARGS__
#else
	#define IF_CONSTRUCTOR_OR_MEMBER(...)
#endif
#if FDEF_TYPE == FTYPE_GLOBAL || FDEF_TYPE == FTYPE_MEMBER
	#define IF_GLOBAL_OR_MEMBER(...) __VA_ARGS__
	#define IF_RETURNING_GLOBAL_OR_MEMBER(...) IF_RETURN(__VA_ARGS__)
#else
	#define IF_GLOBAL_OR_MEMBER(...)
	#define IF_RETURNING_GLOBAL_OR_MEMBER(...)
#endif

//the template definition, which includes the parameters:
//	ty, the class type, if a constructor or member
//	ret_ty, the return type if a global or member that returns a value
//	argx_ty, the argument type for each argument
//	a dummy value on the end (just there to stop syntax errors when ending up with 0 arg template)
#define FUNC_TEMPLATE									\
template<												\
	IF_CONSTRUCTOR_OR_MEMBER(typename ty,)				\
	IF_RETURNING_GLOBAL_OR_MEMBER(typename ret_ty,)		\
	IF_ARG0(typename arg0_ty,)							\
	IF_ARG1(typename arg1_ty,)							\
	IF_ARG2(typename arg2_ty,)							\
	IF_ARG3(typename arg3_ty,)							\
	IF_ARG4(typename arg4_ty,)							\
	IF_ARG5(typename arg5_ty,)							\
	IF_ARG6(typename arg6_ty,)							\
	IF_ARG7(typename arg7_ty,)							\
	IF_ARG8(typename arg8_ty,)							\
	IF_ARG9(typename arg9_ty,)							\
	IF_ARGA(typename argA_ty,)							\
	IF_ARGB(typename argB_ty,)							\
	IF_ARGC(typename argC_ty,)							\
	IF_ARGD(typename argD_ty,)							\
	IF_ARGE(typename argE_ty,)							\
	IF_ARGF(typename argF_ty,)							\
	int dummy=0											\
> 

//the prefix of the actual function type definition:
// global: int(*_nm)
// member: int(ty::*_nm)
// constructor: ty*(*_nm)
//and similarly, the prefix for the function call
// global: (*m_func)
// member: (instance.*m_func)
// constructor: new ty
#if FDEF_TYPE == FTYPE_GLOBAL
	#define FUNC_TYPEDEF_PREFIX(_nm)	RET_TYPE (*_nm)
	#define FUNC_CALL_PREFIX			(*m_func)
#elif FDEF_TYPE == FTYPE_MEMBER
	#define FUNC_TYPEDEF_PREFIX(_nm)	RET_TYPE (ty::*_nm)
	#define FUNC_CALL_PREFIX			(instance.*m_func)
#elif FDEF_TYPE == FTYPE_CONSTRUCTOR
	#define FUNC_TYPEDEF_PREFIX(_nm)	ty* (*_nm)
	#define FUNC_CALL_PREFIX			new (l2cinternal_allocuserdatavalue<ty>(L)->m_data) ty
#endif

//the argument list of the function type definition
#define FUNC_TYPEDEF_ARGS	\
	IF_ARG0( arg0_ty)		\
	IF_ARG1(,arg1_ty)		\
	IF_ARG2(,arg2_ty)		\
	IF_ARG3(,arg3_ty)		\
	IF_ARG4(,arg4_ty)		\
	IF_ARG5(,arg5_ty)		\
	IF_ARG6(,arg6_ty)		\
	IF_ARG7(,arg7_ty)		\
	IF_ARG8(,arg8_ty)		\
	IF_ARG9(,arg9_ty)		\
	IF_ARGA(,argA_ty)		\
	IF_ARGB(,argB_ty)		\
	IF_ARGC(,argC_ty)		\
	IF_ARGD(,argD_ty)		\
	IF_ARGE(,argE_ty)		\
	IF_ARGF(,argF_ty)

//the full function type definition
#define FUNC_TYPEDEF(_nm) FUNC_TYPEDEF_PREFIX(_nm)(FUNC_TYPEDEF_ARGS)

//the argument list for the function call
#define FUNC_CALL_ARGS							\
	IF_ARG0( l2c_to<arg0_ty>(L,arg_idx+0x0))	\
	IF_ARG1(,l2c_to<arg1_ty>(L,arg_idx+0x1))	\
	IF_ARG2(,l2c_to<arg2_ty>(L,arg_idx+0x2))	\
	IF_ARG3(,l2c_to<arg3_ty>(L,arg_idx+0x3))	\
	IF_ARG4(,l2c_to<arg4_ty>(L,arg_idx+0x4))	\
	IF_ARG5(,l2c_to<arg5_ty>(L,arg_idx+0x5))	\
	IF_ARG6(,l2c_to<arg6_ty>(L,arg_idx+0x6))	\
	IF_ARG7(,l2c_to<arg7_ty>(L,arg_idx+0x7))	\
	IF_ARG8(,l2c_to<arg8_ty>(L,arg_idx+0x8))	\
	IF_ARG9(,l2c_to<arg9_ty>(L,arg_idx+0x9))	\
	IF_ARGA(,l2c_to<argA_ty>(L,arg_idx+0xA))	\
	IF_ARGB(,l2c_to<argB_ty>(L,arg_idx+0xB))	\
	IF_ARGC(,l2c_to<argC_ty>(L,arg_idx+0xC))	\
	IF_ARGD(,l2c_to<argD_ty>(L,arg_idx+0xD))	\
	IF_ARGE(,l2c_to<argE_ty>(L,arg_idx+0xE))	\
	IF_ARGF(,l2c_to<argF_ty>(L,arg_idx+0xF))


//the expected number of arguments on the LUA stack (expects 1 extra if a member function)
#if FDEF_TYPE == FTYPE_MEMBER
	#define FUNC_EXPECTED_ARGC (FDEF_ARG_COUNT+1)
#else
	#define FUNC_EXPECTED_ARGC (FDEF_ARG_COUNT)
#endif

//////////////////////////////////////////////////////////////////////////
//The functor class
//////////////////////////////////////////////////////////////////////////
FUNC_TEMPLATE
class FUNCTOR_NAME : public FDEF_BASE_CLASS
{
public:

	//constructor and members
	#if FDEF_TYPE == FTYPE_GLOBAL || FDEF_TYPE == FTYPE_MEMBER 
		//globals/members define and store the function pointer to be called
		typedef FUNC_TYPEDEF(func_ptr);
		func_ptr m_func;
		FUNCTOR_NAME(Config config, func_ptr func) : FDEF_BASE_CLASS(config), m_func(func) {}
	#else
		//constructors don't need a function pointer (they use 'new'), but do define the ret_ty as the same as ty
		typedef ty ret_ty;
		FUNCTOR_NAME(Config config) : FDEF_BASE_CLASS(config) {}
	#endif

	//checks the arguments on the LUA stack to see if they match the function definition
	virtual bool CheckSig(lua_State* L)
	{
		if (lua_gettop(L) != FUNC_EXPECTED_ARGC)
			return false;

		//get base argument index
		int base_idx			= lua_absindex(L,-1-FUNC_EXPECTED_ARGC+1);
		int arg_idx				= base_idx;

		//check if arguments match (member has 1 extra which should be a ty*)
		bool match				= true;
		#if FDEF_TYPE == FTYPE_MEMBER
		match					= match && l2c_is<ty*>(L,arg_idx);
		arg_idx++;
		#endif
		IF_ARG0(match = match && l2c_is<arg0_ty>(L,arg_idx+0x0));
		IF_ARG1(match = match && l2c_is<arg1_ty>(L,arg_idx+0x1));
		IF_ARG2(match = match && l2c_is<arg2_ty>(L,arg_idx+0x2));
		IF_ARG3(match = match && l2c_is<arg3_ty>(L,arg_idx+0x3));
		IF_ARG4(match = match && l2c_is<arg4_ty>(L,arg_idx+0x4));
		IF_ARG5(match = match && l2c_is<arg5_ty>(L,arg_idx+0x5));
		IF_ARG6(match = match && l2c_is<arg6_ty>(L,arg_idx+0x6));
		IF_ARG7(match = match && l2c_is<arg7_ty>(L,arg_idx+0x7));
		IF_ARG8(match = match && l2c_is<arg8_ty>(L,arg_idx+0x8));
		IF_ARG9(match = match && l2c_is<arg9_ty>(L,arg_idx+0x9));
		IF_ARGA(match = match && l2c_is<argA_ty>(L,arg_idx+0xA));
		IF_ARGB(match = match && l2c_is<argB_ty>(L,arg_idx+0xB));
		IF_ARGC(match = match && l2c_is<argC_ty>(L,arg_idx+0xC));
		IF_ARGD(match = match && l2c_is<argD_ty>(L,arg_idx+0xD));
		IF_ARGE(match = match && l2c_is<argE_ty>(L,arg_idx+0xE));
		IF_ARGF(match = match && l2c_is<argF_ty>(L,arg_idx+0xF));
		return match;
	}

	//invokes the function, potentially pushing the result onto the stack
	virtual int Invoke(lua_State* L)
	{
		//get base argument index
		int base_idx			= lua_absindex(L,-1-FUNC_EXPECTED_ARGC+1);
		int arg_idx				= base_idx;

		//get the instance if this is a member function
		#if FDEF_TYPE == FTYPE_MEMBER
		ty& instance			= *l2c_to<ty*>(L,arg_idx);
		arg_idx++;
		#endif

		//begin the return 'push' if we return a value (note: constructors automatically push through call to l2cinternal_allocuserdatavalue)
		#if defined(FDEF_RETURN) && FDEF_TYPE != FTYPE_CONSTRUCTOR
		l2c_pushval(L,
		#endif

		//the actual function call
		FUNC_CALL_PREFIX(FUNC_CALL_ARGS)

		//end the return 'push' if we return a value, then follow with the end of line
		#if defined(FDEF_RETURN) && FDEF_TYPE != FTYPE_CONSTRUCTOR
		)
		#endif
		;

		//clean up stack, and return the result if we have one
		#ifdef FDEF_RETURN
		lua_copy(L,-1,base_idx);
		lua_pop(L,FUNC_EXPECTED_ARGC);
		return 1;
		#else
		lua_pop(L,FUNC_EXPECTED_ARGC);
		return 0;
		#endif
	}
};

//////////////////////////////////////////////////////////////////////////
//This is the function used to build a functor, called from the L2C
//macros. It takes a config and a function pointer, and uses them to
//construct a functor with the correct arguments. Note that the constructor
//still takes the pointer to use for type expansion, though it can be
//null as it is not actually used
//////////////////////////////////////////////////////////////////////////
FUNC_TEMPLATE
inline FDEF_BASE_CLASS* FUNC_NAME(FDEF_BASE_CLASS::Config config, FUNC_TYPEDEF(func))
{
	return new FUNCTOR_NAME<
		IF_CONSTRUCTOR_OR_MEMBER(ty,)
		IF_RETURNING_GLOBAL_OR_MEMBER(ret_ty,)
		IF_ARG0(arg0_ty,)
		IF_ARG1(arg1_ty,)
		IF_ARG2(arg2_ty,)
		IF_ARG3(arg3_ty,)
		IF_ARG4(arg4_ty,)
		IF_ARG5(arg5_ty,)
		IF_ARG6(arg6_ty,)
		IF_ARG7(arg7_ty,)
		IF_ARG8(arg8_ty,)
		IF_ARG9(arg9_ty,)
		IF_ARGA(argA_ty,)
		IF_ARGB(argB_ty,)
		IF_ARGC(argC_ty,)
		IF_ARGD(argD_ty,)
		IF_ARGE(argE_ty,)
		IF_ARGF(argF_ty,)
		0
		>(config IF_GLOBAL_OR_MEMBER(,func));
}

//clear out the massive list of preprocessor crazy!
#undef IF_RETURN
#undef IF_VOID
#undef RET_NAME
#undef RET_TYPE
#undef IF_ARG0
#undef IF_ARG1
#undef IF_ARG2
#undef IF_ARG3
#undef IF_ARG4
#undef IF_ARG5
#undef IF_ARG6
#undef IF_ARG7
#undef IF_ARG8
#undef IF_ARG9
#undef IF_ARGA
#undef IF_ARGB
#undef IF_ARGC
#undef IF_ARGD
#undef IF_ARGE
#undef IF_ARGF
#undef _FUNCTOR_NAME_FORMAT
#undef FUNCTOR_NAME_FORMAT
#undef FUNCTOR_NAME
#undef FUNC_NAME
#undef FUNC_TEMPLATE
#undef IF_CONSTRUCTOR_OR_MEMBER
#undef IF_GLOBAL_OR_MEMBER
#undef IF_RETURNING_GLOBAL_OR_MEMBER
#undef FUNC_TYPEDEF_PREFIX
#undef FUNC_TYPEDEF_ARGS
#undef FUNC_TYPEDEF
#undef FUNC_CALL_PREFIX
#undef FUNC_EXPECTED_ARGC

#endif