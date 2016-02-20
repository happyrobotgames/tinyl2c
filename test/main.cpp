#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include "../src/tinyl2c.h"

struct V3
{
	V3() : x(0), y(0), z(0) {}
	V3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

	inline V3 operator+(V3 other) const
	{
		V3 res = *this;
		res.x += other.x;
		res.y += other.y;
		res.z += other.z;
		return res;
	}
	inline V3 operator-() const
	{
		V3 res = *this;
		res.x = -res.x;
		res.y = -res.y;
		res.z = -res.z;
		return res;
	}

private:
	float x,y,z;

	friend class L2CTypeInterface<V3>;
	
};
L2C_TYPEDECL(V3)

/*
inline V3 operator*(V3 a, float b)
{
	V3 res = a;
	res.x *= b;
	res.y *= b;
	res.z *= b;
	return res;
}
inline V3 operator*(float a, V3 b)
{
	V3 res = b;
	res.x *= a;
	res.y *= a;
	res.z *= a;
	return res;
}
*/

struct TestBase
{
	int hello = 7;

	void BaseFoo() { hello = 2; }
};
L2C_TYPEDECL(TestBase)

struct Test : public TestBase
{
	int x = 0;
	int y = 2;
	int z = 4;
	V3 val;

	void Foo() { x = 1; y = 3; z = 4; }
	void Foo2(int v) { x = v; y = v; z = v; }
	int Foo3() { return x+y+z; }
	V3 GetVal() { return val; }
	V3* GetValRef() { return &val; }

	Test() {}
	Test(int b) { x=y=z=b; }
};
L2C_TYPEDECL(Test)

L2C_TYPEDEF_BEGIN(V3)
	L2C_CONSTRUCTOR()
	L2C_CONSTRUCTOR(float,float,float)
	L2C_VARIABLE(x)
	L2C_VARIABLE(y)
	L2C_VARIABLE(z)
	L2C_OP_ADD(V3,V3,V3)
	//L2C_OP_MUL(V3,V3,float)
	//L2C_OP_MUL(V3,float,V3)
	L2C_OP_UNM(V3,V3)

L2C_TYPEDEF_END()

L2C_TYPEDEF_BEGIN(TestBase)
	L2C_VARIABLE(hello)
	L2C_FUNCTION(BaseFoo)
L2C_TYPEDEF_END()

L2C_TYPEDEF_BEGIN(Test)
	L2C_INHERITS(TestBase)

	L2C_CONSTRUCTOR()
	L2C_CONSTRUCTOR(int)

	L2C_VARIABLE(x)
	L2C_VARIABLE(y)
	L2C_VARIABLE(z)
	L2C_VARIABLE(val)

	L2C_FUNCTION(Foo)
	L2C_FUNCTION(Foo2)
	L2C_FUNCTION(Foo3)
	L2C_FUNCTION(GetVal)
	L2C_FUNCTION(GetValRef)

L2C_TYPEDEF_END()

template<typename ty> void TestNumericType(lua_State* L)
{
	ty val = 10;
	l2c_pushval(L, val);
	bool isres = l2c_is<ty>(L, -1);
	ty tores = l2c_to<ty>(L, -1);
	lua_pop(L, 1);
}

int ptest(lua_State* L)
{

	TestNumericType<int8_t>(L);
	TestNumericType<float32_t>(L);

	return 0;
}

Test gTest;

void TestFunc()
{
	gTest.x = 20;
}

int main(int argc, _TCHAR* argv[])
{
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);

	l2c_addglobal(L,gTest);
	l2c_addglobalfunction(L,TestFunc);
	l2c_addtype<V3>(L);

	lua_pushcfunction(L,l2c_printobject);
	lua_setglobal(L,"printobject");

	lua_pushcfunction(L,l2c_printtable);
	lua_setglobal(L,"printtable");

	lua_pushcfunction(L, ptest);
	if (lua_pcall(L, 0, 0, 0) != LUA_OK)
	{
		l2c_printf("Test function returned with error: %s\n", lua_tostring(L, -1));
	}
	
	if (luaL_loadfile(L, "test/console.lua") != LUA_OK)
	{
		l2c_printf("Failed to load console.lua with error: %s\n", lua_tostring(L, -1));
	}
	else
	{
		if (lua_pcall(L, 0, 0, 0) != LUA_OK)
		{
			l2c_printf("Run console.lua returned with error: %s\n", lua_tostring(L, -1));
		}
	}

	return 0;
}
