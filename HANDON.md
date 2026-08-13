1.Connection.cpp  Connection.hpp:
Connection 用来管理一个客户端的 TCP socket 连接。它首先处于 READING 状态，通过 recv() 接收客户端发送的 HTTP 请求，并把数据保存到 buffer 中。当请求完整后，由 HttpRequest 解析请求，再交给 Router 处理，生成 HttpResponse。然后把 Response 转换成字符串放入输出 buffer，通过 send() 分批发送给客户端。发送完成后，Connection 进入 DONE 状态并最终关闭 socket。同时，它还通过 _lastActivity 检查连接是否超时。

                    一个客户端连接建立
                           │
                           ▼
              Connection::Connection()
                           │
                           ▼
                    _state = READING
                           │
                           ▼
              ┌──────────────────────┐
              │      READING         │
              │ 等待/接收 HTTP 请求   │
              └──────────┬───────────┘
                         │
                         │ toRead()
                         ▼
                       recv()
                         │
                         ▼
                    _requestReceived
                         │
                         │ 找到 "\r\n\r\n"
                         ▼
                  processRequest()
                         │
              ┌──────────┴──────────┐
              │                     │
              ▼                     ▼
        HttpRequest             Router
              │                     │
              └──────────┬──────────┘
                         │
                         ▼
                   HttpResponse
                         │
                         ▼
                 _responseToSend
                         │
                         ▼
                    _state = WRITING
                         │
                         ▼
              ┌──────────────────────┐
              │      WRITING         │
              │ 发送 HTTP Response    │
              └──────────┬───────────┘
                         │
                         │ toWrite()
                         ▼
                       send()
                         │
                         ▼
              _responseSent += bytesSent
                         │
                         │ 全部发送完？
                         ▼
                    _state = DONE
                         │
                         ▼
              ┌──────────────────────┐
              │        DONE          │
              │   可以关闭连接了       │
              └──────────┬───────────┘
                         │
                         ▼
                Connection::~Connection()
                         │
                         ▼
                     close(_fd)

isBodyTooLarge():
            HTTP Request
                  │
                  ↓
       isBodyTooLarge()
                  │
          ┌───────┴────────┐
          │                │
        false             true
          │                │
          ↓                ↓
     正常处理 POST    queueError(413,...)
                           │
                           ↓
                  _responseToSend
                           │
                           ↓
                    _state = WRITING
                           │
                           ↓
                         poll()
                           │
                           ↓
                       toWrite()
                           │
                           ↓
                         send()
                           │
                           ↓
                      客户端收到 413




2.server.cpp server.hpp
void Server::start()
{
    setupListeners();

    while (true)
    {
        buildPollSet();

        poll(...);

        dispatch();

        dropIdleConnections();
    }
}
3.router.cpp router.hpp
POST+Body:
bool	isPathSafe(const std::string& requestPath) const;

handleRequest():
                    HTTP Request
                         │
                         ↓
                ┌─────────────────┐
                │ path 安全吗？    │
                └─────────────────┘
                         │
                         ↓
                ┌─────────────────┐
                │ HTTP 版本支持吗？│
                └─────────────────┘
                         │
                         ↓
                ┌─────────────────┐
                │ method 支持吗？  │
                └─────────────────┘
                         │
                         ↓
                  找到 location
                         │
                         ↓
                ┌─────────────────┐
                │ 有 redirect 吗？ │
                └─────────────────┘
                         │
                         ↓
                ┌────────────────────┐
                │ location 允许这个  │
                │ method 吗？        │
                └────────────────────┘
                         │
                         ↓
                    根据 method 分流
                         │
              ┌──────────┼──────────┐
              ↓          ↓          ↓
             GET        POST      DELETE
              │          │          │
              ↓          ↓          ↓
         handleGet   handlePost  handleDelete

buildErrorResponse(404)
        │
        ↓
有没有配置 error_page 404？
        │
    ┌───┴────┐
    │        │
   有       没有
    │        │
    ↓        ↓
文件存在？  默认错误文本
    │
 ┌──┴──┐
 │     │
有    没有
 │     │
 ↓     ↓
自定义  默认
页面    页面

4.RouterStatic.cpp RouterStatic.hpp

handleGet()
    ↓
找到文件
    ↓
存在？
    ↓
返回文件内容


handleDelete()
    ↓
找到文件
    ↓
是不是目录？
    ↓
文件存在？
    ↓
删除文件
    ↓
成功 → 204

extractMultipart:
                 浏览器
                    │
                    │ POST /upload
                    │ multipart/form-data
                    ▼
             ┌───────────────┐
             │   HttpRequest │
             └───────┬───────┘
                     │
                     │ getBody()
                     ▼
        ┌───────────────────────────┐
        │ ------boundary            │
        │ Content-Disposition...    │
        │ filename="cat.png"        │
        │ Content-Type: image/png   │
        │                           │
        │ [cat.png 二进制数据]      │
        │                           │
        │ ------boundary--          │
        └─────────────┬─────────────┘
                      │
                      ▼
             extractMultipart()
                      │
              ┌───────┴────────┐
              │                │
              ▼                ▼
        fileName           fileData
        "cat.png"          二进制数据
              │                │
              └───────┬────────┘
                      │
                      ▼
                 writeFile()
                      │
                      ▼
          /upload_store/cat.png
                      │
                      ▼
                 201 Created



5.LocationConfig.cpp LocationConfig.hpp
LocationConfig::LocationConfig()
LocationConfig
│
├── _path             location 的路径
├── _root             网站文件根目录
├── _index            默认 index 文件
├── _allowedMethods   允许哪些 HTTP method
├── _autoindex        是否开启目录列表
├── _uploadStore      POST/upload 文件保存位置
└── _redirect         重定向地址

  _path("/"),
  _root("./www"),
  _index("index.html"),
  _allowedMethods("POST"),
  _autoindex(false),
  _uploadStore(""),
  _redirect("")
{}

