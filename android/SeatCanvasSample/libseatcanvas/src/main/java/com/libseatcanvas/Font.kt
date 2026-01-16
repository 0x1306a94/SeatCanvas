package com.libseatcanvas

import java.io.FileInputStream
import java.io.IOException
import android.util.Xml
import org.xmlpull.v1.XmlPullParser
import org.xmlpull.v1.XmlPullParserException
import java.io.File
import java.util.regex.Pattern

class Font(var fontFamily: String, var fontStyle: String) {

    private class FontConfig {
        var language: String = ""
        var fileName: String = ""
        var ttcIndex: Int = 0
        var weight: Int = 400
    }


    companion object {


        private const val DefaultLanguage: String = "zh-Hans"
        private const val SystemFontConfigPath_Lollipop: String = "/system/etc/fonts.xml"
        private const val SystemFontConfigPath_JellyBean: String = "/system/etc/fallback_fonts.xml"
        private const val SystemFontPath: String = "/system/fonts/"
        private val FILENAME_WHITESPACE_PATTERN: Pattern = Pattern.compile("^[ \\n\\r\\t]+|[ \\n\\r\\t]+$")

        private external fun nativeSetFallbackFontPaths(fontNameList: Array<String>, ttcIndices: IntArray)

        @Throws(XmlPullParserException::class, IOException::class)
        private fun parseLollipop(): Array<FontConfig> {
            val file = File(SystemFontConfigPath_Lollipop)
            if (!file.exists()) {
                return emptyArray()
            }

            FileInputStream(file).use { fis ->
                val parser = Xml.newPullParser()
                parser.setInput(fis, null)
                parser.nextTag()
                return readFamilies(parser)
            }
        }

        @Throws(XmlPullParserException::class, IOException::class)
        private fun readFamilies(parser: XmlPullParser): Array<FontConfig> {
            val fallbackList = mutableListOf<FontConfig>()

            parser.require(XmlPullParser.START_TAG, null, "familyset")
            while (parser.next() != XmlPullParser.END_TAG) {
                if (parser.eventType != XmlPullParser.START_TAG) continue

                when (parser.name) {
                    "family" -> readFamily(parser, fallbackList)
                    else -> skip(parser)
                }
            }
            return fallbackList.toTypedArray()
        }

        @Throws(XmlPullParserException::class, IOException::class)
        private fun readFamily(parser: XmlPullParser, fontList: MutableList<FontConfig>) {
            val lang = parser.getAttributeValue(null, "lang")

            val fonts = mutableListOf<FontConfig>()

            while (parser.next() != XmlPullParser.END_TAG) {
                if (parser.eventType != XmlPullParser.START_TAG) continue

                when (parser.name) {
                    "font" -> fonts.add(readFont(parser))
                    else -> skip(parser)
                }
            }

            if (fonts.isEmpty()) return

            val regularFont = fonts.firstOrNull { it.weight == 400 } ?: fonts[0]

            if (regularFont.fileName.isNotEmpty()) {
                regularFont.language = lang ?: ""
                fontList.add(regularFont)
            }
        }

        @Throws(XmlPullParserException::class, IOException::class)
        private fun readFont(parser: XmlPullParser): FontConfig {
            val font = FontConfig()

            val indexStr = parser.getAttributeValue(null, "index")
            font.ttcIndex = indexStr?.toIntOrNull() ?: 0

            val weightStr = parser.getAttributeValue(null, "weight")
            font.weight = weightStr?.toIntOrNull() ?: 400

            val filename = StringBuilder()

            while (parser.next() != XmlPullParser.END_TAG) {
                when (parser.eventType) {
                    XmlPullParser.TEXT -> filename.append(parser.text)
                    XmlPullParser.START_TAG -> skip(parser)
                }
            }

            font.fileName = SystemFontPath + FILENAME_WHITESPACE_PATTERN.matcher(filename.toString()).replaceAll("")

            return font
        }

        @Throws(XmlPullParserException::class, IOException::class)
        private fun skip(parser: XmlPullParser) {
            var depth = 1
            while (depth > 0) {
                when (parser.next()) {
                    XmlPullParser.START_TAG -> depth++
                    XmlPullParser.END_TAG -> depth--
                }
            }
        }

        @Throws(XmlPullParserException::class, IOException::class)
        private fun parseJellyBean(): Array<FontConfig> {
            val file = File(SystemFontConfigPath_JellyBean)
            if (!file.exists()) return emptyArray()

            FileInputStream(file).use { fis ->
                val parser = Xml.newPullParser()
                parser.setInput(fis, null)
                parser.nextTag()
                return readFamiliesJellyBean(parser)
            }
        }

        @Throws(XmlPullParserException::class, IOException::class)
        private fun readFamiliesJellyBean(parser: XmlPullParser): Array<FontConfig> {
            val fallbackList = mutableListOf<FontConfig>()

            parser.require(XmlPullParser.START_TAG, null, "familyset")

            while (parser.next() != XmlPullParser.END_TAG) {
                if (parser.eventType != XmlPullParser.START_TAG) continue

                val tag = parser.name
                if (tag == "family") {
                    while (parser.next() != XmlPullParser.END_TAG) {
                        if (parser.eventType != XmlPullParser.START_TAG) continue

                        when (parser.name) {
                            "fileset" -> readFileset(parser, fallbackList)
                            else -> skip(parser)
                        }
                    }
                } else {
                    skip(parser)
                }
            }

            return fallbackList.toTypedArray()
        }

        @Throws(XmlPullParserException::class, IOException::class)
        private fun readFileset(parser: XmlPullParser, fontList: MutableList<FontConfig>) {
            val fonts = mutableListOf<FontConfig>()

            while (parser.next() != XmlPullParser.END_TAG) {
                if (parser.eventType != XmlPullParser.START_TAG) continue

                when (parser.name) {
                    "file" -> fonts.add(readFont(parser))
                    else -> skip(parser)
                }
            }

            if (fonts.isEmpty()) return

            val regularFont = fonts.firstOrNull { it.weight == 400 } ?: fonts[0]

            if (regularFont.fileName.isNotEmpty()) {
                fontList.add(regularFont)
            }
        }

        private fun getFontByLanguage(fontList: Array<FontConfig>, language: String): FontConfig? {
            val lang = language.lowercase()
            return fontList.firstOrNull { it.language.lowercase() == lang }
        }

        private fun addFont(fontConfig: FontConfig, fontPaths: MutableList<String>, ttcList: MutableList<Int>) {
            if (fontPaths.contains(fontConfig.fileName)) return

            val file = File(fontConfig.fileName)
            if (!file.exists()) return

            fontPaths.add(fontConfig.fileName)
            ttcList.add(fontConfig.ttcIndex)
        }

        private var systemFontLoaded = false

        @JvmStatic
        fun RegisterFallbackFonts() {
            if (systemFontLoaded) return
            systemFontLoaded = true

            val fontList = try {
                val lollipopFile = File(SystemFontConfigPath_Lollipop)
                if (lollipopFile.exists()) {
                    parseLollipop()
                } else {
                    parseJellyBean()
                }
            } catch (e: Exception) {
                e.printStackTrace()
                emptyArray()
            }

            val fontPaths = mutableListOf<String>()
            val ttcList = mutableListOf<Int>()

            getFontByLanguage(fontList, DefaultLanguage)?.let { addFont(it, fontPaths, ttcList) }

            for (font in fontList) {
                addFont(font, fontPaths, ttcList)
            }

            if (fontPaths.isNotEmpty()) {
                val fontPathList = fontPaths.toTypedArray()
                val ttcIndices = ttcList.toIntArray()
                Font.SetFallbackFontPaths(fontPathList, ttcIndices)
            }
        }

        @JvmStatic
        fun SetFallbackFontPaths(fontNameList: Array<String>, ttcIndices: IntArray) {
            nativeSetFallbackFontPaths(fontNameList, ttcIndices)
        }

        init {
            System.loadLibrary("SeatCanvas")
        }
    }

}