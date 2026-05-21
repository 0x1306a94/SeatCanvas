package com.libseatcanvas

/**
 * 生成与 C++ 渲染器相同格式的座位样式 ID。
 */
object SeatRenderStyleId {
    /**
     * @param pricecode 价档 code；null 或空字符串表示无价档槽位。
     * @param status 业务自定义座位状态。
     * @param selected 是否选中。
     */
    @JvmStatic
    fun compose(pricecode: String?, status: Int, selected: Boolean): String {
        return nativeCompose(pricecode ?: "", status, selected)
    }

    private external fun nativeCompose(pricecode: String, status: Int, selected: Boolean): String
}
