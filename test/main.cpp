#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include "../src/tinyl2c.h"

struct TestBase
{
	int hello = 7;

	void BaseFoo() { hello = 2; }
};
L2C_TYPEDECL(TestBase)

struct TestWeakType : public TestBase
{
	int x = 0;
	int y = 2;
	int z = 4;

	void Foo() { x = 1; y = 3; z = 4; }
	void Foo2(int v) { x = v; y = v; z = v; }
	int Foo3() { return x+y+z; }

	TestWeakType() {}
};
L2C_TYPEDECL(TestWeakType)

L2C_TYPEDEF_BEGIN(TestBase)
	L2C_VARIABLE(hello)
	L2C_FUNCTION(BaseFoo)
L2C_TYPEDEF_END()

L2C_TYPEDEF_BEGIN(TestWeakType)
	L2C_INHERITS(TestBase)
	L2C_VARIABLE(x)
	L2C_VARIABLE(y)
	L2C_VARIABLE(z)
	L2C_FUNCTION(Foo)
	L2C_FUNCTION(Foo2)
	L2C_FUNCTION(Foo3)
L2C_TYPEDEF_END()

template<typename ty> void TestNumericType(lua_State* L)
{
	ty val = 10;
	l2c_push(L, val);
	bool isres = l2c_is<ty>(L, -1);
	ty tores = l2c_to<ty>(L, -1);
	lua_pop(L, 1);
}

int ptest(lua_State* L)
{

	TestNumericType<int8_t>(L);
	TestNumericType<float32_t>(L);

	{
		TestWeakType mytype;
		l2c_push(L, mytype);
		mytype.y = 20;
		TestWeakType res = l2c_to<TestWeakType>(L,-1);
		lua_pop(L,1);
	}

	
	{
		TestWeakType mytype;
		l2c_push(L, &mytype);
		mytype.y = 30;
		TestWeakType res = l2c_to<TestWeakType>(L,-1);
		lua_pop(L,1);
	}

	return 0;
}

int gettesttype(lua_State* L)
{
	TestWeakType t;
	l2c_push(L,t);
	return 1;
}

int main(int argc, _TCHAR* argv[])
{
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);

	lua_pushcfunction(L,gettesttype);
	lua_setglobal(L,"gettesttype");

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
