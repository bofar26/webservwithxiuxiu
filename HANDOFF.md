# Webserv 交接说明：从这里接就可以

嗨，这份文档是给接手同学看的。先说人话版：  
我这一段主要做的是 **config 里的 `location` 解析，以及让 Router 真正根据 location 去找文件**。

现在项目已经不是最早那种：

```text
收到 /abc.html
→ 直接去 ./www/abc.html 找
```

而是变成了：

```text
收到 /public
→ 先看 config 里有没有 location /public
→ 有的话用这个 location 的 root/index
→ 最后去 ./www/public/index.html 找
```

这一步是后面 `allowed_methods`、`autoindex`、`error_page`、`CGI` 的地基。  
你接下来不用从零理解全部项目，可以先从这条线看起：

```text
ConfigParser
    ↓
ServerConfig / LocationConfig
    ↓
Server
    ↓
Router
    ↓
buildFilePath()
```

---

## 0. 当前分支

最新工作在：

```bash
config-parser
```

建议先从这个分支接：

```bash
git fetch origin
git checkout config-parser
git pull origin config-parser
```

如果本地没有这个分支：

```bash
git fetch origin
git checkout -b config-parser origin/config-parser
```

现在先不要删其他 branch，也先不要急着 merge 到 `main`。  
GitHub 上其他 branch 可以当作历史存档或其他人的工作线，当前交接以 `config-parser` 为准。

---

## 1. 当前项目大概完成到哪里了

已经完成的部分：

```text
✅ 基础 Server：socket / bind / listen / accept / recv / send
✅ HttpRequest：基础 request 解析
✅ HttpResponse：基础 response 组装
✅ Router：基础 GET 静态文件返回
✅ Router：DELETE 初版
✅ ConfigParser：能解析 server block
✅ ServerConfig：保存 listen/root/index
✅ LocationConfig：保存 location 的 path/root/index
✅ ConfigParser：能解析 location block
✅ ServerConfig：能保存多个 location
✅ Server：现在保存完整 ServerConfig
✅ Router：现在接收完整 ServerConfig
✅ Router：可以根据 request path 匹配 location
✅ Router：buildFilePath() 已支持 location root/index
```

还没完成的部分：

```text
⬜ allowed_methods
⬜ autoindex
⬜ error_page
⬜ client_max_body_size
⬜ POST body 完整处理
⬜ CGI
⬜ 多 server / server_name
⬜ non-blocking / poll 的完整稳定版本
⬜ 更多边界测试
⬜ 路径安全检查，例如 ../ 防逃逸
```

粗略估计：

```text
整个 webserv：约 40%～45%
静态文件 + config routing：约 60%～70%
```

---

## 2. 文件结构怎么看

当前结构大概是这样：

```text
webserv/
├── configs/
│   └── default.conf
│
├── include/
│   ├── ConfigParser.hpp
│   ├── HttpRequest.hpp
│   ├── HttpResponse.hpp
│   ├── LocationConfig.hpp
│   ├── Router.hpp
│   ├── Server.hpp
│   └── ServerConfig.hpp
│
├── src/
│   ├── config-parser/
│   │   ├── ConfigParser.cpp
│   │   ├── LocationConfig.cpp
│   │   └── ServerConfig.cpp
│   │
│   ├── http-request/
│   │   └── HttpRequest.cpp
│   │
│   ├── http-response/
│   │   └── ...
│   │
│   ├── router-static/
│   │   ├── Router.cpp
│   │   ├── RouterFileUtils.cpp
│   │   └── RouterStatic.cpp
│   │
│   └── server/
│       └── Server.cpp
│
├── www/
│   ├── index.html
│   ├── test1.html
│   ├── public/
│   │   └── index.html
│   └── images/
│       └── index.html
│
├── main.cpp
└── Makefile
```

各文件夹的意思：

```text
configs/             放配置文件
include/             放 header
src/config-parser/   config 解析和配置类
src/server/          网络连接层
src/http-request/    request 解析
src/http-response/   response 生成
src/router-static/   路由、静态文件、路径处理
www/                 测试用网页目录
```

---

## 3. 这几个类分别干什么

### 3.1 ConfigParser

负责读配置文件，例如：

```nginx
server {
    listen 8080;
    root ./www;
    index index.html;

    location /public {
        root ./www/public;
        index index.html;
    }
}
```

它的任务是把这份文本 config 解析成 C++ 对象：

```text
ServerConfig
    port = 8080
    root = ./www
    index = index.html
    locations = [...]
```

可以把它理解成：

```text
ConfigParser = 读配置文件的人
```

---

### 3.2 ServerConfig

保存一个 server block 的配置。

目前大概有：

```cpp
int _port;
std::string _root;
std::string _index;
std::vector<LocationConfig> _locations;
```

意思是：

```text
_port       监听端口
_root       server 默认 root
_index      server 默认 index
_locations 这个 server 下面所有 location
```

非常重要：  
`ServerConfig::operator=` 里必须拷贝 `_locations`。

应该有：

```cpp
_locations = other._locations;
```

如果忘了这一句，config 在传递时 location 会全部丢掉。

---

### 3.3 LocationConfig

保存一个 location block 的配置。

目前有：

```cpp
std::string _path;
std::string _root;
std::string _index;
```

例如 config：

```nginx
location /public {
    root ./www/public;
    index index.html;
}
```

对应：

```text
_path  = /public
_root  = ./www/public
_index = index.html
```

可以把它理解成：

```text
LocationConfig = 某个 URL 前缀对应的一套规则
```

---

### 3.4 Server

Server 负责网络层：

```text
socket
bind
listen
accept
recv
send
close
```

Server 不应该负责判断：

```text
/public 应该去哪找文件？
这个 method 允不允许？
这个目录能不能 autoindex？
```

这些都应该交给 Router。

现在 Server 里应该保存完整配置：

```cpp
ServerConfig _config;
```

构造函数应该类似：

```cpp
Server::Server(const ServerConfig& config)
    : _config(config), _serverFd(-1)
{
}
```

---

### 3.5 Router

Router 是目前最关键的业务层。

它的任务是：

```text
拿 request
拿 config
根据 config 的规则决定怎么响应 request
```

也就是说：

```text
HttpRequest 只告诉你：
    method = GET
    path = /public

ServerConfig 告诉你：
    /public 对应 ./www/public

Router 把两者合起来：
    最终应该返回 ./www/public/index.html
```

Router 现在也保存完整配置：

```cpp
ServerConfig _config;
```

构造函数应该类似：

```cpp
Router::Router(const ServerConfig& config)
    : _config(config)
{
}
```

---

## 4. Config、Request、Router 的关系

这是整个项目后面最容易乱的地方，所以这里多说几句。

### Config 是规则

比如：

```nginx
location /public {
    root ./www/public;
    index index.html;
}
```

它表示：

```text
URL 以 /public 开头的时候
真实文件从 ./www/public 下面找
如果访问的是目录首页
就找 index.html
```

### Request 是这一次客户端问了什么

比如：

```http
GET /public HTTP/1.1
```

HttpRequest 解析出来：

```text
method = GET
path = /public
```

### Router 是做决定的人

Router 做：

```text
request path = /public
    ↓
去 config 里找 location
    ↓
找到 location /public
    ↓
使用 root ./www/public
    ↓
/public 这个 path 去掉 /public 前缀后剩下空
    ↓
空路径当成 /
    ↓
返回 ./www/public/index.html
```

一句话：

```text
Config 是地图。
Request 是客户要去的地方。
Router 是拿着地图带路的人。
```

---

## 5. 当前 config 应该长什么样

当前测试 config 建议保持这样：

```nginx
server {
    listen 8080;
    root ./www;
    index index.html;

    location /public {
        root ./www/public;
        index index.html;
    }

    location /images {
        root ./www/images;
        index index.html;
    }
}
```

对应文件结构：

```text
www/
├── index.html
├── test1.html
├── public/
│   └── index.html
└── images/
    └── index.html
```

映射关系：

```text
/              -> ./www/index.html
/test1.html    -> ./www/test1.html

/public        -> ./www/public/index.html
/public/       -> ./www/public/index.html

/images        -> ./www/images/index.html
/images/       -> ./www/images/index.html
```

注意：  
`location /public` 里面的 `root` 写的是真实文件夹位置。  
因为真实文件在 `./www/public/index.html`，所以 root 要写：

```nginx
root ./www/public;
```

如果写成：

```nginx
root ./public;
```

那 Router 会去找：

```text
./public/index.html
```

如果这个文件不存在，就会 404。

---

## 6. 这次工作实际做了什么

这一段是交接重点。

原来 Router 的路径逻辑更简单，大概类似：

```cpp
if (requestPath == "/")
    return (root + "/" + index);
return (root + requestPath);
```

也就是说：

```text
收到 /public
直接拼成 ./www/public
```

这个时候它不会知道：

```text
/public 其实在 config 里有单独的 location
应该去 ./www/public/index.html
```

现在改成了：

```text
先 findLocation(requestPath)
如果找到 location，就用 location 的 root/index
如果没找到 location，就用 server 默认 root/index
```

核心变化是：

```text
Router 终于开始真正使用 config 了
```

---

## 7. 当前 Router 的两个核心函数

### 7.1 findLocation()

当前版本大概是：

```cpp
const LocationConfig* Router::findLocation(const std::string& path) const
{
    const std::vector<LocationConfig>& config_locations = _config.getLocation();

    for (size_t i = 0; i < config_locations.size(); i++)
    {
        if (path.compare(0,
                         config_locations[i].getPath().length(),
                         config_locations[i].getPath()) == 0)
            return (&config_locations[i]);
    }
    return (NULL);
}
```

它的意思是：

```text
遍历所有 location
如果 request path 是以 location path 开头
就返回这个 location
如果都没有匹配
返回 NULL
```

例子：

```text
path = /public/a.html
location path = /public
```

这句：

```cpp
path.compare(0, locationPath.length(), locationPath) == 0
```

意思是：

```text
path 的开头是不是 locationPath
```

也就是：

```text
/public/a.html 的前面是不是 /public
```

是，所以匹配。

为什么返回指针？

```cpp
const LocationConfig*
```

因为可能找不到。  
找不到的时候可以：

```cpp
return (NULL);
```

如果返回引用，就不方便表示“没找到”。

---

### 7.2 buildFilePath()

当前版本大概是：

```cpp
std::string Router::buildFilePath(const std::string& requestPath) const
{
    std::string root;
    std::string index;
    const LocationConfig* location;
    std::string relativePath;

    root = _config.getRoot();
    index = _config.getIndex();
    location = findLocation(requestPath);
    relativePath = requestPath;

    if (location != NULL)
    {
        if (location->getRoot() != "")
            root = location->getRoot();
        if (location->getIndex() != "")
            index = location->getIndex();

        relativePath = relativePath.substr(location->getPath().length());
        if (relativePath == "")
            relativePath = "/";
    }

    if (relativePath == "/")
        return (root + "/" + index);
    return (root + relativePath);
}
```

它的逻辑是：

```text
先默认使用 server root/index

如果匹配到了 location：
    root 改成 location root
    index 改成 location index
    relativePath 去掉 location path 前缀

最后：
    如果 relativePath 是 /
        返回 root/index
    否则
        返回 root + relativePath
```

例子 1：

```text
requestPath = /public
location path = /public
root = ./www/public
```

去掉 `/public` 后：

```text
relativePath = ""
```

空字符串改成：

```text
relativePath = /
```

最终：

```text
./www/public/index.html
```

例子 2：

```text
requestPath = /public/a.html
location path = /public
root = ./www/public
```

去掉 `/public` 后：

```text
relativePath = /a.html
```

最终：

```text
./www/public/a.html
```

---

## 8. 当前完整请求流程

以：

```bash
curl -i http://localhost:8080/public
```

为例。

流程：

```text
Client 发请求
    ↓
Server::handleClient()
    ↓
recv raw request
    ↓
HttpRequest request(rawRequest)
    ↓
request.getPath() = /public
    ↓
Router router(_config)
    ↓
Router::handleRequest(request)
    ↓
Router::handleGet(request)
    ↓
buildFilePath("/public")
    ↓
findLocation("/public")
    ↓
匹配到 location /public
    ↓
root = ./www/public
index = index.html
relativePath = ""
relativePath 改成 "/"
    ↓
filePath = ./www/public/index.html
    ↓
fileExists(filePath)
    ↓
buildFileResponse(filePath)
    ↓
Server send response
```

流程图：

```text
┌────────────────────────┐
│ Client: GET /public     │
└───────────┬────────────┘
            │
            ▼
┌────────────────────────┐
│ Server::handleClient    │
│ recv raw request        │
└───────────┬────────────┘
            │
            ▼
┌────────────────────────┐
│ HttpRequest             │
│ path = /public          │
└───────────┬────────────┘
            │
            ▼
┌────────────────────────┐
│ Router(_config)         │
└───────────┬────────────┘
            │
            ▼
┌────────────────────────┐
│ findLocation(/public)   │
└───────────┬────────────┘
            │
            ▼
┌────────────────────────┐
│ match location /public  │
│ root = ./www/public     │
└───────────┬────────────┘
            │
            ▼
┌────────────────────────┐
│ buildFilePath           │
│ ./www/public/index.html │
└───────────┬────────────┘
            │
            ▼
┌────────────────────────┐
│ fileExists              │
└───────────┬────────────┘
            │
            ▼
┌────────────────────────┐
│ 200 OK                  │
└────────────────────────┘
```

---

## 9. 一个非常重要的坑：不要传空 config

这次调试时踩过一个很关键的坑。

错误写法：

```cpp
void Server::handleClient(int clientFd)
{
    std::string rawRequest;
    ServerConfig config;  // 这里新建了一个空 config

    ...

    HttpRequest request(rawRequest);
    Router router(config); // 把空 config 传给 Router
}
```

这个会导致：

```text
Router 里的 location count = 0
findLocation() 永远找不到 location
/public 会被当作普通路径
filePath 会变成 ./www/public
```

正确写法：

```cpp
HttpRequest request(rawRequest);
Router router(_config);
response = router.handleRequest(request);
```

因为 Server 自己已经保存了从 main 传进来的完整 `_config`。  
`handleClient()` 里面不要再创建一个新的空 `ServerConfig`。

记住这一句就行：

```cpp
Router router(_config);
```

不要写：

```cpp
ServerConfig config;
Router router(config);
```

---

## 10. 当前怎么测试

先编译：

```bash
make clean
make
```

启动：

```bash
./webserv
```

另开一个终端测试：

```bash
curl -i http://localhost:8080/
curl -i http://localhost:8080/public
curl -i http://localhost:8080/public/
curl -i http://localhost:8080/images
curl -i http://localhost:8080/images/
curl -i http://localhost:8080/test1.html
curl -i http://localhost:8080/public/notexist.html
```

预期：

```text
/                         -> ./www/index.html
/public                   -> ./www/public/index.html
/public/                  -> ./www/public/index.html
/images                   -> ./www/images/index.html
/images/                  -> ./www/images/index.html
/test1.html               -> ./www/test1.html
/public/notexist.html     -> 404 Not Found
```

---

## 11. 如果测试失败，先看这几个地方

### 11.1 `/public` 返回 404

先检查 config：

```nginx
location /public {
    root ./www/public;
    index index.html;
}
```

再检查文件是否存在：

```bash
ls -l ./www/public/index.html
```

### 11.2 filePath 变成 `./www/public`

这通常说明没有匹配到 location，Router 走了 server 默认 root。

检查：

```cpp
Router router(_config);
```

不要传空 config。

### 11.3 location count 是 0

说明 config 传到 Router 的时候丢了。

重点检查：

```cpp
ServerConfig::operator=
```

里面是否有：

```cpp
_locations = other._locations;
```

也检查 Server 和 Router 构造函数是否真的用了：

```cpp
_config(config)
```

而不是：

```cpp
_config()
```

---

## 12. 接下来最建议做什么

下一步最建议做：

```text
allowed_methods
```

因为现在 Router 已经能知道请求属于哪个 location 了。  
接下来就应该判断：

```text
这个 location 允不允许这个 method？
```

目标 config：

```nginx
location /public {
    root ./www/public;
    index index.html;
    allowed_methods GET;
}

location /upload {
    root ./www/uploads;
    index index.html;
    allowed_methods GET POST DELETE;
}
```

目标行为：

```text
GET /public        -> 允许
DELETE /public     -> 405 Method Not Allowed
POST /upload       -> 允许
PUT /upload        -> 405 Method Not Allowed
```

---

## 13. allowed_methods 开发路线

### Step 1：LocationConfig 加成员

```cpp
std::vector<std::string> _allowedMethods;
```

加 getter/setter：

```cpp
void setAllowedMethods(const std::vector<std::string>& methods);
const std::vector<std::string>& getAllowedMethods() const;
```

也可以加一个辅助函数：

```cpp
bool isMethodAllowed(const std::string& method) const;
```

---

### Step 2：ConfigParser 解析

支持这种配置：

```nginx
allowed_methods GET POST DELETE;
```

解析思路：

```text
遇到 allowed_methods
    ↓
一直读 token，直到 ;
    ↓
把 GET / POST / DELETE 放进 vector
    ↓
location.setAllowedMethods(methods)
```

这个 directive 建议先只放在 location block 里做。

---

### Step 3：Router 检查 method

Router 里在处理 GET/DELETE/POST 前，先判断 method 是否被允许。

大概是：

```cpp
const LocationConfig* location = findLocation(request.getPath());

if (location != NULL && !location->isMethodAllowed(request.getMethod()))
    return (buildTextResponse(405, "Method Not Allowed\n"));
```

默认规则可以先简单一点：  
如果 location 没有写 allowed_methods，可以默认允许当前已经实现的 method，比如：

```text
GET
DELETE
```

具体按你们后续设计定。

---

## 14. allowed_methods 之后的路线

建议顺序：

```text
1. allowed_methods
2. directory/index 更完整处理
3. autoindex
4. error_page
5. client_max_body_size
6. POST body
7. CGI
8. 多 server / server_name
9. non-blocking / poll 完善
10. 大量测试
```

为什么不要马上做 CGI：

```text
CGI 依赖 method、body、config、location 都比较稳定。
如果前面这些没稳，CGI 会非常难调。
```

---

## 15. 交接前检查清单

交接前建议确认：

```text
[ ] 删除临时 debug 输出
[ ] make clean && make 成功
[ ] ./webserv 能启动
[ ] curl / 成功
[ ] curl /public 成功
[ ] curl /public/ 成功
[ ] curl /images 成功
[ ] curl /images/ 成功
[ ] curl /test1.html 成功
[ ] curl /public/notexist.html 返回 404
[ ] configs/default.conf 和 www/ 文件结构匹配
[ ] Server::handleClient 里使用 Router router(_config)
[ ] ServerConfig::operator= 复制 _locations
```

---

## 16. 建议提交

确认没问题后：

```bash
git status
git add .
git commit -m "Add location config parsing and routing"
git push origin config-parser
```

如果单独提交这份文档：

```bash
git add HANDOFF.md
git commit -m "Add handoff notes"
git push origin config-parser
```

---

## 17. 最后一句话

当前项目最重要的进展是：

```text
Router 已经开始真正根据 config 做路由，而不是单纯按 request path 拼文件。
```

后面所有核心功能都会接在这里：

```text
location matching
    ↓
allowed_methods
    ↓
autoindex
    ↓
error_page
    ↓
client_max_body_size
    ↓
CGI
```

所以接下来从 `allowed_methods` 接，是最顺的。
