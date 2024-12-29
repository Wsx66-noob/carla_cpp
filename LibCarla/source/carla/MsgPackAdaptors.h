// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Exception.h"//包含 Carla 项目中定义的异常类的头文件
#include "carla/MsgPack.h"//包含 Carla 项目中用于消息打包（可能是序列化）的头文件

#include <boost/optional.hpp>//包含 Boost.Optional 库的头文件

#ifdef _MSC_VER//这是一个条件编译指令，用于检查是否定义了宏 _MSC_VER
#pragma warning(push)//将当前的警告设置保存到一个堆栈中
#pragma warning(disable:4583)
#pragma warning(disable:4582)
#include <boost/variant2/variant.hpp>//包含 Boost.Variant2 库的头文件
#pragma warning(pop)//恢复之前保存的警告设置
#else
#include <boost/variant2/variant.hpp>
#endif

#include <tuple>

namespace clmdep_msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

  // ===========================================================================
  // -- Adaptors for boost::optional -------------------------------------------
  // ===========================================================================

  // 为 boost::optional<T> 定义的转换特化
  template<typename T>
  struct convert<boost::optional<T>> {
  
    // 重载函数调用运算符，将 MsgPack 对象转换为 boost::optional<T>
    const clmdep_msgpack::object &operator()(
        const clmdep_msgpack::object &o,
        boost::optional<T> &v) const {
      // 确保 MsgPack 对象是一个数组
      if (o.type != clmdep_msgpack::type::ARRAY) {
        ::carla::throw_exception(clmdep_msgpack::type_error());
      }
      
      // 检查数组大小以决定如何设置 optional
      if (o.via.array.size == 1) {
        // 如果大小为 1，表示没有值
        v.reset();
      } else if (o.via.array.size == 2) {
        // 如果大小为 2，从第二个元素中获取值
        v.reset(o.via.array.ptr[1].as<T>());
      } else {
        // 如果大小不为 1 或 2，抛出类型错误
        ::carla::throw_exception(clmdep_msgpack::type_error());
      }
      return o;
    }
  };
  // 为 boost::optional<T> 定义的打包特化
  template<typename T>
  struct pack<boost::optional<T>> {
    template <typename Stream>
    packer<Stream> &operator()(
        clmdep_msgpack::packer<Stream> &o,
        const boost::optional<T> &v) const {
      if (v.has_value()) {
        // 如果 optional 有值，将其打包为大小为 2 的数组
        o.pack_array(2);
        o.pack(true);// 第一个元素表示存在
        o.pack(*v);// 第二个元素是实际值
      } else {
        // 如果 optional 为空，将其打包为大小为 1 的数组
        o.pack_array(1);
        o.pack(false);// 表示值不存在
      }
      return o;
    }
  };
  // 为 boost::optional<T> 定义的带区域的对象特化
  template<typename T>
  struct object_with_zone<boost::optional<T>> {
    void operator()(
        clmdep_msgpack::object::with_zone &o,
        const boost::optional<T> &v) const {
      o.type = type::ARRAY; // 设置类型为数组
      if (v.has_value()) {
        // 如果 optional 有值，设置大小为 2 并分配内存
        o.via.array.size = 2;
        o.via.array.ptr = static_cast<clmdep_msgpack::object*>(o.zone.allocate_align(
            sizeof(clmdep_msgpack::object) * o.via.array.size,
            MSGPACK_ZONE_ALIGNOF(clmdep_msgpack::object)));
        // 第一个元素：true（表示存在）
        o.via.array.ptr[0] = clmdep_msgpack::object(true, o.zone);
         // 第二个元素：实际值
        o.via.array.ptr[1] = clmdep_msgpack::object(*v, o.zone);
      } else {
        // 如果 optional 为空，设置大小为 1 并分配内存
        o.via.array.size = 1;
        o.via.array.ptr = static_cast<clmdep_msgpack::object*>(o.zone.allocate_align(
            sizeof(clmdep_msgpack::object) * o.via.array.size,
            MSGPACK_ZONE_ALIGNOF(clmdep_msgpack::object)));
        // 第一个元素：false（表示不存在）
        o.via.array.ptr[0] = clmdep_msgpack::object(false, o.zone);
      }
    }
  };

  // ===========================================================================
  // -- Adaptors for boost::variant2::variant ----------------------------------
  // ===========================================================================

// 定义一个模板结构体，用于将boost::variant2::variant类型的对象从clmdep_msgpack::object转换
template<typename... Ts>
struct convert<boost::variant2::variant<Ts...>> {

  // 定义一个函数，它是msgpack对象到boost::variant2::variant的转换函数
  const clmdep_msgpack::object &operator()(
      const clmdep_msgpack::object &o,
      boost::variant2::variant<Ts...> &v) const {
    // 检查msgpack对象的类型是否为数组
    if (o.type != clmdep_msgpack::type::ARRAY) {
      // 如果不是数组，抛出类型错误异常
      ::carla::throw_exception(clmdep_msgpack::type_error());
    }
    // 检查数组的大小是否为2（通常用于存储类型索引和实际值）
    if (o.via.array.size != 2) {
      // 如果大小不是2，抛出类型错误异常
      ::carla::throw_exception(clmdep_msgpack::type_error());
    }
    // 获取数组中的第一个元素，它应该是一个索引，用于确定variant中哪个类型将被使用
    const auto index = o.via.array.ptr[0].as<uint64_t>();
    // 使用索引和msgpack对象来复制值到variant对象中
    copy_to_variant(index, o, v, std::make_index_sequence<sizeof...(Ts)>());
    // 返回原始的msgpack对象
    return o;
  }

 private:
  // 结构体的私有成员和函数将在这里定义
};

    // 从对象中复制到变体的实现
    template <uint64_t I>
    static void copy_to_variant_impl(
        const clmdep_msgpack::object &o,
        boost::variant2::variant<Ts...> &v) {
      /// @todo 找到类型的工作环绕。
      auto dummy = std::get<I>(std::tuple<Ts...>{});
      using T = decltype(dummy);
      v = o.via.array.ptr[1].as<T>();// 从对象中获取并赋值给变体
    }
    // 复制到变体的主函数
    template <uint64_t... Is>
// 函数名为 `copy_to_variant`，它是一个静态成员函数，意味着可以通过类名直接调用，而不需要实例化类对象
// 该函数的作用从参数来看可能是将某个 `clmdep_msgpack::object` 类型的数据根据索引复制到 `boost::variant2::variant<Ts...>` 类型的变量中
// 这里的 `Ts...` 同样是可变参数模板参数，代表一系列的类型（具体由调用时传入的类型决定）
    static void copy_to_variant(
// `index` 参数，类型为 `uint64_t`，可能用于指定要操作的索引位置，用于在下面的逻辑中判断和选择合适的处理分支
        const uint64_t index,
// `o` 参数，类型为 `clmdep_msgpack::object`，应该是要被复制的数据来源对象
        const clmdep_msgpack::object &o,
// `v` 参数，类型为 `boost::variant2::variant<Ts...>`，是目标变量，也就是要将数据复制到其中的变量
        boost::variant2::variant<Ts...> &v,
// `std::index_sequence<Is...>` 是一个编译期的索引序列，用于辅助实现一些编译期的循环或者按索引展开等操作（常配合可变参数模板使用）
        std::index_sequence<Is...>) {
    // 使用初始化列表语法，初始化列表中的元素表达式通过逗号分隔
    // 这里的表达式 `(index == Is? copy_to_variant_impl<Is>(o, v), 0 : 0)...` 是一个展开式，会针对每个 `Is` 值进行展开
    // 意思是如果当前的 `index` 值等于某个 `Is` 值，就调用 `copy_to_variant_impl<Is>(o, v)` 函数（具体功能由该函数定义决定，这里应该是真正执行复制数据到变体类型变量 `v` 的操作），并且整个表达式返回值为 `0`（这里的 `0` 只是为了满足初始化列表元素语法要求，并无实际业务含义）；如果 `index` 不等于 `Is`，就直接返回 `0`
      std::initializer_list<int> ({
        (index == Is ? copy_to_variant_impl<Is>(o, v), 0 : 0)...
      });
    }
  };
// 定义一个模板结构体 `pack`，针对 `boost::variant2::variant<Ts...>` 类型进行特化（这里 `Ts...` 同样是可变参数模板参数，表示一系列类型）
// 这个结构体可能用于对 `boost::variant2::variant` 类型进行一些打包或者序列化相关的操作（结合后面的函数重载推测）
  template<typename... Ts>
  struct pack<boost::variant2::variant<Ts...>> {
    // 定义一个函数重载操作符 `()`，使得该结构体的对象可以像函数一样被调用
    // 这个函数接受一个 `clmdep_msgpack::packer<Stream>` 类型的引用 `o`（从类型来看可能是用于将数据打包到某个流中的工具类对象），以及一个 `const boost::variant2::variant<Ts...> &` 类型的常量引用 `v`（要被打包的数据，也就是上面提到的变体类型变量）
    template <typename Stream>
    packer<Stream> &operator()(
        clmdep_msgpack::packer<Stream> &o,
        const boost::variant2::variant<Ts...> &v) const {
      o.pack_array(2);// 打包数组大小
      o.pack(static_cast<uint64_t>(v.index()));
      // 使用访问器打包变体的值
      boost::variant2::visit([&](const auto &value) { o.pack(value); }, v);
      return o;
    }
  };
// 定义一个模板结构体 `object_with_zone`，针对 `boost::variant2::variant<Ts...>` 类型进行特化。
// 这里的 `Ts...` 是可变参数模板参数，表示一系列的类型，意味着这个结构体针对 `boost::variant2::variant` 包含不同具体类型的情况做特殊处理。
  template<typename... Ts>
  struct object_with_zone<boost::variant2::variant<Ts...>> {
    // 定义函数调用操作符 `()`，使得该结构体的对象可以像函数一样被调用，这里的函数是 `const`（常量）成员函数，意味着它不会修改对象的成员变量（如果有的话）。
    // 函数接受两个参数，第一个参数 `o` 是 `clmdep_msgpack::object::with_zone` 类型的引用，从名字推测可能是与带区域（zone，也许和内存分配区域等概念相关）的消息包对象相关，第二个参数 `v` 是 `const boost::variant2::variant<Ts...> &` 类型的常量引用，即要操作的变体类型的数据
    void operator()(
        clmdep_msgpack::object::with_zone &o,
        const boost::variant2::variant<Ts...> &v) const {
// 将传入的 `clmdep_msgpack::object::with_zone` 类型对象 `o` 的 `type` 成员变量设置为 `type::ARRAY`，表明接下来要处理的数据结构形式可能是数组形式（这里的 `type` 应该是用于标记对象的数据类型相关属性，具体取决于 `clmdep_msgpack::object` 类型的定义）。
      o.type = type::ARRAY;
// 设置 `o` 对象中与数组相关的成员变量 `size`，将其赋值为 `2`，意味着接下来要处理的这个数组结构可能包含两个元素（具体含义需结合更多该类型相关的逻辑来确定）。
      o.via.array.size = 2;
// 调用 `o` 对象所属区域（zone）的 `allocate_align` 函数，为存储 `clmdep_msgpack::object` 类型的数据分配内存空间
// `sizeof(clmdep_msgpack::object) * o.via.array.size` 计算出需要分配的内存大小，是 `clmdep_msgpack::object` 类型的大小乘以数组元素个数（这里前面设置为 `2`）。
        // `MSGPACK_ZONE_ALIGNOF(clmdep_msgpack::object)` 可能是用于指定内存对齐方式相关的参数，确保分配的内存按照合适的对齐要求来进行，便于后续对数据的高效访问等操作。
        // 然后将分配得到的内存地址转换为 `clmdep_msgpack::object*` 类型（即指向 `clmdep_msgpack::object` 类型对象的指针），并赋值给 `o` 对象中与数组相关的 `ptr` 成员变量，这样 `o` 对象就能通过这个指针来访问分配好的内存空间用于存储数组元素了。
      o.via.array.ptr = static_cast<clmdep_msgpack::object*>(o.zone.allocate_align(
          sizeof(clmdep_msgpack::object) * o.via.array.size,
          MSGPACK_ZONE_ALIGNOF(clmdep_msgpack::object)));
      // 设置数组的第一个元素为索引
      o.via.array.ptr[0] = clmdep_msgpack::object(static_cast<uint64_t>(v.index()), o.zone);
      boost::variant2::visit([&](const auto &value) {
        o.via.array.ptr[1] = clmdep_msgpack::object(value, o.zone);
      }, v);
    }
  };

} // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack
