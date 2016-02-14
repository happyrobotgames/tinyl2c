
//member variable
template<typename ty, typename var_ty> class TL2CVariable : public L2CVariable
{
public:
	int m_offset;

	TL2CVariable(Config config, int offset) : L2CVariable(config), m_offset(offset) {}

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
		l2c_push(L,*member);
		lua_copy(L,-1,-2);
		lua_pop(L,1);
		return 1;
	}
};
template<typename ty, typename var_ty> 
inline L2CVariable* l2cinternal_buildvariable(L2CVariable::Config config, int offset)
{
	return new TL2CVariable<ty, var_ty>(config,offset);
}

//global variable
template<typename var_ty> class TL2CGlobalVariable : public L2CVariable
{
public:
	var_ty* m_variable;

	TL2CGlobalVariable(Config config, var_ty* variable) : L2CVariable(config), m_variable(variable) {}

	virtual int Set(lua_State* L)
	{
		*m_variable	= l2c_to<var_ty>(L,-1);
		lua_pop(L,1);
		return 0;
	}

	virtual int Get(lua_State* L)
	{
		l2c_push(L,*m_variable);
		return 1;
	}
};
template<typename var_ty> 
inline L2CVariable* l2cinternal_buildglobalvariable(L2CVariable::Config config, int offset)
{
	return new TL2CGlobalVariable<var_ty>(config,offset);
}
