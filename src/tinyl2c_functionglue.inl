//////////////////////////////////////////////////////////////////////////
//Generated templates to allow C++ function invocation from the LUA
//stack. Each entry is:
//- a function that either returns void or a value, and has n parameters
//- a template class specialization used to invoke it
//- a template function overload used to build it
//////////////////////////////////////////////////////////////////////////

//void ty::function()
//LUA stack inputs: 0
//LUA stack outputs: 0
template<typename ty> class TL2CFunc_Void_0 : public L2CFunction
{
public:
	typedef void (ty::*func_ptr)();

	func_ptr m_func;

	TL2CFunc_Void_0(Config config, func_ptr func) : L2CFunction(config), m_func(func) {}

	virtual int Invoke(lua_State* L)
	{
		ty& instance = *l2c_to<ty*>(L,-1);
		(instance.*m_func)();
		lua_pop(L,1);
		return 0;
	}
};
template<typename ty> 
inline L2CFunction* l2cinternal_buildfunc(L2CFunction::Config config, void (ty::*func)())
{
	return new TL2CFunc_Void_0<ty>(config,func);
}

//void ty::function(arg0_ty)
//LUA stack inputs: 1
//LUA stack outputs: 0
template<typename ty, typename arg0_ty> class TL2CFunc_Void_1 : public L2CFunction
{
public:
	typedef void (ty::*func_ptr)(arg0_ty);

	func_ptr m_func;

	TL2CFunc_Void_1(Config config, func_ptr func) : L2CFunction(config), m_func(func) {}

	virtual int Invoke(lua_State* L)
	{
		ty& instance = *l2c_to<ty*>(L,-2);
		(instance.*m_func)(l2c_to<arg0_ty>(L,-1));
		lua_pop(L,1);
		return 0;
	}
};
template<typename ty, typename arg0_ty> 
inline L2CFunction* l2cinternal_buildfunc(L2CFunction::Config config, void (ty::*func)(arg0_ty))
{
	return new TL2CFunc_Void_1<ty,arg0_ty>(config,func);
}

//ret_ty ty::function()
//LUA stack inputs: 0
//LUA stack outputs: 1
template<typename ty, typename ret_ty> class TL2CFunc_Ret_0 : public L2CFunction
{
public:
	typedef ret_ty (ty::*func_ptr)();

	func_ptr m_func;

	TL2CFunc_Ret_0(Config config, func_ptr func) : L2CFunction(config), m_func(func) {}

	virtual int Invoke(lua_State* L)
	{
		ty& instance = *l2c_to<ty*>(L,-1);
		l2c_push(L,(instance.*m_func)());
		lua_copy(L,-1,-2);
		lua_pop(L,1);
		return 1;
	}
};
template<typename ty, typename ret_ty> 
inline L2CFunction* l2cinternal_buildfunc(L2CFunction::Config config, ret_ty (ty::*func)() )
{
	return new TL2CFunc_Ret_0<ty,ret_ty>(config,func);
}
