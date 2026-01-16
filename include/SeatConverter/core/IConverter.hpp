//
//  IConverter.hpp
//  SeatDataConverter
//
//  转换器接口
//

#ifndef IConverter_hpp
#define IConverter_hpp

#include <memory>
#include <string>

#include <SeatConverter/core/ConvertResult.hpp>

namespace kk::converter {

/// 转换器接口
class IConverter {
  public:
    virtual ~IConverter() = default;

    /// 转换数据
    /// @param jsonData 第三方JSON数据
    /// @return 转换结果
    virtual ConvertResult convert(const std::string &jsonData) = 0;

    /// 获取转换器名称
    virtual std::string getName() const = 0;
};

}  // namespace kk::converter

#endif /* IConverter_hpp */
