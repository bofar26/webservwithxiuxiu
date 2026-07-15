#!/usr/bin/env python3
"""
webserv 基础功能测试脚本

用法:
    1. 先在另一个终端启动服务器: ./webserv conf/webserv.conf
    2. 再运行本脚本:            python3 tests/test_basic.py

设计说明:
    用原始 socket 而不是 requests/http.client 库, 是因为我们还想测试
    "格式不对的请求"(malformed request)服务器会不会崩溃或者卡死,
    高层库通常不允许我们发送这种非法请求。
"""

import socket
import sys

HOST = "127.0.0.1"
PORT = 8080
TIMEOUT = 5  # 秒。 如果服务器卡住不响应, 测试不应该一直死等

passed = 0
failed = 0


def send_raw(request_bytes, read_bytes=65536):
    """建立一个新连接, 发送原始字节, 返回收到的原始响应字节。"""
    with socket.create_connection((HOST, PORT), timeout=TIMEOUT) as sock:
        sock.sendall(request_bytes)
        sock.settimeout(TIMEOUT)
        chunks = []
        try:
            while True:
                data = sock.recv(4096)
                if not data:
                    break
                chunks.append(data)
        except socket.timeout:
            # 服务器可能保持连接开放(keep-alive), 超时是正常现象,
            # 只要我们已经收到了数据就够用了
            pass
        return b"".join(chunks)


def get_status_code(response_bytes):
    """从响应里提取状态行的数字状态码, 例如 b'HTTP/1.1 404 Not Found...' -> 404"""
    if not response_bytes:
        return None
    first_line = response_bytes.split(b"\r\n", 1)[0]
    parts = first_line.split(b" ")
    if len(parts) < 2:
        return None
    try:
        return int(parts[1])
    except ValueError:
        return None


def check(name, condition, detail=""):
    global passed, failed
    if condition:
        passed += 1
        print(f"[PASS] {name}")
    else:
        failed += 1
        print(f"[FAIL] {name}  {detail}")


def test_get_existing_file():
    resp = send_raw(b"GET / HTTP/1.1\r\nHost: localhost\r\n\r\n")
    code = get_status_code(resp)
    check("GET / 返回 200", code == 200, f"实际返回 {code}")
    check("GET / 响应体包含预期内容", b"Ok computer" in resp, "响应体里没找到预期文字, 检查www/index.html是否还是这个内容")


def test_get_missing_file():
    resp = send_raw(b"GET /this-file-does-not-exist.html HTTP/1.1\r\nHost: localhost\r\n\r\n")
    code = get_status_code(resp)
    check("GET 不存在的文件返回 404", code == 404, f"实际返回 {code}")


def test_method_not_allowed():
    # PUT 不在 Router::isSupportedMethod() 允许的方法里, 应该返回405
    resp = send_raw(b"PUT /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n")
    code = get_status_code(resp)
    check("不支持的方法(PUT)返回 405", code == 405, f"实际返回 {code}")


def test_unsupported_http_version():
    resp = send_raw(b"GET / HTTP/2.0\r\nHost: localhost\r\n\r\n")
    code = get_status_code(resp)
    check("不支持的HTTP版本返回 505", code == 505, f"实际返回 {code}")


def test_malformed_request_does_not_crash():
    # 故意发送一个完全不合法的请求行, 服务器不应该崩溃或者返回200
    resp = send_raw(b"THIS IS NOT HTTP AT ALL\r\n\r\n")
    code = get_status_code(resp)
    check("畸形请求不会返回200(服务器正确拒绝而不是误处理)", code != 200, f"实际返回 {code}")

    # 再发一次正常请求, 确认服务器上一次的畸形请求没有把它搞挂
    resp2 = send_raw(b"GET / HTTP/1.1\r\nHost: localhost\r\n\r\n")
    check("畸形请求之后服务器依然存活并能正常响应", get_status_code(resp2) == 200)


def test_delete_nonexistent():
    resp = send_raw(b"DELETE /nope-does-not-exist.html HTTP/1.1\r\nHost: localhost\r\n\r\n")
    code = get_status_code(resp)
    check("DELETE 不存在的文件返回 404", code == 404, f"实际返回 {code}")


def test_response_has_content_length():
    resp = send_raw(b"GET / HTTP/1.1\r\nHost: localhost\r\n\r\n")
    check("响应包含 Content-Length header", b"Content-Length:" in resp)


def main():
    print(f"正在连接 {HOST}:{PORT} ...")
    try:
        socket.create_connection((HOST, PORT), timeout=TIMEOUT).close()
    except OSError as e:
        print(f"无法连接到服务器: {e}")
        print("请先在另一个终端启动服务器: ./webserv conf/webserv.conf")
        sys.exit(1)

    test_get_existing_file()
    test_get_missing_file()
    test_method_not_allowed()
    test_unsupported_http_version()
    test_malformed_request_does_not_crash()
    test_delete_nonexistent()
    test_response_has_content_length()

    print()
    print(f"共 {passed + failed} 个测试, 通过 {passed} 个, 失败 {failed} 个")
    sys.exit(0 if failed == 0 else 1)


if __name__ == "__main__":
    main()
