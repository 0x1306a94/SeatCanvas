pluginManagement {
    repositories {
        val useAliyunMirror = java.util.Properties().let { properties ->
            val localPropertiesFile = java.io.File(settings.rootDir, "local.properties")
            if (localPropertiesFile.exists()) {
                localPropertiesFile.inputStream().use { properties.load(it) }
            }
            properties.getProperty("useAliyunMirror") == "true"
        }
        if (useAliyunMirror) {
            maven { url = uri("https://maven.aliyun.com/repository/google") }
            maven { url = uri("https://maven.aliyun.com/repository/public") }
            maven { url = uri("https://maven.aliyun.com/repository/gradle-plugin") }
        }
        google {
            content {
                includeGroupByRegex("com\\.android.*")
                includeGroupByRegex("com\\.google.*")
                includeGroupByRegex("androidx.*")
            }
        }
        mavenCentral()
        gradlePluginPortal()
    }
}
dependencyResolutionManagement {
    repositoriesMode.set(RepositoriesMode.FAIL_ON_PROJECT_REPOS)
    repositories {
        val useAliyunMirror = java.util.Properties().let { properties ->
            val localPropertiesFile = java.io.File(settings.rootDir, "local.properties")
            if (localPropertiesFile.exists()) {
                localPropertiesFile.inputStream().use { properties.load(it) }
            }
            properties.getProperty("useAliyunMirror") == "true"
        }
        if (useAliyunMirror) {
            maven { url = uri("https://maven.aliyun.com/repository/google") }
            maven { url = uri("https://maven.aliyun.com/repository/public") }
        }
        google()
        mavenCentral()
    }
}

rootProject.name = "SeatCanvasSample"
include(":app")
include(":libseatcanvas")
