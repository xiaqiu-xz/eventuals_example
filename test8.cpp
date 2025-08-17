#include <iostream>                // 标准输入输出库，用于 std::cout
#include <string>                  // 使用 std::string 处理字符串
#include <chrono>                  // 使用 std::chrono 模拟延时
#include <thread>                  // 使用 std::this_thread::sleep_for
#include <future>                  // 使用 std::future 处理异步结果
#include "eventuals/eventual.h"    // 引入 Eventual 类，定义异步操作
#include "eventuals/event-loop.h"  // 引入 EventLoop，管理异步任务
#include "eventuals/then.h"        // 引入 Then 组合器，处理异步结果
#include "eventuals/terminal.h"    // 引入 Terminal 组合器，手动处理回调
#include "eventuals/promisify.h"   // 引入 Promisify，将 Eventual 转换为 future
#include "eventuals/just.h"        // 引入 Just，将值注入异步管道
#include "eventuals/http.h"        // HTTP 请求模拟（假设库提供）
#include "eventuals/if.h"          // If 组合器，实现异步条件分支
using namespace eventuals;  // 使用 Eventuals 命名空间
// 模拟 HTTP 响应结构
struct HttpResponse {
    int code;          // HTTP 状态码
    std::string body;  // 响应内容
};
// 模拟异步 HTTP GET 请求
auto HttpGet(const std::string& url) {
    return Just(0) >> Then([url](int) -> HttpResponse {
               std::cout << "正在请求: " << url << std::endl;
               std::this_thread::sleep_for(std::chrono::milliseconds(500));  // 模拟网络延迟
               // 根据 URL 返回不同的模拟响应
               if (url.find("3rdparty.dev") != std::string::npos &&
                   url.find("www.") == std::string::npos) {
                   return HttpResponse{404, "Not Found"};  // 主服务器404
               } else if (url.find("www.3rdparty.dev") != std::string::npos) {
                   return HttpResponse{200, "Hello from backup server!"};  // 备用服务器200
               } else {
                   return HttpResponse{200, "Hello World!"};  // 默认成功
               }
           });
}
// 模拟异步处理函数，处理 HttpResponse 并返回字符串
auto SomeAsynchronousFunction(const HttpResponse& response) {
    return Just(response) >> Then([](const HttpResponse& resp) {
               std::cout << "异步处理响应，状态码: " << resp.code << std::endl;
               std::this_thread::sleep_for(std::chrono::milliseconds(300));  // 模拟处理延迟
               return "异步处理完成: " + resp.body;  // 返回处理后的字符串
           });
}
int main() {
    // 初始化默认事件循环，必须在异步操作前调用
    EventLoop::ConstructDefault();
    // --- 示例1: 基本 Just 用法 ---
    std::cout << "=== 示例1: Just - 将值注入到 pipeline ===\n" << std::endl;
    // 将一个字符串注入 Eventual pipeline，并使用 Then 处理
    auto [future1, k1] = Promisify(
        "just-example", Just(std::string("hello world")) >> Then([](const std::string& value) {
                            std::cout << "从 Just 获得值: " << value << std::endl;
                            return value + " - 处理完成";  // 返回新的字符串
                        }));
    k1.Start();                    // 启动 Eventual pipeline
    auto result1 = future1.get();  // 阻塞等待结果
    std::cout << "最终结果: " << result1 << "\n" << std::endl;
    // --- 示例2: Then 处理异步结果 ---
    std::cout << "=== 示例2: Then - 处理异步计算结果 ===\n" << std::endl;
    auto [future2, k2] =
        Promisify("then-async-example",
                  HttpGet("https://example.com") >> Then([](const HttpResponse& response) {
                      // 返回一个异步 Eventual
                      return SomeAsynchronousFunction(response);
                  }));
    k2.Start();                    // 启动异步任务
    auto result2 = future2.get();  // 等待结果
    std::cout << "异步处理结果: " << result2 << "\n" << std::endl;
    // --- 示例3: Then 返回同步值 ---
    std::cout << "=== 示例3: Then - 返回同步值 ===\n" << std::endl;
    auto [future3, k3] =
        Promisify("then-sync-example",
                  HttpGet("https://example.com") >> Then([](const HttpResponse& response) {
                      // 返回同步值，Eventuals 自动封装为 Eventual
                      bool success = response.code == 200;
                      std::cout << "请求是否成功: " << (success ? "是" : "否") << std::endl;
                      return success;
                  }));
    k3.Start();
    auto result3 = future3.get();
    std::cout << "同步处理结果: " << result3 << "\n" << std::endl;
    // --- 示例4: 使用 If 实现条件分支 ---
    std::cout << "=== 示例4: 使用 If 实现条件分支 ===\n" << std::endl;
    auto [future4, k4] =
        Promisify("conditional-example",
                  HttpGet("https://3rdparty.dev") >> Then([](const HttpResponse& response) -> auto{
                      std::cout << "第一次请求状态码: " << response.code << std::endl;
                      // 条件分支，如果第一次请求失败，则访问备用服务器
                      return If(response.code != 200)
                          .yes([]() {
                              return HttpGet("https://www.3rdparty.dev") >>
                                     Then([](const HttpResponse& backup_response) {
                                         return backup_response.body;  // 返回备用服务器内容
                                     });
                          })
                          .no([body = response.body]() {
                              return "Received HTTP Status OK with body: " + body;
                          });
                  }));
    k4.Start();
    auto result4 = future4.get();
    std::cout << "条件分支结果: " << result4 << "\n" << std::endl;
    // --- 示例5: 使用 Terminal() 手动处理回调 ---
    std::cout << "=== 示例5: 使用 Terminal() 手动处理回调 ===\n" << std::endl;
    auto e6 = HttpGet("https://example.com") >>
              Terminal()
                  .start([](const HttpResponse& response) {
                      std::cout << "Pipeline 成功! 状态码: " << response.code
                                << ", 内容: " << response.body << std::endl;
                  })
                  .fail([](auto&& error) { std::cout << "Pipeline 失败!" << std::endl; })
                  .stop([](auto&&) { std::cout << "Pipeline 停止!" << std::endl; });
    auto k6 = Build(std::move(e6));  // 构建 continuation
    k6.Start();                      // 启动 pipeline
    std::cout << "示例5 完成\n" << std::endl;
    // 清理事件循环
    EventLoop::DestructDefault();
    return 0;
}
