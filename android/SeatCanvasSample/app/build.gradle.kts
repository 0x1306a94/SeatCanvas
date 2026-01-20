import java.time.LocalDateTime
import java.time.format.DateTimeFormatter

plugins {
    alias(libs.plugins.android.application)
    alias(libs.plugins.kotlin.android)
    id("kotlin-parcelize")
}

android {

//    packaging {
//        jniLibs {
//            useLegacyPackaging = true
//        }
//    }

    signingConfigs {
        create("release") {
            storeFile = file("../SeatCanvasSample.jks")
            storePassword = "123456"
            keyAlias = "key0"
            keyPassword = "123456"
        }
    }

    namespace = "com.seatcanvas.sample"
    ndkVersion = rootProject.ext["ndkVersion"].toString()
    compileSdk = rootProject.ext["compileSdkVersion"].toString().toInt()

    defaultConfig {
        applicationId = "com.seatcanvas.sample"
        minSdk = rootProject.ext["minSdk"].toString().toInt()
        targetSdk = rootProject.ext["targetSdk"].toString().toInt()
        versionCode = rootProject.ext["versionCode"].toString().toInt()
        versionName = rootProject.ext["versionName"].toString()

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules.pro")
            signingConfig = signingConfigs.getByName("release")

            ndk {
                debugSymbolLevel = "none"
            }
        }

        debug {
            signingConfig = signingConfigs.getByName("debug")
            isJniDebuggable = true

            ndk {
                debugSymbolLevel = "full"
            }
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }
    kotlinOptions {
        jvmTarget = "11"
    }
    buildFeatures {
        viewBinding = true
    }

    android.applicationVariants.all {
        outputs.all {
            if (this is com.android.build.gradle.internal.api.ApkVariantOutputImpl) {
                val config = project.android.defaultConfig
                val versionName = config.versionName
//                val formatter = DateTimeFormatter.ofPattern("yyyyMMddHHmm")
//                val createTime = LocalDateTime.now().format(formatter)
                this.outputFileName = "${rootProject.ext["appName"].toString()}_${this.name}_${versionName}.apk"
            }
        }
    }
}

dependencies {

    implementation(libs.androidx.core.ktx)
    implementation(libs.androidx.appcompat)
    implementation(libs.material)
    implementation(libs.androidx.constraintlayout)
    implementation(libs.androidx.navigation.fragment.ktx)
    implementation(libs.androidx.navigation.ui.ktx)
    implementation(libs.gson)
    testImplementation(libs.junit)
    androidTestImplementation(libs.androidx.junit)
    androidTestImplementation(libs.androidx.espresso.core)

    implementation(project(":libseatcanvas"))
}