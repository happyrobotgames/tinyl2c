//////////////////////////////////////////////////////////////////////////
//Generated templates to allow C++ function invocation from the LUA
//stack. Each entry is:
//- a function takes n parameters, and can be:
//   - member function, returns void 
//   - member function, returns value
//   - global function, returns void 
//   - global function, returns value
//   - constructor
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
	virtual bool CheckSig(lua_State* L)
	{
		return lua_gettop(L) == 1 && l2c_is<ty*>(L,-1);
	}
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

//ret_ty ty::function()
//LUA stack inputs: 0
//LUA stack outputs: 1
template<typename ty, typename ret_ty> class TL2CFunc_Ret_0 : public L2CFunction
{
public:
	typedef ret_ty (ty::*func_ptr)();

	func_ptr m_func;

	TL2CFunc_Ret_0(Config config, func_ptr func) : L2CFunction(config), m_func(func) {}

	virtual bool CheckSig(lua_State* L)
	{
		return lua_gettop(L) == 1 && l2c_is<ty*>(L,-1);
	}
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

//void function()
//LUA stack inputs: 0
//LUA stack outputs: 0
class TL2CGlobalFunc_Void_0 : public L2CFunction
{
public:
	typedef void (*func_ptr)();

	func_ptr m_func;

	TL2CGlobalFunc_Void_0(Config config, func_ptr func) : L2CFunction(config), m_func(func) {}

	virtual bool CheckSig(lua_State* L)
	{
		return lua_gettop(L) == 0;
	}
	virtual int Invoke(lua_State* L)
	{
		(m_func)();
		return 0;
	}
};
inline L2CFunction* l2cinternal_buildglobalfunc(L2CFunction::Config config, void (*func)())
{
	return new TL2CGlobalFunc_Void_0(config,func);
}

//ret_ty ty::function()
//LUA stack inputs: 0
//LUA stack outputs: 1
template<typename ret_ty> class TL2CGlobalFunc_Ret_0 : public L2CFunction
{
public:
	typedef ret_ty (*func_ptr)();

	func_ptr m_func;

	TL2CGlobalFunc_Ret_0(Config config, func_ptr func) : L2CFunction(config), m_func(func) {}

	virtual bool CheckSig(lua_State* L)
	{
		return lua_gettop(L) == 0;
	}
	virtual int Invoke(lua_State* L)
	{
		l2c_push(L,(m_func)());
		lua_copy(L,-1,-2);
		lua_pop(L,1);
		return 1;
	}
};
template<typename ret_ty> 
inline L2CFunction* l2cinternal_buildglobalfunc(L2CFunction::Config config, ret_ty (*func)() )
{
	return new TL2CGlobalFunc_Ret_0<ret_ty>(config,func);
}

//constructor ty()
//LUA stack inputs: 0
//LUA stack outputs: 1
template<typename ty> class TL2CFunc_Construct_0 : public L2CFunction
{
public:
	typedef ty* (*func_ptr)();

	TL2CFunc_Construct_0(Config config, func_ptr func) : L2CFunction(config) {}

	virtual bool CheckSig(lua_State* L)
	{
		return lua_gettop(L) == 0;
	}
	virtual int Invoke(lua_State* L)
	{
		void* addr = l2cinternal_allocuserdatavalue<ty>(L)->m_data;
		new (addr) ty();
		return 1;
	}
};
template<typename ty> 
inline L2CFunction* l2cinternal_buildconstructor(L2CFunction::Config config, ty* (*func)())
{
	return new TL2CFunc_Construct_0<ty>(config,func);
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

	virtual bool CheckSig(lua_State* L)
	{
		return lua_gettop(L) == 2 && l2c_is<ty*>(L,-2) && l2c_is<arg0_ty>(L,-1);
	}
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


//constructor ty(arg0_ty)
//LUA stack inputs: 0
//LUA stack outputs: 1
template<typename ty, typename arg0_ty> class TL2CFunc_Construct_1 : public L2CFunction
{
public:
	typedef ty* (*func_ptr)(arg0_ty);

	TL2CFunc_Construct_1(Config config, func_ptr func) : L2CFunction(config) {}

	virtual bool CheckSig(lua_State* L)
	{
		return lua_gettop(L) == 1 && l2c_is<arg0_ty>(L,-1);
	}
	virtual int Invoke(lua_State* L)
	{
		void* addr = l2cinternal_allocuserdatavalue<ty>(L)->m_data;
		new (addr) ty(l2c_to<arg0_ty>(L,-2));
		return 1;
	}
};
template<typename ty, typename arg0_ty> 
inline L2CFunction* l2cinternal_buildconstructor(L2CFunction::Config config, ty* (*func)(arg0_ty))
{
	return new TL2CFunc_Construct_1<ty,arg0_ty>(config,func);
}