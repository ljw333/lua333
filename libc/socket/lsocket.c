#include <netdb.h>
#include <execinfo.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <fcntl.h>

static int lsetnonblock(lua_State *L){
    int sockfd;
    int error;
    sockfd = (int)lua_tonumber(L, 1);
    error = fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
    lua_pushinteger(L, 1);
    return error;
}

static int lclose(lua_State *L){
    int sockfd;
    int error;
    sockfd = (int)lua_tonumber(L, 1);
    error = close(sockfd);
    lua_pushinteger(L, error);
    return 1;
}

static int lsocket(lua_State *L){
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    lua_pushinteger(L, sockfd);
    return 1;
}

static int llisten(lua_State *L){
    int sockfd;
    char *host;
    unsigned short port;
    int error;
    sockfd = (int)lua_tointeger(L, 1);
    host = (char *)lua_tostring(L, 2);
    port = (unsigned short)lua_tointeger(L, 3);
    lua_pushinteger(L, 1);
    return 1;
}

static int lsend(lua_State *L){
    size_t len;
    int error;
    char *str;
    int sockfd;
    sockfd = (int)lua_tonumber(L, 1);
    str = (char *)lua_tolstring(L, 2, &len);
    error = send(sockfd, str, len, 0);
    lua_pushinteger(L, error);
    return 1;
}

static luaL_Reg lua_lib[] ={
    {"socket", lsocket},
    {"listen", llisten},
    {"recv", lrecv},
    {"send", lsend},
    {"close", lclose},
    {"setnonblock", lsetnonblock},
    {NULL, NULL}
};

int luaopen_os(lua_State *L){
	luaL_register(L, "Date", (luaL_Reg*)lua_lib);
	return 1;
}

