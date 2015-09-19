#include <netdb.h>
#include <execinfo.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ifaddrs.h>

#define EARG 1

static int lfork(lua_State *L){
	int ir = fork();
    lua_pushinteger(L, ir);
    return 1;
}

static int lgetpid(lua_State *L){
	int ir = getpid();
    lua_pushinteger(L, ir);
    return 1;
}

static int lsetsid(lua_State *L){
	int ir = setsid();
    lua_pushinteger(L, ir);
    return 1;
}

static int ltime(lua_State *L){
    int t = time(NULL);
    lua_pushinteger(L, t);
    return 1;
}

static int lgettimeofday(lua_State *L){
    struct timeval tv; 
    struct timezone tz; 
    gettimeofday(&tv,&tz);
    lua_pushinteger(L,tv.tv_sec);
    lua_pushinteger(L,tv.tv_usec);
    return 2;
}

static int lgetcwd(lua_State *L){
    char buf[128];
    buf[0] = 0;
	getcwd(buf, sizeof(buf));
    lua_pushstring(L, buf); 
    return 1;
}

static int lchdir(lua_State *L){
	const char *dir = (const char *)lua_tostring(L, 1);
	int error = chdir(dir);
    lua_pushinteger(L, error);
    return 1;
}

static int lnanosleep(lua_State *L){
	if (lua_gettop(L) == 1 && lua_isnumber(L, 1)){
		int t = (int)lua_tonumber(L, 1);
        int sec = t / 1000;
        int nsec = (t % 1000) * 1000000;
        struct timespec slptm;
        slptm.tv_sec = sec;
        slptm.tv_nsec = nsec;
        nanosleep(&slptm, NULL);
        lua_pushboolean(L, true);
        return 1;
	}
    lua_pushboolean(L, false);
    return 1;
}

static int lsleep(lua_State *L){
    int seconds;
    int error;
    seconds = (int)lua_tointeger(L, 1);
    error = sleep(t);
    lua_pushinteger(L, error);
    return 1;
}

static int lwaitpid(lua_State *L){
    int status;
    pid_t pid;
    int options;
    int error;
    pid = (int)lua_tointeger(L, 1);
    status = (int)lua_tointeger(L, 2);
    options = (int)lua_tointeger(L, 3);
    pid = waitpid(pid, &status, options);
    lua_pushinteger(L, pid);
    lua_pushinteger(L, status);
    return 2;
}

static int lmkdirs(lua_State *L){
	if (lua_gettop(L) == 1 && lua_isstring(L, 1)){
        size_t dir_len = 0;
		char *dir = (char *)lua_tolstring(L, 1, &dir_len);
        for(int i = 0; i < dir_len; i++){
            if(dir[i] == '/'){
                char c = dir[i];
                dir[i] = 0;
                if(access(dir, 0)){
                    mkdir(dir, 0777);    
                }
                dir[i] = c;
            }
        }
        if(access(dir, 0)){
            mkdir(dir, 0777);    
        }
        lua_pushboolean(L, true);
        return 1;
	}
    lua_pushboolean(L, false);
    return 1;
}

static int lmkdir(lua_State *L){
	if (lua_gettop(L) == 1 && lua_isstring(L, 1)){
		const char *dir = (const char *)lua_tostring(L, 1);
        if(mkdir(dir, 0777)){
            lua_pushboolean(L, false);
        }else{
            lua_pushboolean(L, true);
        }
        return 1;
	}
    lua_pushboolean(L, false);
    return 1;
}

static int lexists(lua_State *L){
    int amode;
	const char *dir;
    int error;
    dir = (const char *)lua_tostring(L, 1);
    amode = (int)lua_tointeger(L, 2);
    error = access(dir, amode);
    lua_pushinteger(L, error);
    return 1;
}

static int lclose(lua_State *L){
    int error;
    int fd;
    fd  = (int)lua_tointeger(L, 1);
    error = close(fd);
    lua_pushinteger(L, error);
    return 1;
}

static int lremove(lua_State *L){
    int error;
	const char *filepath = (const char *)lua_tostring(L, 1);
    remove(filepath);
    lua_pushinteger(L, 1);
    return 1;
}

static int lrename(lua_State *L){
    int error;
	const char *src = (const char *)lua_tostring(L, 1);
	const char *dst = (const char *)lua_tostring(L, 2);
    error = rename(src, dst);
    lua_pushinteger(L, error);
    return 1;
}
static int llistdir(lua_State *L){
	if (lua_gettop(L) == 1 && lua_isstring(L, 1)){
		const char *dir_name = (const char *)lua_tostring(L, 1);
        struct dirent *ent;
        DIR *dir = opendir(dir_name);
        lua_newtable(L);
        int idx = 1;
        if(dir == NULL){
            return 1;
        }
        while((ent = readdir(dir)) != NULL){
            if(ent->d_type & DT_DIR || ent->d_type & DT_REG){
                if(strcmp(ent->d_name, ".") == 0){
                    continue;
                }
                if(strcmp(ent->d_name, "..") == 0){
                    continue;
                }
                lua_pushnumber(L, idx++);

                lua_newtable(L);
                lua_pushstring(L, "type");
                if(ent->d_type & DT_DIR){
                    lua_pushstring(L, "dir");
                }
                else{
                    lua_pushstring(L, "file");
                }
                lua_settable(L, -3);

                lua_pushstring(L, "name");
                lua_pushstring(L, ent->d_name);
                lua_settable(L, -3);

                lua_settable(L, -3);
            }
        }
        closedir(dir);
        return 1;
	}
    lua_pushboolean(L, false);
    return 1;
}

static int lstrftime(lua_State *L){
    if(lua_gettop(L) == 2 && lua_isstring(L, 1) && lua_isnumber(L, 2)){
        const char *format = (const char *)lua_tostring(L, 1);
        time_t time = (time_t)lua_tonumber(L, 2);
        struct tm *tm;
        tm = localtime(&time);
        char str[32];
        if(strftime(str, sizeof(str), format, tm)){
        }
        lua_pushstring(L, str);
        return 1;
    }
    return 0;
}
static int lmd5(lua_State *L){
    if(lua_gettop(L) == 1 && lua_isstring(L, 1)){
        size_t str_len;
        const unsigned char* str = (const unsigned char*)lua_tolstring(L, 1, &str_len);
        unsigned char md[16];
        char tmp[3]={'\0'},buf[33]={'\0'};
        MD5(str, str_len, md);
        for(int i = 0; i < 16; i++){
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

//短网址
static int lshorturl(lua_State *L){
    if(lua_gettop(L) == 1 && lua_isstring(L, 1)){
        char* origstring = (char*)lua_tostring(L, 1);
        string* str = shorturl(origstring, strlen(origstring));
        lua_pushstring(L, str[0].c_str());
        delete []str;
        return 1;
    }else{
        lua_pushstring(L, "");
        return 1;
    }
}
#define NON_NUM '0'
static int hex2num(char c){
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'z') return c - 'a' + 10;
    if (c >= 'A' && c <= 'Z') return c - 'A' + 10;
    return NON_NUM;
}

static int urldecode(const char *str, const int str_len, char *result, const int result_len){
    char ch, ch1, ch2;
    int i;
    int j = 0;
    if ((str == NULL) || (result == NULL) || (str_len <= 0) || (result_len <= 0)) {
        return 0;
    }
    for (i = 0; i < str_len && j < result_len; ++i) {
        ch = str[i];
        switch (ch) {
            case '+':
                result[j++] = ' ';
                break;
            case '%':
                if (i + 2 < str_len) {
                    ch1 = hex2num(str[i + 1]);
                    ch2 = hex2num(str[i + 2]);
                    if ((ch1 != NON_NUM) && (ch2 != NON_NUM)) {
                        result[j++] = (char)((ch1 << 4) | ch2);
                    }
                    i += 2;
                    break;
                } else {
                    break;
                }
            default:
                result[j++] = ch;
                break;
        }
    }
    result[j] = 0;
    return j;
}
static int urlencode(const char *str, const int str_len, char *result, const int result_len){
    int i;
    int j = 0;
    char ch;

    if ((str == NULL) || (result == NULL) || (str_len <= 0) || (result_len <= 0)) {
        return 0;
    }
    for (i = 0; i < str_len && j < result_len; ++i) {
        ch = str[i];
        if (((ch >= 'A') && (ch < 'Z')) ||
            ((ch >= 'a') && (ch < 'z')) ||
            ((ch >= '0') && (ch < '9'))) {
                result[j++] = ch;
         } else if (ch == ' '){
            result[j++] = '+';
         } else if (ch == '.' || ch == '-' || ch == '_' || ch == '*') {
            result[j++] = ch;
         } else {
            if (j + 3 < result_len) {
                sprintf(result + j, "%%%02X", (unsigned char)ch);
                j += 3;
            } else {
                return 0;
            }
         }
    }
    result[j] = '\0';
    return j;
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
    int domain;
    int type;
    int protocol;
    domain = (int)lua_tointeger(L, 1);
    type = (int)lua_tointeger(L, 2);
    protocol = (int)lua_tointeger(L, 3);
    domain = (int)lua_tointeger(L, 1);
    sockfd = socket(domain, type, protocol);
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
    {"fork", lfork},
    {"daemon", ldaemon},
    {"getpid", lgetpid},
    {"setsid", lsetsid},
    {"chdir", lchdir},
    {"getcwd", lgetcwd},
    {"stdout", lstdout},
    {"listdir", llistdir},
    {"mkdir", lmkdir},
    {"mkdirs", lmkdirs},
    {"exists", lexists},
    {"remove", lremove},
    {"close", lclose},
    {"rename", lrename},
    {"time", ltimenow},
    {"waitpid", lwaitpid},
    {"sleep", lsleep},
    {"nanosleep", lnanosleep},
    {"gettimeofday", gettimeofday},
    {"strftime", lstrftime},
    {NULL, NULL}
};

int luaopen_os(lua_State *L){
	luaL_register(L, "os", (luaL_Reg*)lua_lib);
	return 1;
}
