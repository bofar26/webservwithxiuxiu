#!/usr/bin/env python3
"""
webserv 压力测试脚本

用法:
    1. 先启动服务器: ./webserv conf/webserv.conf
    2. 运行: python3 tests/stress_test.py [并发数] [每个连接的请求数]

设计说明:
    subject 明确要求 "Stress test your server to ensure it remains available
    at all times", 这个脚本用多线程模拟多个客户端同时连接, 检查:
      1. 所有请求最终都能拿到响应(没有连接被无限期挂起)
      2. 压测过程中和压测结束后, 服务器都还活着、还能正常响应
"""

import socket
import sys
import threading
import time

HOST = "127.0.0.1"
PORT = 8080
TIMEOUT = 5

results_lock = threading.Lock()
success_count = 0
failure_count = 0


def worker(worker_id, requests_per_worker):
    global success_count, failure_count

    for i in range(requests_per_worker):
        try:
            with socket.create_connection((HOST, PORT), timeout=TIMEOUT) as sock:
                sock.sendall(b"GET / HTTP/1.1\r\nHost: localhost\r\n\r\n")
                sock.settimeout(TIMEOUT)
                data = sock.recv(4096)

            ok = data.startswith(b"HTTP/1.1 200") or data.startswith(b"HTTP/1.0 200")
            with results_lock:
                if ok:
                    global success_count
                    success_count += 1
                else:
                    global failure_count
                    failure_count += 1
        except (socket.timeout, ConnectionRefusedError, OSError):
            with results_lock:
                failure_count += 1


def check_server_still_alive():
    """压测结束后, 用一次干净的请求确认服务器还活着(没有崩溃/卡死)"""
    try:
        with socket.create_connection((HOST, PORT), timeout=TIMEOUT) as sock:
            sock.sendall(b"GET / HTTP/1.1\r\nHost: localhost\r\n\r\n")
            sock.settimeout(TIMEOUT)
            data = sock.recv(4096)
        return data.startswith(b"HTTP/1.1 200") or data.startswith(b"HTTP/1.0 200")
    except OSError:
        return False


def main():
    concurrency = int(sys.argv[1]) if len(sys.argv) > 1 else 20
    requests_per_worker = int(sys.argv[2]) if len(sys.argv) > 2 else 20
    total_requests = concurrency * requests_per_worker

    print(f"目标: {HOST}:{PORT}")
    print(f"并发数: {concurrency}, 每个并发发送 {requests_per_worker} 次请求, 共 {total_requests} 个请求")
    print("开始压测...")

    start = time.time()
    threads = []
    for w in range(concurrency):
        t = threading.Thread(target=worker, args=(w, requests_per_worker))
        threads.append(t)
        t.start()
    for t in threads:
        t.join()
    elapsed = time.time() - start

    print()
    print(f"耗时: {elapsed:.2f} 秒")
    print(f"成功: {success_count} / {total_requests}")
    print(f"失败: {failure_count} / {total_requests}")
    if elapsed > 0:
        print(f"吞吐量: {total_requests / elapsed:.1f} 请求/秒")

    print()
    print("压测后检查服务器是否还存活...")
    alive = check_server_still_alive()
    print("[PASS] 服务器压测后依然存活并正常响应" if alive else "[FAIL] 服务器压测后无响应, 可能已崩溃或卡死")

    if failure_count > 0 or not alive:
        sys.exit(1)


if __name__ == "__main__":
    main()
