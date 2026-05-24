package com.libseatcanvas.style

import androidx.annotation.ColorInt
import org.json.JSONArray
import org.json.JSONObject

/**
 * 构建座位样式 JSON，格式与核心层解析协议一致：顶层为数组，元素含字符串键 `key` 与对象 `config`
 */
class SeatStyleConfigBuilder {
    private val configs = mutableMapOf<String, SeatStyleConfig>()

    /**
     * 注册圆形样式
     * @param styleId 样式ID（使用[SeatRenderStyleId.compose] 创建）
     * @param fill 填充色
     * @param overlay 遮罩色，仅选中态需要时可传
     * @param checkmark 对勾色，仅选中态需要时可传
     */
    fun addCircleStyle(
        styleId: String,
        @ColorInt fill: Int,
        @ColorInt overlay: Int? = null,
        @ColorInt checkmark: Int? = null
    ): SeatStyleConfigBuilder {
        if (styleId.isEmpty()) {
            return this
        }
        configs[styleId] = CircleSeatStyleConfig(fill, overlay, checkmark)
        return this
    }

    /**
     * 注册 SVG 样式
     * @param styleId 样式ID（使用[SeatRenderStyleId.compose] 创建）
     * @param content SVG 字符串内容
     */
    fun addSVGStyle(styleId: String, content: String): SeatStyleConfigBuilder {
        if (styleId.isEmpty()) {
            return this
        }
        configs[styleId] = SVGSeatStyleConfig(content)
        return this
    }

    /** 序列化为 UTF-8 JSON 字节，失败返回 null */
    fun toJSONData(): ByteArray? {
        return try {
            val jsonString = toJSONString()
            jsonString?.toByteArray(Charsets.UTF_8)
        } catch (e: Exception) {
            null
        }
    }

    /** 序列化为 JSON 字符串，失败返回 null */
    fun toJSONString(): String? {
        return try {
            val jsonArray = JSONArray()
            for ((styleId, config) in configs) {
                val entry = JSONObject()
                entry.put("key", styleId)
                val configObj = JSONObject()
                configObj.put("type", config.type.value)
                when (config) {
                    is CircleSeatStyleConfig -> {
                        configObj.put("fill", config.fill.toARGBHex())
                        config.overlay?.let { configObj.put("overlay", it.toARGBHex()) }
                        config.checkmark?.let { configObj.put("checkmark", it.toARGBHex()) }
                    }
                    is SVGSeatStyleConfig -> {
                        configObj.put("content", config.content)
                    }
                }
                entry.put("config", configObj)
                jsonArray.put(entry)
            }
            jsonArray.toString()
        } catch (e: Exception) {
            null
        }
    }

    /** 清空已注册的样式 */
    fun clear() {
        configs.clear()
    }
}
