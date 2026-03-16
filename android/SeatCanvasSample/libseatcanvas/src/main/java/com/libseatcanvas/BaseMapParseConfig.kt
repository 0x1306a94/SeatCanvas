package com.libseatcanvas

import org.json.JSONArray
import org.json.JSONObject

interface BaseMapParseConfig {
    fun serializeToByteArray(): ByteArray?
}

class SVGBaseMapParseConfig(
    val zoneIdAttributeNames: List<String> = listOf("zoneId")
) : BaseMapParseConfig {
    override fun serializeToByteArray(): ByteArray? {
        val json = JSONObject()
        json.put("zoneIdAttributeNames", JSONArray(zoneIdAttributeNames))
        return json.toString().toByteArray()
    }
}
