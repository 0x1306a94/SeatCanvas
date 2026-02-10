package com.libseatcanvas

/**
 * 缩放级别配置，对应 C++ 层的 ZoomLevelConfig。
 *
 * @property seat 最近视角（座位级别）
 * @property row 行/小块级视角
 * @property zone 区域级视角
 * @property venue 全场/概览级视角
 */
data class ZoomLevel(
    val seat: Float,
    val row: Float,
    val zone: Float,
    val venue: Float
)

