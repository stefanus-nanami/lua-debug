#include <cstdio>
#include <iostream>
#include <vector>
#include <unistd.h>

#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"
#include "LRDB/server.hpp"

using namespace std;
using namespace luabridge;
using namespace lrdb;

class LuaApp
{
	lua_State* mLuaState;
	vector< char > mPackagePath;
  const char* mScriptPath;

	void setupEnvironment() {
	  LuaRef package = getGlobal(mLuaState, "package");

		const char* lua_file = "?.lua";
		
		int length = snprintf(nullptr, 0, "%s/%s", mScriptPath, lua_file);
	  mPackagePath.reserve(length + 1);

		snprintf(mPackagePath.data(), mPackagePath.capacity(),	"%s/%s", mScriptPath, lua_file);
		cout << "Package path = " << mPackagePath.data() << endl;

		package["path"] = static_cast< const char* >(mPackagePath.data());
	}

public:
	LuaApp(const char* script_path)
		: mLuaState(nullptr),
      mScriptPath(script_path)
	{
		lua_State* luaState = luaL_newstate();
		if (luaState != nullptr) {
			mLuaState = luaState;
			luaL_openlibs(mLuaState);
			setupEnvironment();
		}
	}

	~LuaApp() {
		if(mLuaState != nullptr) {
			luaL_openlibs(mLuaState);
			lua_close(mLuaState);
			mLuaState = nullptr;
		}
	}

	void execute() {
	  lrdb::server debug_server(21110);
	  debug_server.reset(mLuaState);

		lua_getglobal(mLuaState, "require");
		lua_pushstring(mLuaState, "main");
		int err = lua_pcall(mLuaState, 1, 0, 0);
		if(err != LUA_OK) {
			cout << "Failed to run main." << endl;
		}

		debug_server.reset();
	}

};

int main(int argc, const char** argv)
{
  if(argc != 2) {
    cout << "Usage: DebugLua [script_path]" << endl;
    return 1;
  }

  LuaApp app(argv[1]);
	app.execute();
	return 0;
}
