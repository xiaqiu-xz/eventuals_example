#include <string>
#include "eventuals/eventual.h"   // 引入 Eventual 类
#include "eventuals/promisify.h"  // 引入 Promisify 功能（可选，未使用）
#include <thread>
#include <iostream>
#include <unistd.h>        // sleep 函数
#include <glog/logging.h>  // CHECK_EQ 宏
using namespace eventuals;
int main(int argc, char** argv) {
    // 定义一个 lambda，用于创建 Eventual<std::string> 对象
    auto e = []() {
        return Eventual<std::string>([](auto& k) {
            // 创建一个新线程来模拟异步操作
            auto thread = std::thread([&k]() mutable {
                sleep(2);           // 模拟耗时操作，线程睡眠 2 秒
                k.Start("awake!");  // 异步操作完成，通知 eventuals pipeline
            });
            thread.detach();  // 分离线程，不阻塞主线程
            std::cout << "thread will sleep for 2 seconds ..." << std::endl;
        });
    };
    // 运行 Eventual 并阻塞等待结果
    // e() 返回 std::optional<std::string>，解引用得到值
    CHECK_EQ("awake!", *e());
    return 0;
}
