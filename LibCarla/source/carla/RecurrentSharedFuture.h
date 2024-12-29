// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // 防止头文件被重复包含

#include "carla/Exception.h"  // 引入CARLA项目中的异常处理头文件 
#include "carla/Time.h"   // 引入CARLA项目中的时间处理头文件

#include <boost/optional.hpp>  // 引入 Boost 库中的 optional 头文件，用于表示可选的值
#ifdef _MSC_VER  // 如果是在 Microsoft Visual C++ (MSVC) 环境下编译
#pragma warning(push)  // 保存当前的警告状态，以便之后恢复 
#pragma warning(disable:4583)  // 禁用特定于 MSVC 的警告 4583，这个警告通常与编译器如何处理模板实例化有关
#pragma warning(disable:4582)  // 禁用特定于 MSVC 的警告 4582，这个警告涉及构造函数或析构函数的隐式转换
#include <boost/variant2/variant.hpp> // 引入 Boost 库中的 variant2 头文件，variant2 是 Boost.Variant 的后续版本，提供了更灵活和强大的类型安全联合体
#pragma warning(pop) // 恢复之前保存的警告状态
#else
#include <boost/variant2/variant.hpp> // 如果不是在 MSVC 环境下，直接引入 Boost.Variant2
#endif

#include <condition_variable> // 引入 C++ 标准库中的条件变量头文件，用于同步操作，如等待某个条件成立 
#include <exception> // 引入 C++ 标准库中的异常处理头文件
#include <map> // 引入 C++ 标准库中的 map 头文件，map 是一个关联容器，存储的元素是键值对 
#include <mutex> // 引入 C++ 标准库中的互斥锁头文件，用于提供互斥锁，以保护共享数据的同步访问 

namespace carla {

namespace detail {

  class SharedException; // 定义一个异常类，用于在文件中共享和传递异常信息

} // namespace detail

  // ===========================================================================
  // -- 并发共享未来 RecurrentSharedFuture ------------------------------------
  // ===========================================================================

  /// 这个类类似于共享未来（shared future）的使用方式，但是它的值可以被设置任意次数的值。
  /// 未来设计模式的核心思想是异步调用。
  /// Future接口象征着异步执行任务的结果即执行一个耗时任务完全可以另起一个线程执行，然后此时我们可以去做其他事情，做完其他事情我们再调用Future.get()方法获取结果即可。
  /// 对于未来模式来说，它无法立即返回你需要的数据，但是它会返回一个契约，将来你可以凭借这个契约去获取你需要的信息。
  /// 服务程序并不等数据处理完成便立即返回客户端一个伪造的数据（如：商品的订单，而不是商品本身）；
  /// 在完成其他业务处理后，最后再使用返回比较慢的Future数据。
  /// 参考：https://blog.csdn.net/weixin_43816711/article/details/125664746
  template <typename T>
  class RecurrentSharedFuture {
  public:

    using SharedException = detail::SharedException; // 使用detail命名空间下的SharedException类型，作为此模板类的一部分 

    ///等待直到下一个值被设置。任意数量的线程可以同时等待。
    /// 
    /// @return 如果达到超时时间timeout仍然未获得结果，则返回空的 boost::optional
    /// boost::optional 即可选返回值，是函数的返回值，可能并不总是返回结果。
    boost::optional<T> WaitFor(time_duration timeout);

    /// 设置值并通知所有等待的线程
    template <typename T2>
    void SetValue(const T2 &value);

    /// 设置一个异常，这个异常将会被抛给所有正在等待的线程
    ///
    /// @note The @a exception 将被存储在一个名为 SharedException 的共享对象上，并且会作为这样的异常被抛出
    template <typename ExceptionT> /// 定义一个模板类，该类可以处理并存储特定类型的异常
    void SetException(ExceptionT &&exception);

  private:
      // 互斥量：可以确保一次只有一个线程可以访问共享资源，避免竞争条件的发生
    std::mutex _mutex;
      // 条件变量是c++中提供的一种多线程同步机制，它允许一个或多个线程等待另一个消除发出通知，以便能够有效地进行线程同步
    std::condition_variable _cv;
      // 定义一个结构体，用于映射键（const char*）到值和等待状态 
    struct mapped_type {
      bool should_wait;
      boost::variant2::variant<SharedException, T> value;  // boost::variant2实现类型转换
    };

    std::map<const char *, mapped_type> _map;  // 所有线程构成的map
  };

  // ===========================================================================
  // -- RecurrentSharedFuture 实现 ---------------------------------------------
  // ===========================================================================
// 定义了一个名为 detail 的命名空间
namespace detail {
//// 定义一个线程局部的静态常量字符变量，用于标识或标记当前线程，其值默认为空字符（'\0'）
// 定义一个线程局部静态常量char变量，不初始化，其值未定义
// thread_local关键字表示这个变量在每个线程中都有独立的实例
static thread_local const char thread_tag{};

// 定义一个名为SharedException的类，它继承自标准库中的std::exception类
class SharedException : public std::exception {
public:
  // SharedException类的构造函数，无参数
  // 创建一个std::runtime_error异常的shared_ptr，并初始化_exception成员变量
  SharedException()
    : _exception(std::make_shared<std::runtime_error>("uninitialized SharedException")) {}

  // SharedException类的构造函数，接受一个std::shared_ptr<std::exception>类型的参数
  // 将传入的shared_ptr移动到_exception成员变量
  SharedException(std::shared_ptr<std::exception> e)
    : _exception(std::move(e)) {}

  // 重写std::exception的what()函数，返回异常信息
  // noexcept表示这个函数不会抛出异常
  const char *what() const noexcept override {
    return _exception->what();
  }

  // 提供一个函数，返回内部存储的std::exception的shared_ptr
  std::shared_ptr<std::exception> GetException() const {
    return _exception;
  }

private:
  // SharedException类的私有成员变量，存储一个指向std::exception的shared_ptr
  std::shared_ptr<std::exception> _exception;
};

// 结束detail命名空间（虽然代码中没有显示开始detail命名空间，但根据闭合大括号可以推断）
} // namespace detail

} // namespace detail

  //  如果达到超时时间timeout仍然未获得结果，则返回空的 boost::optional
  template <typename T>
  boost::optional<T> RecurrentSharedFuture<T>::WaitFor(time_duration timeout) {
    // std::mutex提供的lock()和unlock()方法，用于在需要访问共享资源时加锁和解锁。
    // 当一个线程获得了锁之后，其他线程会被阻塞直到锁被释放。
    // 这样可以保证同一时刻只有一个线程可以访问共享资源，从而确保数据的一致性和正确性。
    std::unique_lock<std::mutex> lock(_mutex);
    auto &r = _map[&detail::thread_tag];
    r.should_wait = true;
    // wait_for() 函数用于阻塞线程并等待唤醒，它可以设置一个超时时间 timeout.to_chrono()。
    if (!_cv.wait_for(lock, timeout.to_chrono(), [&]() { return !r.should_wait; })) {
      return {};
    }
    if (r.value.index() == 0) {
      throw_exception(boost::variant2::get<SharedException>(r.value));
    }
    return boost::variant2::get<T>(std::move(r.value));
  }

  // 设置值并通知所有等待的线程
// 这是一个模板函数，允许为不同类型的RecurrentSharedFuture设置值
template <typename T>
template <typename T2>
void RecurrentSharedFuture<T>::SetValue(const T2 &value) {
  // 使用std::lock_guard来自动管理互斥锁的锁定和解锁
  std::lock_guard<std::mutex> lock(_mutex);
  // 遍历_map中的所有线程对应的值对
  for (auto &pair : _map) {
    // 将每个线程的should_wait标志设置为false，表示不再需要等待
    pair.second.should_wait = false;
    // 为每个线程设置新的值
    pair.second.value = value;
  }
  // 通知所有等待的线程，它们可以继续执行了
  _cv.notify_all();
}

// 设置一个异常，并通知所有等待的线程
// 这是一个模板函数，允许为不同类型的异常设置RecurrentSharedFuture的异常状态
template <typename T>
template <typename ExceptionT>
void RecurrentSharedFuture<T>::SetException(ExceptionT &&e) {
  // 将传入的异常封装为一个std::shared_ptr，然后使用SetValue设置为当前值
  SetValue(SharedException(std::make_shared<ExceptionT>(std::forward<ExceptionT>(e))));
}

} // 结束carla命名空间
