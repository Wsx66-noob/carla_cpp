// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 这是一个预处理指令，用于防止头文件被多次包含
#pragma once // 防止头文件被多次包含

// 引入CARLA项目中的调试相关的头文件
#include "carla/Debug.h" // 引入调试相关的头文件

// 引入Boost库中的时间处理功能，特别是用于处理POSIX时间类型的功能
#include <boost/date_time/posix_time/posix_time_types.hpp> // 引入 Boost 的时间处理功能

// 引入C++标准库中的chrono库，用于时间、持续时间和时钟功能
#include <chrono>

// 定义一个名为carla的命名空间，用于封装相关的代码和标识符
namespace carla {

  /// 这是一个文档注释，说明接下来的time_duration类的功能
  /// time_duration类表示一个正的时间持续期，精确到毫秒。
  /// 它能够在std::chrono::duration和boost::posix_time::time_duration之间自动转换。
  class time_duration {
  public:
    // 类的公有成员将在这里定义
  };
}
// 静态成员函数，用于创建一个表示指定秒数的时间间隔
    static inline time_duration seconds(size_t timeout) {
      return std::chrono::seconds(timeout);
    }
// 静态成员函数，用于创建一个表示指定毫秒数的时间间隔
    static inline time_duration milliseconds(size_t timeout) {
      return std::chrono::milliseconds(timeout);
    }
// 默认构造函数，创建一个表示 0 毫秒的时间间隔
    constexpr time_duration() noexcept : _milliseconds(0u) {}

// 模板构造函数，接受一个 std::chrono::duration 类型的参数，将其转换为以毫秒为单位的时间间隔
    template <typename Rep, typename Period>
    time_duration(std::chrono::duration<Rep, Period> duration)
      : _milliseconds([=]() {
      	// 将传入的 duration 转换为毫秒数，并进行断言确保结果非负
          const auto count = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
          DEBUG_ASSERT(count >= 0);
          return static_cast<size_t>(count);
        }()) {}
// 构造函数，接受一个 boost::posix_time::time_duration 类型的参数
// 并将该参数转换为以毫秒为单位的时间间隔，然后初始化 time_duration 对象
time_duration(boost::posix_time::time_duration timeout)
  : time_duration(std::chrono::milliseconds(timeout.total_milliseconds())) {}

// 拷贝构造函数，使用编译器提供的默认实现
// 这意味着 time_duration 对象可以使用另一个 time_duration 对象进行初始化
time_duration(const time_duration &) = default;

// 拷贝赋值运算符，使用编译器提供的默认实现
// 这意味着 time_duration 对象可以使用另一个 time_duration 对象进行赋值
time_duration &operator=(const time_duration &) = default;

// 将当前时间间隔转换为 boost::posix_time::time_duration 类型
// 这个函数返回一个 boost::posix_time::time_duration 对象，代表相同的时间间隔
boost::posix_time::time_duration to_posix_time() const {
  return boost::posix_time::milliseconds(_milliseconds);
}

// 将当前时间间隔转换为 std::chrono::milliseconds 类型
// 这个函数返回一个 std::chrono::milliseconds 对象，代表相同的时间间隔
constexpr auto to_chrono() const {
  return std::chrono::milliseconds(_milliseconds);
}

// 类型转换运算符，允许 time_duration 对象在需要 boost::posix_time::time_duration 类型时被隐式转换
// 这个运算符返回一个 boost::posix_time::time_duration 对象，代表相同的时间间隔
operator boost::posix_time::time_duration() const {
  return to_posix_time();
}

// 返回以毫秒为单位的时间值
// 不修改对象状态，保证不抛出异常（noexcept），返回值是 size_t 类型
constexpr size_t milliseconds() const noexcept {
  return _milliseconds;
}
  private:
// 存储时间间隔的毫秒数
    size_t _milliseconds;
  };

} // namespace carla
