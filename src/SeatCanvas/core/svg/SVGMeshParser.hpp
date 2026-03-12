//
//  SVGMeshParser.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#ifndef SVGMeshParser_hpp
#define SVGMeshParser_hpp

#include <map>
#include <memory>
#include <vector>

#include <tgfx/core/Matrix.h>
#include <tgfx/core/Path.h>
#include <tgfx/core/Size.h>
#include <tgfx/svg/SVGLengthContext.h>

namespace tgfx {
class SVGDOM;
class SVGNode;
class SVGLine;
class SVGCircle;
class SVGEllipse;
class SVGPath;
class SVGPoly;
class SVGRect;
class SVGGroup;
};  // namespace tgfx

namespace kk::renderer {
class ZoneMeshInfo;
class BaseMapMeshBuilder;
};  // namespace kk::renderer

namespace kk::svg {

/// SVG Mesh 解析结果
struct SVGMeshParseResult {
    /// SVG 原始尺寸
    tgfx::Size size;
    /// 底图数据（包含 mesh 数据、区域信息、Path）
    std::shared_ptr<kk::renderer::BaseMapMeshBuilder> meshBuilder;
};

/// SVG Mesh 解析器
/// 负责从 SVGDOM 解析图形数据：
/// 1. 解析图形节点（Circle, Rect, Ellipse, Path, Poly）生成 mesh 顶点数据
/// 2. 收集区域信息（bounds, zoneId, attributes）
/// 3. 保存 Path 用于精确 hit-test
class SVGMeshParser {
  public:
    SVGMeshParser() = default;
    ~SVGMeshParser() = default;

    /// 解析 SVG DOM
    /// @param dom SVG DOM
    /// @return 解析结果
    std::unique_ptr<SVGMeshParseResult> parse(std::shared_ptr<tgfx::SVGDOM> dom);

  private:
    void parseNode(tgfx::SVGNode *node, const tgfx::SVGLengthContext &lengthContext,
                   const tgfx::Matrix &parentTransform);
    void parseGroup(tgfx::SVGGroup *node, const tgfx::SVGLengthContext &lengthContext,
                    const tgfx::Matrix &parentTransform);
    void parseLine(tgfx::SVGLine *node, const tgfx::SVGLengthContext &lengthContext,
                   const tgfx::Matrix &parentTransform);
    void parseCircle(tgfx::SVGCircle *node, const tgfx::SVGLengthContext &lengthContext,
                     const tgfx::Matrix &parentTransform);
    void parseEllipse(tgfx::SVGEllipse *node, const tgfx::SVGLengthContext &lengthContext,
                      const tgfx::Matrix &parentTransform);
    void parseRect(tgfx::SVGRect *node, const tgfx::SVGLengthContext &lengthContext,
                   const tgfx::Matrix &parentTransform);
    void parsePath(tgfx::SVGPath *node, const tgfx::SVGLengthContext &lengthContext,
                   const tgfx::Matrix &parentTransform);
    void parsePoly(tgfx::SVGPoly *node, const tgfx::SVGLengthContext &lengthContext,
                   const tgfx::Matrix &parentTransform);

    void removeInvisible();

    /// 处理解析后的 Path，生成 mesh 和区域信息
    std::shared_ptr<kk::renderer::ZoneMeshInfo> processPath(const tgfx::Path &path, tgfx::SVGNode *node, const tgfx::SVGLengthContext &lengthContext);

  private:
    kk::renderer::BaseMapMeshBuilder *meshBuilder = nullptr;
    std::vector<std::pair<tgfx::SVGNode *, tgfx::Path>> shapeOrders = {};
};

};  // namespace kk::svg

#endif /* SVGMeshParser_hpp */
