//////////////////////////////////////////////////////////////////////////
//Template to generate get and set functions to access a member
//of a class
//////////////////////////////////////////////////////////////////////////
template<typename ty, typename member_ty> class TL2CVariable : public L2CVariable
{
public:
	int m_offset;

	TL2CVariable(Config config, int offset) : L2CVariable(config), m_offset(offset) {}

	virtual int Set(lua_State* L)
	{
		ty* instance		= l2c_to<ty*>(L,-2);
		member_ty* member	= (member_ty*)l2c_addptr(instance,m_offset);
		*member				= l2c_to<member_ty>(L,-1);
		lua_pop(L,2);
		return 0;
	}

	virtual int Get(lua_State* L)
	{
		ty* instance		= l2c_to<ty*>(L,-1);
		member_ty* member	= (member_ty*)l2c_addptr(instance,m_offset);
		l2c_push(L,*member);
		lua_copy(L,-1,-2);
		lua_pop(L,1);
		return 1;
	}
};
template<typename ty, typename member_ty> 
inline L2CVariable* l2cinternal_buildvariable(L2CVariable::Config config, int offset)
{
	return new TL2CVariable<ty, member_ty>(config,offset);
}
