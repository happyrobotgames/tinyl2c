//void ty::function(arg0_ty)
//LUA stack inputs: 1
//LUA stack outputs: 0

#ifndef _FDEF_RECURSING

#ifndef FDEF_ARG_COUNT
#error "You must specify an arg count"
#endif

#define _FDEF_RECURSING

#define FTYPE_MEMBER 1
#define FTYPE_GLOBAL 2
#define FTYPE_CONSTRUCTOR 3
#define FDEF_BASE_CLASS L2CFunction
#define FDEF_TYPE 

#undef FDEF_TYPE
#define FDEF_TYPE FTYPE_MEMBER
#include "tinyl2c_functionglue_preproc.inl"

#undef FDEF_TYPE
#define FDEF_TYPE FTYPE_GLOBAL
#include "tinyl2c_functionglue_preproc.inl"

#define FDEF_RETURN

#undef FDEF_TYPE
#define FDEF_TYPE FTYPE_MEMBER
#include "tinyl2c_functionglue_preproc.inl"

#undef FDEF_TYPE
#define FDEF_TYPE FTYPE_GLOBAL
#include "tinyl2c_functionglue_preproc.inl"

#undef FDEF_TYPE
#define FDEF_TYPE FTYPE_CONSTRUCTOR
#include "tinyl2c_functionglue_preproc.inl"

#undef FDEF_RETURN
#undef FTYPE_MEMBER
#undef FTYPE_GLOBAL
#undef FTYPE_CONSTRUCTOR
#undef FDEF_BASE_CLASS
#undef FDEF_TYPE
#undef FDEF_ARG_COUNT
#undef _FDEF_RECURSING

#else

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

#if FDEF_TYPE == FTYPE_MEMBER
#define _FUNC_NAME_FORMAT(_nm,_args) TL2CMemberFunc_##_nm##_##_args
#elif FDEF_TYPE == FTYPE_GLOBAL
#define _FUNC_NAME_FORMAT(_nm,_args) TL2CGlobalFunc_##_nm##_##_args
#elif FDEF_TYPE == FTYPE_CONSTRUCTOR
#define _FUNC_NAME_FORMAT(_nm,_args) TL2CConstructorFunc_##_args
#endif

#define FUNC_NAME_FORMAT(_nm,_args) _FUNC_NAME_FORMAT(_nm,_args)
#define FUNC_NAME FUNC_NAME_FORMAT(RET_NAME,FDEF_ARG_COUNT)

template<
	#if FDEF_TYPE == FTYPE_CONSTRUCTOR || FDEF_TYPE == FTYPE_MEMBER
	typename ty,
	#endif
	#if FDEF_TYPE != FTYPE_CONSTRUCTOR
	IF_RETURN(typename ret_ty,)
	#endif
	IF_ARG0(typename arg0_ty,)
	IF_ARG1(typename arg1_ty,)
	IF_ARG2(typename arg2_ty,)
	IF_ARG3(typename arg3_ty,)
	IF_ARG4(typename arg4_ty,)
	IF_ARG5(typename arg5_ty,)
	IF_ARG6(typename arg6_ty,)
	IF_ARG7(typename arg7_ty,)
	IF_ARG8(typename arg8_ty,)
	IF_ARG9(typename arg9_ty,)
	IF_ARGA(typename argA_ty,)
	IF_ARGB(typename argB_ty,)
	IF_ARGC(typename argC_ty,)
	IF_ARGD(typename argD_ty,)
	IF_ARGE(typename argE_ty,)
	IF_ARGF(typename argF_ty,)
	int dummy=0
> 
class FUNC_NAME : public FDEF_BASE_CLASS
{
public:
	#if FDEF_TYPE == FTYPE_GLOBAL || FDEF_TYPE == FTYPE_MEMBER 
	typedef RET_TYPE (
		#if FDEF_TYPE == FTYPE_MEMBER
		ty::
		#endif
		*func_ptr)
		(
			IF_ARG0( arg0_ty)
			IF_ARG1(,arg1_ty)
			IF_ARG2(,arg2_ty)
			IF_ARG3(,arg3_ty)
			IF_ARG4(,arg4_ty)
			IF_ARG5(,arg5_ty)
			IF_ARG6(,arg6_ty)
			IF_ARG7(,arg7_ty)
			IF_ARG8(,arg8_ty)
			IF_ARG9(,arg9_ty)
			IF_ARGA(,argA_ty)
			IF_ARGB(,argB_ty)
			IF_ARGC(,argC_ty)
			IF_ARGD(,argD_ty)
			IF_ARGE(,argE_ty)
			IF_ARGF(,argF_ty)
		);
	func_ptr m_func;
	FUNC_NAME(Config config, func_ptr func) : FDEF_BASE_CLASS(config), m_func(func) {}
	#else
	typedef ty ret_ty;
	FUNC_NAME(Config config) : FDEF_BASE_CLASS(config) {}
	#endif


	virtual bool CheckSig(lua_State* L)
	{
		if (lua_gettop(L) != (FDEF_ARG_COUNT+1))
			return false;

		int base_idx			= lua_absindex(L,-1-FDEF_ARG_COUNT);
		int arg_idx				= base_idx;

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
	virtual int Invoke(lua_State* L)
	{
		int base_idx			= lua_absindex(L,-1-FDEF_ARG_COUNT);
		int arg_idx				= base_idx;

		#if FDEF_TYPE == FTYPE_MEMBER
		ty& instance			= *l2c_to<ty*>(L,base_idx);
		arg_idx++;
		#endif

		#ifdef FDEF_RETURN
		l2c_push
		(L,
		#endif
		#if FDEF_TYPE == FTYPE_MEMBER
			(instance.*m_func)
		#elif FDEF_TYPE == FTYPE_GLOBAL
			(m_func)
		#elif FDEF_TYPE == FTYPE_CONSTRUCTOR
			new ty
		#endif
			(
				IF_ARG0( l2c_to<arg0_ty>(L,arg_idx+0x0))
				IF_ARG1(,l2c_to<arg1_ty>(L,arg_idx+0x1))
				IF_ARG2(,l2c_to<arg2_ty>(L,arg_idx+0x2))
				IF_ARG3(,l2c_to<arg3_ty>(L,arg_idx+0x3))
				IF_ARG4(,l2c_to<arg4_ty>(L,arg_idx+0x4))
				IF_ARG5(,l2c_to<arg5_ty>(L,arg_idx+0x5))
				IF_ARG6(,l2c_to<arg6_ty>(L,arg_idx+0x6))
				IF_ARG7(,l2c_to<arg7_ty>(L,arg_idx+0x7))
				IF_ARG8(,l2c_to<arg8_ty>(L,arg_idx+0x8))
				IF_ARG9(,l2c_to<arg9_ty>(L,arg_idx+0x9))
				IF_ARGA(,l2c_to<argA_ty>(L,arg_idx+0xA))
				IF_ARGB(,l2c_to<argB_ty>(L,arg_idx+0xB))
				IF_ARGC(,l2c_to<argC_ty>(L,arg_idx+0xC))
				IF_ARGD(,l2c_to<argD_ty>(L,arg_idx+0xD))
				IF_ARGE(,l2c_to<argE_ty>(L,arg_idx+0xE))
				IF_ARGF(,l2c_to<argF_ty>(L,arg_idx+0xF))
			)
		#ifdef FDEF_RETURN
		)
		#endif
		;

		#ifdef FDEF_RETURN
		lua_copy(L,-1,base_idx);
		lua_pop(L,FDEF_ARG_COUNT);
		return 1;
		#else
		lua_pop(L,FDEF_ARG_COUNT);
		return 0;
		#endif
	}
};


template<
	#if FDEF_TYPE == FTYPE_CONSTRUCTOR || FDEF_TYPE == FTYPE_MEMBER
	typename ty,
	#endif
	#if FDEF_TYPE != FTYPE_CONSTRUCTOR
	IF_RETURN(typename ret_ty,)
	#endif
	IF_ARG0(typename arg0_ty,)
	IF_ARG1(typename arg1_ty,)
	IF_ARG2(typename arg2_ty,)
	IF_ARG3(typename arg3_ty,)
	IF_ARG4(typename arg4_ty,)
	IF_ARG5(typename arg5_ty,)
	IF_ARG6(typename arg6_ty,)
	IF_ARG7(typename arg7_ty,)
	IF_ARG8(typename arg8_ty,)
	IF_ARG9(typename arg9_ty,)
	IF_ARGA(typename argA_ty,)
	IF_ARGB(typename argB_ty,)
	IF_ARGC(typename argC_ty,)
	IF_ARGD(typename argD_ty,)
	IF_ARGE(typename argE_ty,)
	IF_ARGF(typename argF_ty,)
	int dummy=0
	> 
inline FDEF_BASE_CLASS* 
		#if FDEF_TYPE == FTYPE_MEMBER
l2cinternal_buildmemberfunc
		#elif FDEF_TYPE == FTYPE_GLOBAL
l2cinternal_buildglobalfunc
		#elif FDEF_TYPE == FTYPE_CONSTRUCTOR
l2cinternal_buildconstructorfunc
		#endif
	(FDEF_BASE_CLASS::Config config
	#if FDEF_TYPE == FTYPE_GLOBAL || FDEF_TYPE == FTYPE_MEMBER 
	, RET_TYPE (
		#if FDEF_TYPE == FTYPE_MEMBER
		ty::
		#endif
	#else
	, ty* (
	#endif
		*func)
		(
			IF_ARG0( arg0_ty)
			IF_ARG1(,arg1_ty)
			IF_ARG2(,arg2_ty)
			IF_ARG3(,arg3_ty)
			IF_ARG4(,arg4_ty)
			IF_ARG5(,arg5_ty)
			IF_ARG6(,arg6_ty)
			IF_ARG7(,arg7_ty)
			IF_ARG8(,arg8_ty)
			IF_ARG9(,arg9_ty)
			IF_ARGA(,argA_ty)
			IF_ARGB(,argB_ty)
			IF_ARGC(,argC_ty)
			IF_ARGD(,argD_ty)
			IF_ARGE(,argE_ty)
			IF_ARGF(,argF_ty)
		)
	)
{
	return new FUNC_NAME<
		#if FDEF_TYPE == FTYPE_CONSTRUCTOR || FDEF_TYPE == FTYPE_MEMBER
		ty,
		#endif
		#if FDEF_TYPE != FTYPE_CONSTRUCTOR
		IF_RETURN(ret_ty,)
		#endif
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
		>(
		config
		#if FDEF_TYPE == FTYPE_GLOBAL || FDEF_TYPE == FTYPE_MEMBER 
		,func
		#endif
		);
}

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
#undef _FUNC_NAME_FORMAT
#undef FUNC_NAME_FORMAT
#undef FUNC_NAME

#endif