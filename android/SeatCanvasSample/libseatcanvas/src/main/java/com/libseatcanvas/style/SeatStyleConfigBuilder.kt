package com.libseatcanvas.style

import androidx.annotation.ColorInt
import org.json.JSONArray
import org.json.JSONObject

class SeatStyleConfigBuilder {
    private val configs = mutableMapOf<SeatStyleKey, SeatStyleConfig>()

    /**
     * 添加圆形样式配置
     * @param status 座位状态
     * @param selected 是否选中
     * @param fill 填充颜色（ARGB 格式的 Int 值，可使用 Color.parseColor() 或 Color.rgb() 等方法创建）
     * @param overlay 覆盖层颜色（选中时显示，ARGB 格式的 Int 值）
     * @param checkmark 勾选标记颜色（ARGB 格式的 Int 值）
     * @return Builder 实例，支持链式调用
     */
    fun addCircleStyle(
        status: UInt,
        selected: Boolean,
        @ColorInt fill: Int,
        @ColorInt overlay: Int,
        @ColorInt checkmark: Int
    ): SeatStyleConfigBuilder {
        val key = SeatStyleKey(status, selected)
        val config = CircleSeatStyleConfig(fill, overlay, checkmark)
        configs[key] = config
        return this
    }

    /**
     * 添加 SVG 样式配置
     * @param status 座位状态
     * @param selected 是否选中
     * @param content SVG 内容
     * @return Builder 实例，支持链式调用
     */
    fun addSVGStyle(
        status: UInt,
        selected: Boolean,
        content: String
    ): SeatStyleConfigBuilder {
        val key = SeatStyleKey(status, selected)
        val config = SVGSeatStyleConfig(content)
        configs[key] = config
        return this
    }

    /**
     * 序列化为 JSON 数据
     * @return JSON 字节数组，如果序列化失败则返回 null
     */
    fun toJSONData(): ByteArray? {
        return try {
            val jsonString = toJSONString()
            jsonString?.toByteArray(Charsets.UTF_8)
        } catch (e: Exception) {
            null
        }
    }

    /**
     * 序列化为 JSON 字符串
     * @return JSON 字符串，如果序列化失败则返回 null
     */
    fun toJSONString(): String? {
        return try {
            val jsonArray = JSONArray()
            for ((key, config) in configs) {
                val entry = JSONObject()
                
                // 构建 key 对象
                val keyObj = JSONObject()
                keyObj.put("status", key.status.toLong())
                keyObj.put("selected", key.selected)
                entry.put("key", keyObj)
                
                // 构建 config 对象
                val configObj = JSONObject()
                configObj.put("type", config.type.value)
                
                when (config) {
                    is CircleSeatStyleConfig -> {
                        configObj.put("fill", config.fill.toARGBHex())
                        configObj.put("overlay", config.overlay.toARGBHex())
                        configObj.put("checkmark", config.checkmark.toARGBHex())
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

    /**
     * 清空所有配置
     */
    fun clear() {
        configs.clear()
    }
}
