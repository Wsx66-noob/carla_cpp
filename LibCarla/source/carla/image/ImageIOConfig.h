// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // 确保头文件只被包含一次

#include "carla/FileSystem.h"  // 引入文件系统相关的头文件
#include "carla/Logging.h"     // 引入日志记录相关的头文件
#include "carla/StringUtil.h"  // 引入字符串工具相关的头文件
#include "carla/image/BoostGil.h"  // 引入Boost.GIL库的图像处理功能

// 检测是否支持PNG格式
#ifndef LIBCARLA_IMAGE_WITH_PNG_SUPPORT
#  if defined(__has_include) && __has_include("png.h")  // 检查是否能找到png.h头文件
#    define LIBCARLA_IMAGE_WITH_PNG_SUPPORT true  // 定义为支持
#  else
#    define LIBCARLA_IMAGE_WITH_PNG_SUPPORT false // 定义为不支持
#  endif
#endif

// 如果定义了LIBCARLA_IMAGE_WITH_JPEG_SUPPORT宏，表示支持JPEG格式的图像处理
#if LIBCARLA_IMAGE_WITH_JPEG_SUPPORT

    // 定义一个静态常量函数，用于获取默认的文件扩展名
    static constexpr const char *get_default_extension() {
      // 返回“jpeg”作为默认的文件扩展名
      return "jpeg";
    }

    // 定义一个模板函数，用于检查给定的字符串是否匹配JPEG文件的扩展名
    template <typename Str>
    static bool match_extension(const Str &str) {
      // 判断字符串是否以默认扩展名“jpeg”结尾，或者以“jpg”结尾
      return StringUtil::EndsWith(str, get_default_extension()) ||
             StringUtil::EndsWith(str, "jpg");
    }

    // 定义一个模板函数，用于从文件中读取JPEG图像
    template <typename Str, typename ImageT>
    static void read_image(Str &&in_filename, ImageT &image) {
      // 使用boost库的gil模块读取JPEG格式的图像
      boost::gil::read_image(std::forward<Str>(in_filename), image, boost::gil::jpeg_tag());
    }

    // 定义一个模板函数，用于将视图写入JPEG格式的文件
    // 这个函数只在支持写入JPEG视图的情况下可用
    template <typename Str, typename ViewT>
    static typename std::enable_if<is_write_supported<ViewT, boost::gil::jpeg_tag>::value>::type
    write_view(Str &&out_filename, const ViewT &view) {
      // 使用boost库的gil模块写入JPEG视图
      boost::gil::write_view(std::forward<Str>(out_filename), view, boost::gil::jpeg_tag());
    }
#endif

#if LIBCARLA_IMAGE_WITH_PNG_SUPPORT == true  // 如果支持PNG格式
#  ifndef png_infopp_NULL
#    define png_infopp_NULL (png_infopp)NULL  // 定义PNG信息指针为NULL
#  endif // png_infopp_NULL
#  ifndef int_p_NULL
#    define int_p_NULL (int*)NULL  // 定义整数指针为NULL
#  endif // int_p_NULL
#  if defined(__clang__)  // 如果使用的是Clang编译器
#    pragma clang diagnostic push  // 保存当前的诊断状态
#    pragma clang diagnostic ignored "-Wignored-qualifiers"  // 忽略类型限定符被忽略的警告
#    pragma clang diagnostic ignored "-Wparentheses"  // 忽略括号相关的警告
#  endif
#    include <boost/gil/extension/io/png.hpp>  // 引入Boost.GIL的PNG输入输出扩展
#  if defined(__clang__)  // 如果使用的是Clang编译器
#    pragma clang diagnostic pop  // 恢复之前的诊断状态
#  endif
#endif

#if LIBCARLA_IMAGE_WITH_JPEG_SUPPORT == true  // 如果支持JPEG格式
#  include <boost/gil/extension/io/jpeg.hpp>  // 引入Boost.GIL的JPEG输入输出扩展
#endif

#if LIBCARLA_IMAGE_WITH_TIFF_SUPPORT == true  // 如果支持TIFF格式
#  include <boost/gil/extension/io/tiff.hpp>  // 引入Boost.GIL的TIFF输入输出扩展
#endif

#if defined(__clang__)  // 如果使用的是Clang编译器
#  pragma clang diagnostic pop  // 恢复之前的诊断状态
#endif

namespace carla {  // 定义命名空间carla
namespace image {  // 定义命名空间image
namespace io {  // 定义命名空间io

  constexpr bool has_png_support() {  // 检查是否支持PNG格式
    return LIBCARLA_IMAGE_WITH_PNG_SUPPORT;  // 返回支持状态
  }

  constexpr bool has_jpeg_support() {  // 检查是否支持JPEG格式
    return LIBCARLA_IMAGE_WITH_JPEG_SUPPORT;  // 返回支持状态
  }

  constexpr bool has_tiff_support() {  // 检查是否支持TIFF格式
    return LIBCARLA_IMAGE_WITH_TIFF_SUPPORT;  // 返回支持状态
  }

  static_assert(has_png_support() || has_jpeg_support() || has_tiff_support(),
      "No image format supported, please compile with at least one of "
      "LIBCARLA_IMAGE_WITH_PNG_SUPPORT, LIBCARLA_IMAGE_WITH_JPEG_SUPPORT, "
      "or LIBCARLA_IMAGE_WITH_TIFF_SUPPORT");  // 检查至少支持一种图像格式

namespace detail {  // 定义命名空间detail

  template <typename ViewT, typename IOTag>  // 模板结构体，接受视图类型和输入输出标签
  struct is_write_supported {  // 定义写入支持结构体
    static constexpr bool value = boost::gil::is_write_supported<typename boost::gil::get_pixel_type<ViewT>::type, IOTag>::value;  // 判断写入是否支持
  };

  struct io_png {  // 定义PNG输入输出结构体
#if LIBCARLA_IMAGE_WITH_PNG_SUPPORT // 如果支持PNG格式

    static constexpr const char *get_default_extension() { // 获取默认扩展名
      return "png"; // 返回“png”作为默认扩展名
    }

    template <typename Str>
    static bool match_extension(const Str &str) { // 匹配文件扩展名
      return StringUtil::EndsWith(str, get_default_extension()); // 判断字符串是否以默认扩展名结尾
    }

    template <typename Str, typename ImageT>
    static void read_image(Str &&in_filename, ImageT &image) { // 读取图像
      boost::gil::read_and_convert_image(std::forward<Str>(in_filename), image, boost::gil::png_tag()); // 使用boost库读取并转换PNG图像
    }

    template <typename Str, typename ViewT>
    static void write_view(Str &&out_filename, const ViewT &view) { // 写入视图到文件
      boost::gil::write_view(std::forward<Str>(out_filename), view, boost::gil::png_tag()); // 使用boost库写入PNG视图
    }

#endif // LIBCARLA_IMAGE_WITH_PNG_SUPPORT // 结束PNG支持条件编译

  };

  struct io_jpeg { // JPEG输入输出结构体

    static constexpr bool is_supported = has_jpeg_support(); // 检查是否支持JPEG格式

#if LIBCARLA_IMAGE_WITH_JPEG_SUPPORT // 如果支持JPEG格式

    static constexpr const char *get_default_extension() { // 获取默认扩展名
      return "jpeg"; // 返回“jpeg”作为默认扩展名
    }

    template <typename Str>
    static bool match_extension(const Str &str) { // 匹配文件扩展名
      return StringUtil::EndsWith(str, get_default_extension()) || // 判断字符串是否以默认扩展名结尾
             StringUtil::EndsWith(str, "jpg"); // 判断字符串是否以“jpg”结尾
    }

    template <typename Str, typename ImageT>
    static void read_image(Str &&in_filename, ImageT &image) { // 读取图像
      boost::gil::read_image(std::forward<Str>(in_filename), image, boost::gil::jpeg_tag()); // 使用boost库读取JPEG图像
    }

    template <typename Str, typename ViewT>
    static typename std::enable_if<is_write_supported<ViewT, boost::gil::jpeg_tag>::value>::type
    write_view(Str &&out_filename, const ViewT &view) { // 写入视图到文件（支持的情况）
      boost::gil::write_view(std::forward<Str>(out_filename), view, boost::gil::jpeg_tag()); // 使用boost库写入JPEG视图
    }

    template <typename Str, typename ViewT>
    static typename std::enable_if<!is_write_supported<ViewT, boost::gil::jpeg_tag>::value>::type
    write_view(Str &&out_filename, const ViewT &view) { // 写入视图到文件（不支持的情况）
      boost::gil::write_view(
          std::forward<Str>(out_filename),
          boost::gil::color_converted_view<boost::gil::rgb8_pixel_t>(view), // 将视图转换为RGB8像素格式
          boost::gil::jpeg_tag()); // 使用boost库写入JPEG视图
    }
#endif // LIBCARLA_IMAGE_WITH_JPEG_SUPPORT // 结束JPEG支持的条件编译

struct io_tiff { // 定义一个io_tiff结构体

    static constexpr bool is_supported = has_tiff_support(); // 判断是否支持TIFF格式

#if LIBCARLA_IMAGE_WITH_TIFF_SUPPORT // 如果开启了TIFF支持

    static constexpr const char *get_default_extension() { // 获取默认扩展名
        return "tiff"; // 返回TIFF
    }

    template <typename Str> // 模板函数，接受任何字符串类型
    static bool match_extension(const Str &str) { // 检查文件扩展名是否匹配
        return StringUtil::EndsWith(str, get_default_extension()); // 返回是否以默认扩展名结尾
    }

    template <typename Str, typename ImageT> // 模板函数，接受字符串和图像类型
    static void read_image(Str &&in_filename, ImageT &image) { // 读取图像
        boost::gil::read_and_convert_image(std::forward<Str>(in_filename), image, boost::gil::tiff_tag()); // 使用boost库读取并转换图像
    }

    template <typename Str, typename ViewT> // 模板函数，接受字符串和视图类型
    static typename std::enable_if<is_write_supported<ViewT, boost::gil::tiff_tag>::value>::type // 如果支持写入
    write_view(Str &&out_filename, const ViewT &view) { // 写入视图
        boost::gil::write_view(std::forward<Str>(out_filename), view, boost::gil::tiff_tag()); // 使用boost库写入视图
    }

    template <typename Str, typename ViewT> // 模板函数，接受字符串和视图类型
    static typename std::enable_if<!is_write_supported<ViewT, boost::gil::tiff_tag>::value>::type // 如果不支持写入
    write_view(Str &&out_filename, const ViewT &view) { // 写入视图
        boost::gil::write_view( // 使用boost库写入
            std::forward<Str>(out_filename), // 转发输出文件名
            boost::gil::color_converted_view<boost::gil::rgb8_pixel_t>(view), // 先进行颜色转换
            boost::gil::tiff_tag()); // 使用TIFF标签
    }

#endif // LIBCARLA_IMAGE_WITH_TIFF_SUPPORT // 结束TIFF支持的条件编译
};

struct io_resolver { // 定义一个io_resolver结构体

    template <typename IO, typename Str> // 模板函数，接受IO类型和字符串类型
    static typename std::enable_if<IO::is_supported, bool>::type match_extension(const Str &str) { // 如果IO支持
        return IO::match_extension(str); // 调用IO的匹配扩展名函数
    }

    template <typename IO, typename Str> // 模板函数，接受IO类型和字符串类型
    static typename std::enable_if<!IO::is_supported, bool>::type match_extension(const Str &) { // 如果IO不支持
        return false; // 返回false
    }

    template <typename IO, typename Str, typename... Args> // 模板函数，接受IO类型、字符串和其他参数
    static typename std::enable_if<IO::is_supported>::type read_image(const Str &path, Args &&... args) { // 如果IO支持读取
        log_debug("reading", path, "as", IO::get_default_extension()); // 日志记录正在读取的文件
        IO::read_image(path, std::forward<Args>(args)...); // 调用IO的读取图像函数
    }

    template <typename IO, typename... Args> // 模板函数，接受IO类型和其他参数
    static typename std::enable_if<!IO::is_supported>::type read_image(Args &&...) { // 如果IO不支持读取
        DEBUG_ASSERT(false); // 断言失败
    }

    template <typename IO, typename... Args> // 模板函数，接受IO类型和其他参数
    static typename std::enable_if<IO::is_supported>::type write_view(std::string &path, Args &&... args) { // 如果IO支持写入
        FileSystem::ValidateFilePath(path, IO::get_default_extension()); // 验证文件路径
        log_debug("writing", path, "as", IO::get_default_extension()); // 日志记录正在写入的文件
        IO::write_view(path, std::forward<Args>(args)...); // 调用IO的写入视图函数
    }

    template <typename IO, typename... Args> // 模板函数，接受IO类型和其他参数
    static typename std::enable_if<!IO::is_supported>::type write_view(Args &&...) { // 如果IO不支持写入
        DEBUG_ASSERT(false); // 断言失败
    }
};

template <typename... IOs> // 模板结构体，接受多个IO类型
struct io_impl; // 前向声明

template <typename IO> // 模板结构体，接受单个IO类型
struct io_impl<IO> { // 实现io_impl
    constexpr static bool is_supported = IO::is_supported; // 是否支持IO

    template <typename... Args> // 模板函数，接受任意参数
    static void read_image(Args &&... args) { // 读取图像
        io_resolver::read_image<IO>(std::forward<Args>(args)...); // 调用io_resolver的读取函数
    }

    template <typename... Args> // 模板函数，接受任意参数
    static void write_view(Args &&... args) { // 写入视图
        io_resolver::write_view<IO>(std::forward<Args>(args)...); // 调用io_resolver的写入函数
    }

    template <typename Str, typename... Args> // 模板函数，接受字符串和任意参数
    static bool try_read_image(const Str &filename, Args &&... args) { // 尝试读取图像
        if (io_resolver::match_extension<IO>(filename)) { // 如果扩展名匹配
            io_resolver::read_image<IO>(filename, std::forward<Args>(args)...); // 读取图像
            return true; // 返回true
        }
        return false; // 返回false
    }

    template <typename Str, typename... Args> // 模板函数，接受字符串和任意参数
    static bool try_write_view(Str &filename, Args &&... args) { // 尝试写入视图
        if (io_resolver::match_extension<IO>(filename)) { // 如果扩展名匹配
            io_resolver::write_view<IO>(filename, std::forward<Args>(args)...); // 写入视图
            return true; // 返回true
        }
        return false; // 返回false
    }
};
  template <typename IO, typename... IOs> // 定义一个模板结构体io_impl，接受一个IO类型和多个IO类型
struct io_impl<IO, IOs...> { // 特化io_impl
private:
    using self = io_impl<IO>; // 定义self为当前IO的io_impl
    using recursive = io_impl<IOs...>; // 定义recursive为剩余IOs的io_impl
public:

    constexpr static bool is_supported = self::is_supported || recursive::is_supported; // 判断是否支持任一IO

    template <typename... Args> // 定义一个接受任意参数的模板函数
    static void read_image(Args &... args) { // 读取图像函数
      if (!recursive::try_read_image(args...)) { // 如果递归IO未能读取图像
        self::read_image(args...); // 则调用当前IO读取图像
      }
    }

    template <typename... Args> // 定义一个接受任意参数的模板函数
    static bool try_read_image(Args &... args) { // 尝试读取图像函数
      return recursive::try_read_image(args...) || self::try_read_image(args...); // 返回递归IO或当前IO是否能读取图像
    }

    template <typename... Args> // 定义一个接受任意参数的模板函数
    static void write_view(Args &... args) { // 写入视图函数
      if (!recursive::try_write_view(args...)) { // 如果递归IO未能写入视图
        self::write_view(args...); // 则调用当前IO写入视图
      }
    }

    template <typename... Args> // 定义一个接受任意参数的模板函数
    static bool try_write_view(Args &... args) { // 尝试写入视图函数
      return recursive::try_write_view(args...) || self::try_write_view(args...); // 返回递归IO或当前IO是否能写入视图
    }
};

template <typename DefaultIO, typename... IOs> // 定义一个模板结构体io_any，接受默认IO和多个IO
struct io_any : detail::io_impl<DefaultIO, IOs...> { // 继承自detail::io_impl
    static_assert(DefaultIO::is_supported, "Default IO needs to be supported."); // 确保默认IO是支持的
};

} // namespace detail

struct png : detail::io_impl<detail::io_png> {}; // 定义png结构体，继承自io_impl

struct jpeg : detail::io_impl<detail::io_jpeg> {}; // 定义jpeg结构体，继承自io_impl

struct tiff : detail::io_impl<detail::io_tiff> {}; // 定义tiff结构体，继承自io_impl

#if LIBCARLA_IMAGE_WITH_PNG_SUPPORT // 如果支持PNG格式

struct any : detail::io_any<detail::io_png, detail::io_tiff, detail::io_jpeg> {}; // 定义any结构体，支持PNG、TIFF和JPEG

#elif LIBCARLA_IMAGE_WITH_TIFF_SUPPORT // 如果不支持PNG但支持TIFF

struct any : detail::io_any<detail::io_tiff, detail::io_jpeg> {}; // 定义any结构体，支持TIFF和JPEG

#else // 如果以上都不支持

struct any : detail::io_any<detail::io_jpeg> {}; // 定义any结构体，仅支持JPEG

#endif

} // namespace io
} // namespace image
} // namespace carla
