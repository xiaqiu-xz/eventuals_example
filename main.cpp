#include "eventuals/eventual.h"  // 引入 Eventual 类
#include "eventuals/promisify.h" // 引入 Promisify 功能（可选，未使用）
#include <glog/logging.h>        // CHECK_EQ 宏
#include <iostream>
#include <string>
#include <thread>
#include <unistd.h> // sleep 函数
using namespace eventuals;
int main(int argc, char **argv) {
  std::cout << "Starting program..." << std::endl;

  auto e = []() {
    return Eventual<std::string>([](auto &k) {
      auto thread = std::thread([&k]() mutable {
        sleep(2);
        std::cout << "Thread finished sleeping, calling k.Start()..."
                  << std::endl;
        k.Start("awake!");
      });
      thread.detach();
      std::cout << "thread will sleep for 2 seconds ..." << std::endl;
    });
  };

  std::cout << "Waiting for result..." << std::endl;
  auto result = *e();
  std::cout << "Got result: " << result << std::endl;

  CHECK_EQ("awake!", result);
  std::cout << "CHECK_EQ passed!" << std::endl;

  return 0;
}