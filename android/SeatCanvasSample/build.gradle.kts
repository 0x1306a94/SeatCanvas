// Top-level build file where you can add configuration options common to all sub-projects/modules.
plugins {
    alias(libs.plugins.android.application) apply false
    alias(libs.plugins.kotlin.android) apply false
    alias(libs.plugins.android.library) apply false
}

ext["appName"] = "SeatCanvasSample"
ext["compileSdkVersion"] = 36
ext["targetSdk"] = 36
ext["minSdk"] = 24
ext["versionCode"] = 1
ext["versionName"] = "1.0.0"
ext["ndkVersion"] = "26.3.11579264"
