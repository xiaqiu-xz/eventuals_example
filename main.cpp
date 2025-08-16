#include "eventuals/eventual.h"  // 引入 Eventual 类，用于异步操作
#include "eventuals/promisify.h" // 引入 Promisify 功能（可选，当前示例未使用）
#include <glog/logging.h>        // 引入 Google glog 日志库，提供 CHECK_EQ 宏
#include <iostream>
#include <string>
#include <thread>   // std::thread
#include <unistd.h> // sleep 函数

using namespace eventuals;

int main(int argc, char **argv) {
  std::cout << "Starting program..." << std::endl;

  // 定义一个 Eventual 对象 e，返回 std::string 类型
  auto e = []() {
    // Eventual 构造函数接收一个 lambda，lambda 接收一个 Continuation k
    return Eventual<std::string>([](auto &k) {
      // 在新线程中异步执行
      auto thread = std::thread([&k]() mutable {
        sleep(2); // 模拟耗时操作，睡眠 2 秒
        std::cout << "Thread finished sleeping, calling k.Start()..."
                  << std::endl;
        k.Start("awake!"); // 触发 continuation，将结果传递出去
      });

      thread.detach(); // 分离线程，主线程无需 join
      std::cout << "thread will sleep for 2 seconds ..." << std::endl;
    });
  };

  std::cout << "Waiting for result..." << std::endl;

  // 执行 Eventual，获取结果（阻塞直到 k.Start() 被调用）
  auto result = *e();
  std::cout << "Got result: " << result << std::endl;

  // 检查返回值是否符合预期
  CHECK_EQ("awake!", result);
  std::cout << "CHECK_EQ passed!" << std::endl;

  return 0;
}
