//
//  ConverterFactory.hpp
//  SeatDataConverter
//
//  转换器工厂
//

#ifndef ConverterFactory_hpp
#define ConverterFactory_hpp

#include <SeatConverter/core/IConverter.hpp>

#include <functional>
#include <map>
#include <memory>
#include <vector>

namespace kk::converter {

/// 转换器工厂
class ConverterFactory {
  public:
    using ConverterCreator = std::function<std::unique_ptr<IConverter>()>;

    static ConverterFactory &instance();

    /// 注册转换器
    void registerConverter(const std::string &name, ConverterCreator creator);

    /// 根据名称创建转换器
    std::unique_ptr<IConverter> createByName(const std::string &name);

    std::vector<std::string> supportNames() const;

  private:
    ConverterFactory() = default;
    ~ConverterFactory() = default;
    ConverterFactory(const ConverterFactory &) = delete;
    ConverterFactory &operator=(const ConverterFactory &) = delete;

    std::map<std::string, ConverterCreator> converters_;
};

}  // namespace kk::converter

#endif /* ConverterFactory_hpp */
