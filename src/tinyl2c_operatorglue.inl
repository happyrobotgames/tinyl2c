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
		l2c_push(L,res);
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
		l2c_push(L,res);
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
