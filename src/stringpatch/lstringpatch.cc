#include <netdb.h>
#include <unistd.h>
#include <execinfo.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <ctype.h>
extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}
#ifndef __APPLE__ 
//#include <openssl/md5.h>
#include "md5.h"
#else
#include "md5.h"
#endif

static int lmd5(lua_State *L){
    if(lua_gettop(L) == 1 && lua_isstring(L, 1)){
        int i;
        size_t str_len;
        const unsigned char* str = (const unsigned char*)lua_tolstring(L, 1, &str_len);
        unsigned char md[16];
        char tmp[3]={'\0'},buf[33]={'\0'};
        MD5(str, str_len, md);
        for(i = 0; i < 16; i++){
            sprintf(tmp,"%2.2x",md[i]);
            strcat(buf,tmp);
        }
        lua_pushstring(L, buf);
        return 1;
    }else{
        lua_pushstring(L, "");
        return 1;
    }
}

static int lcap(lua_State *L){
    if (lua_isstring(L, 1)) {
        char dst[1024];
        size_t str_len;
        char *str = (char *)lua_tolstring(L, 1, &str_len);
        strcpy(dst, str);
        dst[0] = toupper(dst[0]);
        lua_pushlstring(L, dst, str_len);
        return 1;
    }
    return 0;
}

static int lsplit(lua_State *L){
    size_t i;
    if (lua_isstring(L, 1) && lua_isstring(L, 2)) {
        size_t str_len;
        size_t token_len;
        char *str = (char *)lua_tolstring(L, 1, &str_len);
        char *token = (char *)lua_tolstring(L, 2, &token_len);
        char *last_str = str;
        lua_newtable(L);
        int index = 1;
        for (i = 0; i < str_len; i++) {
            if (!strncmp(str + i, token, token_len)) {
                lua_pushinteger(L, index++); 
                lua_pushlstring(L, last_str, str + i - last_str);
                lua_settable(L, -3);
                i = i + token_len - 1;
                last_str = str + i + token_len;
            }
        }
        if (last_str == str) {
            lua_pushinteger(L, index);
            lua_pushstring(L, str);
            lua_settable(L, -3);
        } else {
            lua_pushinteger(L, index); 
            lua_pushstring(L, last_str);
            lua_settable(L, -3);
        }
        return 1;
    }
    return 0;
}

static luaL_Reg lua_lib[] ={
    {"split", lsplit},
    {"cap", lcap},
    {"md5", lmd5},
    {NULL, NULL}
};

extern "C" {
int luaopen_stringpatch(lua_State *L){
	luaL_register(L, "string", lua_lib);
	return 1;
}
}