#include <iostream>
#include "eventuals/eventual.h"
#include "eventuals/just.h"
#include "eventuals/then.h"        // 需要引入 Then 组合器，用于处理异步结果
#include "eventuals/http.h"        // HTTP GET 请求支持
#include "eventuals/if.h"          // 异步条件分支 If
#include "eventuals/event-loop.h"  // 事件循环管理异步任务
#include "eventuals/terminal.h"    // Terminal 组合器，用于处理最终结果回调
#include "eventuals/promisify.h"   // 将 Eventual 转换为 std::future
#include <chrono>
#include <future>  // std::future
using namespace eventuals;
// ---------------------------
// HTTP GET 封装函数
// ---------------------------
auto GetBody(const std::string& uri) {
    return http::Get("https://example.com/") >> Then([](http::Response && response) -> auto{
               auto code = response.code();             // 获取 HTTP 状态码
               auto body = std::move(response.body());  // 获取响应内容并移动
               // 异步条件分支：根据状态码判断
               return If(code == 200)
                   .yes([body = std::move(body)]() {  // 状态码 200
                       std::cout << "yes" << std::endl;
                       return Just(std::move(body));  // 返回 body
                   })
                   .no([code]() {  // 非 200 状态码
                       std::cout << "no" << std::endl;
                       // 返回统一的 Eventual 类型，而不是直接返回字符串
                       return Just(std::string{"HTTP GET failed w/ code "} + std::to_string(code));
                   });
           });
}
// ---------------------------
// 帮助函数：轮询直到条件满足
// ---------------------------
void RunUntil(const std::function<bool()>& condition) {
    while (!condition()) {
        EventLoop::Default().RunUntilIdle();  // 处理事件循环中所有任务
    }
}
// 模板版本：轮询直到 future 完成
template <typename T>
void RunUntil(const std::future<T>& future) {
    return RunUntil([&future]() {
        auto status = future.wait_for(std::chrono::nanoseconds::zero());
        return status == std::future_status::ready;  // future 已就绪
    });
}
// ---------------------------
// 主函数
// ---------------------------
int main() {
    bool done = false;  // 标志异步任务是否完成
    // 构造默认事件循环
    EventLoop::ConstructDefault();
    std::cout << "开始HTTP请求..." << std::endl;
    // 创建 Eventual 管道并附加 Terminal 回调
    auto e = GetBody("https://www.3rdparty.dev") >>
             Terminal()
                 .start([&done](auto&& result) {  // 成功回调
                     std::cout << " Get SUCCESS:\n" << result << std::endl;
                     done = true;
                 })
                 .fail([&done](auto&&... errors) {  // 失败回调
                     std::cout << " Get  FAILED" << std::endl;
                     done = true;
                 })
                 .stop([&done]() {  // 停止回调
                     std::cout << " Get STOPPED" << std::endl;
                     done = true;
                 });
    // 构建可启动的异步任务
    auto k = Build(std::move(e));
    k.Start();  // 启动异步管道
    // 获取默认事件循环
    auto& loop = EventLoop::Default();
    // 设置轮询等待超时时间
    int timeout = 5000;  // 最大等待 5000 ms
    int elapsed = 0;
    // 手动轮询事件循环直到任务完成或超时
    while (!done && elapsed < timeout) {
        loop.RunUntilIdle();  // 处理当前事件循环中的所有任务
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        elapsed += 10;
    }
    // 销毁默认事件循环
    EventLoop::DestructDefault();
    return 0;
}
