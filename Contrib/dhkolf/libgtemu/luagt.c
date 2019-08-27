/*

    This file is part of libgtemu, a library for Gigatron emulation.
    Copyright (C) 2019 David Heiko Kolf

    Published under the BSD-2-Clause license.
    https://opensource.org/licenses/BSD-2-Clause

*/

#include <lua.h>
#include <lauxlib.h>
#include <SDL2/SDL.h>

#include <time.h>

#include "gtemu.h"
#include "gtsdl.h"

#define newud(L,t) ((t*)lua_newuserdata((L),sizeof(t)))

struct GTEmulationData {
	struct GTState gt;
	struct GTPeriph ph;
	struct GTRomEntry rom[0x10000];
	unsigned char ram[0x10000];
};

static struct GTEmulationData *checkemu (lua_State *L, int n)
{
	return (struct GTEmulationData *)
		luaL_checkudata(L, 1, "gtemu.emulation");
}

static int lgtsdlerror (lua_State *L)
{
	return luaL_error(L, "SDL: %s", SDL_GetError());
}

static void lgtclosesdl (void)
{
	SDL_Quit();
}

static int lgtinitsdl (lua_State *L)
{
	if (SDL_Init(0) < 0) {
		return lgtsdlerror(L);
	}
	atexit(lgtclosesdl);
	return 0;
}

static int lgtopenwindow (lua_State *L)
{
	const char *str = lua_tostring(L, 1);

	struct GTSDLState *s = newud(L, struct GTSDLState);

	memset(s, 0, sizeof(*s));
	luaL_setmetatable(L, "gtemu.sdlstate");

	if (!gtsdl_openwindow(s, str)) {
		return lgtsdlerror(L);
	}

	return 1;
}

static int lgtclosewindow (lua_State *L)
{
	struct GTSDLState *s = (struct GTSDLState *)
		luaL_checkudata(L, 1, "gtemu.sdlstate");
	gtsdl_close(s);
	return 0;
}

static int lgtnewemulation (lua_State *L)
{
	struct GTSDLState *s = (struct GTSDLState *)
		luaL_checkudata(L, 1, "gtemu.sdlstate");
	int ramkb = luaL_optinteger(L, 2, 32);
	size_t ramsize;
	unsigned long rngstate;
	struct GTEmulationData *emu;

	if (ramkb == 32) {
		ramsize = 0x8000;
	} else if (ramkb == 64) {
		ramsize = 0x10000;
	} else {
		return luaL_argerror(L, 2, "is neither 32 nor 64");
	}

	emu = newud(L, struct GTEmulationData);

	rngstate = time(0);
	rngstate = gtemu_randomizemem(rngstate, emu->rom, sizeof(emu->rom));
	rngstate = gtemu_randomizemem(rngstate, emu->ram, sizeof(emu->ram));

	gtemu_init(&emu->gt, emu->rom, sizeof(emu->rom),
		emu->ram, ramsize);
	gtemu_initperiph(&emu->ph, gtsdl_getaudiofreq(s), rngstate);

	luaL_setmetatable(L, "gtemu.emulation");
	return 1;
}

static int lgtemuloadrom (lua_State *L)
{
	struct GTEmulationData *emu = checkemu(L, 1);
	size_t len, i, j;
	const char *str = lua_tolstring(L, 2, &len);

	for (i = 0, j = 0;
		i + 1 < len && j < sizeof(emu->rom) / sizeof(emu->rom[0]);
		i += 2, j++) {

		emu->rom[j].i = (unsigned char) str[i];
		emu->rom[j].d = (unsigned char) str[i+1];
	}

	return 0;
}

static int lgtemuprocesstick (lua_State *L)
{
	struct GTEmulationData *emu = checkemu(L, 1);
	lua_pushinteger(L, gtemu_processtick(&emu->gt, &emu->ph));
	return 1;
}

static int lgtemuprocessscreen (lua_State *L)
{
	struct GTEmulationData *emu = checkemu(L, 1);
	size_t dummy;
	lua_pushinteger(L, gtemu_processscreen(&emu->gt, &emu->ph,
		NULL, 0, NULL, 0, &dummy));
	return 1;
}

static int lgtemusendgt1 (lua_State *L)
{
	struct GTEmulationData *emu = checkemu(L, 1);
	size_t len;
	const char *str = lua_tolstring(L, 2, &len);
	int res;

	res = gtloader_validategt1(str, len);

	if (!res) {
		return luaL_error(L, "invalid GT1 structure");
	}

	if (gtloader_sendgt1(&emu->ph, str, len)) {
		/* prevent garbage collection while the loader is working */
		lua_pushvalue(L, 1);
		lua_pushvalue(L, 2);
		lua_settable(L, lua_upvalueindex(1));
	} else {
		luaL_error(L, "loader is busy");
	}

	return 0;
}

static int lgtemucreatebuffer (lua_State *L)
{
	struct GTEmulationData *emu = checkemu(L, 1);
	lua_Integer size = luaL_checkinteger(L, 2);
	size_t *buf = (size_t *) lua_newuserdata(L, sizeof(size_t) + size);
	buf[0] = 0;
	gtserialout_setbuffer(&emu->ph, (char *)&buf[1], size, buf);
	lua_pushvalue(L, 1);
	lua_pushvalue(L, -2);
	lua_rawset(L, lua_upvalueindex(2));
	return 1;
}

static int lgtemugetbuffer (lua_State *L)
{
	size_t *buf;
	lua_pushvalue(L, 1);
	lua_rawget(L, lua_upvalueindex(2));
	buf = lua_touserdata(L, -1);
	if (buf != NULL) {
		lua_pushlstring(L, (char *)&buf[1], buf[0]);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

static int lgtemuresetbuffer (lua_State *L)
{
	size_t *buf;
	lua_pushvalue(L, 1);
	lua_rawget(L, lua_upvalueindex(2));
	buf = lua_touserdata(L, -1);
	if (buf != NULL) {
		buf[0] = 0;
	}
	return 0;
}

static int lgtemuindex (lua_State *L)
{
	struct GTEmulationData *emu = checkemu(L, 1);
	int idx, isnum;

	idx = lua_tointegerx(L, 2, &isnum);

	if (isnum && idx >= 0 && idx < 0x10000) {
		lua_pushinteger(L, emu->ram[idx & emu->gt.rammask]);
		return 1;
	}

	lua_pushvalue(L, 2);
	lua_rawget(L, lua_upvalueindex(2));
	if (!lua_isnil(L, -1)) {
		lua_pushvalue(L, 1);
		lua_call(L, 1, 1);
		return 1;
	}

	lua_pushvalue(L, 2);
	lua_rawget(L, lua_upvalueindex(1));
	return 1;
}

static int lgtemunewindex (lua_State *L)
{
	struct GTEmulationData *emu = checkemu(L, 1);
	int idx, isnum;

	idx = lua_tointegerx(L, 2, &isnum);

	if (isnum && idx >= 0 && idx < 0x10000) {
		emu->ram[idx & emu->gt.rammask] = luaL_checkinteger(L, 3);
		return 0;
	}

	lua_pushvalue(L, 2);
	lua_rawget(L, lua_upvalueindex(1));
	if (!lua_isnil(L, -1)) {
		lua_pushvalue(L, 1);
		lua_pushvalue(L, 3);
		lua_call(L, 2, 0);
		return 0;
	}

	return luaL_error(L, "element is not writable");
}

#define makepropfn(prop, T) \
static int lgtemuget_##prop (lua_State *L) \
{ \
	lua_pushinteger(L, checkemu(L, 1)->gt.prop); \
	return 1; \
} \
static int lgtemuset_##prop (lua_State *L) \
{ \
	checkemu(L, 1)->gt.prop = (T)luaL_checkinteger(L, 2); \
	return 0; \
}

makepropfn(pc, unsigned short)
makepropfn(ir, unsigned char)
makepropfn(d, unsigned char)
makepropfn(ac, unsigned char)
makepropfn(x, unsigned char)
makepropfn(y, unsigned char)
makepropfn(out, unsigned char)
makepropfn(in, unsigned char)

#undef makegetpropfn

static int lgtemuget_vpc (lua_State *L)
{
	struct GTEmulationData *emu = checkemu(L, 1);
	lua_pushinteger(L, emu->ram[0x16] | (emu->ram[0x17] << 8));
	return 1;
}

static int lgtemuset_vpc (lua_State *L)
{
	struct GTEmulationData *emu = checkemu(L, 1);
	int value = luaL_checkinteger(L, 2);
	emu->ram[0x16] = value & 0xff;
	emu->ram[0x17] = (value >> 8) & 0xff;
	return 0;
}

static int lgtemuget_clock (lua_State *L)
{
	struct GTEmulationData *emu = checkemu(L, 1);
	lua_pushinteger(L, gtemu_getclock(&emu->ph));
	return 1;
}

static int lgtemuget_xout (lua_State *L)
{
	struct GTEmulationData *emu = checkemu(L, 1);
	lua_pushinteger(L, gtemu_getxout(&emu->ph));
	return 1;
}

static void pushkeysym(lua_State *L, SDL_KeyboardEvent *ev)
{
	lua_pushstring(L, SDL_GetKeyName(ev->keysym.sym));
	lua_pushinteger(L, ev->keysym.mod);
	lua_pushinteger(L, ev->keysym.scancode);
	lua_pushinteger(L, ev->keysym.sym);
}

static int lgtrunloop (lua_State *L)
{
	struct GTSDLState *s = (struct GTSDLState *)
		luaL_checkudata(L, 1, "gtemu.sdlstate");
	struct GTEmulationData *emu = (struct GTEmulationData *)
		luaL_checkudata(L, 2, "gtemu.emulation");
	int onkeydown = 0, onkeyup = 0, ontextinput = 0, breaksym = 7;

	if (lua_isnoneornil(L, 3)) {
		lua_settop(L, 7);
	} else {
		lua_settop(L, 3);
		luaL_checktype(L, 3, LUA_TTABLE);

		lua_getfield(L, 3, "onkeydown");
		onkeydown = lua_isnil(L, 4) ? 0 : 4;

		lua_getfield(L, 3, "onkeyup");
		onkeyup = lua_isnil(L, 5) ? 0 : 5;

		lua_getfield(L, 3, "ontextinput");
		ontextinput = lua_isnil(L, 6) ? 0 : 6;

		lua_pushliteral(L, "break");
	}

	for (;;) {
		SDL_Event ev;
		int checkres = 0;
		if (gtsdl_runuiframe(s, &emu->gt, &emu->ph, &ev) == 0) {
			continue;
		}
		if (ev.type == SDL_QUIT) {
			break;
		}
		if (ev.type == SDL_KEYDOWN && onkeydown != 0) {
			lua_pushvalue(L, onkeydown);
			pushkeysym(L, &ev.key);
			lua_call(L, 4, 1);
			checkres = 1;
		} else if (ev.type == SDL_KEYUP && onkeyup != 0) {
			lua_pushvalue(L, onkeyup);
			pushkeysym(L, &ev.key);
			lua_call(L, 4, 1);
			checkres = 1;
		} else if (ev.type == SDL_TEXTINPUT && ontextinput != 0) {
			lua_pushvalue(L, ontextinput);
			lua_pushstring(L, ev.text.text);
			lua_call(L, 1, 1);
			checkres = 1;
		}
		if (checkres) {
			if (lua_compare(L, -1, breaksym, LUA_OPEQ)) {
				lua_pop(L, 1);
				break;
			} else if (lua_toboolean(L, -1)) {
				lua_pop(L, 1);
				continue;
			} else {
				lua_pop(L, 1);
			}
		}
		if (ev.type == SDL_KEYDOWN &&
			ev.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {

			break;
		}
		gtsdl_handleevent(s, &emu->gt, &ev);
	}

	return 0;
}

static luaL_Reg lgtsdlstatemeta[] = {
	{"__gc", lgtclosewindow},
	{NULL, NULL}
};

static luaL_Reg lgtsdlstatefns[] = {
	{"runloop", lgtrunloop},
	{"close", lgtclosewindow},
	{NULL, NULL}
};

static luaL_Reg lgtemufns[] = {
	{"loadrom", lgtemuloadrom},
	{"processtick", lgtemuprocesstick},
	{"processscreen", lgtemuprocessscreen},
	{"sendgt1", lgtemusendgt1},
	{"createbuffer", lgtemucreatebuffer},
	{"getbuffer", lgtemugetbuffer},
	{"resetbuffer", lgtemuresetbuffer},
	{NULL, NULL}
};

static luaL_Reg lgtemugetfns[] = {
	{"pc", lgtemuget_pc},
	{"ir", lgtemuget_ir},
	{"d", lgtemuget_d},
	{"ac", lgtemuget_ac},
	{"x", lgtemuget_x},
	{"y", lgtemuget_y},
	{"out", lgtemuget_out},
	{"inp", lgtemuget_in}, /* "in" is a keyword in Lua */
	{"vpc", lgtemuget_vpc},
	{"clock", lgtemuget_clock},
	{"xout", lgtemuget_xout},
	{NULL, NULL}
};

static luaL_Reg lgtemusetfns[] = {
	{"pc", lgtemuset_pc},
	{"ir", lgtemuset_ir},
	{"d", lgtemuset_d},
	{"ac", lgtemuset_ac},
	{"x", lgtemuset_x},
	{"y", lgtemuset_y},
	{"out", lgtemuset_out},
	{"inp", lgtemuset_in}, /* "in" is a keyword in Lua */
	{"vpc", lgtemuset_vpc},
	{NULL, NULL}
};

static luaL_Reg lgtemulibfns[] = {
	{"initsdl", lgtinitsdl},
	{"openwindow", lgtopenwindow},
	{"newemulation", lgtnewemulation},
	{NULL, NULL}
};

int luaopen_gtemu (lua_State *L)
{
	luaL_newmetatable(L, "gtemu.sdlstate");
	luaL_setfuncs(L, lgtsdlstatemeta, 0);
	luaL_newlib(L, lgtsdlstatefns);
	lua_setfield(L, -2, "__index");
	lua_pop(L, 1);

	luaL_newmetatable(L, "gtemu.emulation");
	luaL_newlibtable(L, lgtemufns);
	lua_newtable(L); /* data for loader */
	lua_newtable(L); /* buffer for storage */
	luaL_setfuncs(L, lgtemufns, 2);
	luaL_newlib(L, lgtemugetfns);
	lua_pushcclosure(L, lgtemuindex, 2);
	lua_setfield(L, -2, "__index");

	luaL_newlib(L, lgtemusetfns);
	lua_pushcclosure(L, lgtemunewindex, 1);
	lua_setfield(L, -2, "__newindex");
	lua_pop(L, 1);


	luaL_newlibtable(L, lgtemulibfns);
	luaL_setfuncs(L, lgtemulibfns, 0);
	return 1;
}

