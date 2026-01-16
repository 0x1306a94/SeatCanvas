plugins {
    alias(libs.plugins.android.library)
    alias(libs.plugins.kotlin.android)
}

android {
    namespace = "com.libseatcanvas"
    ndkVersion = rootProject.ext["ndkVersion"].toString()
    compileSdk = rootProject.ext["compileSdkVersion"].toString().toInt()

    packaging {
        jniLibs.useLegacyPackaging = true
    }

    defaultConfig {
        minSdk = rootProject.ext["minSdk"].toString().toInt()

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
        consumerProguardFiles("consumer-rules.pro")

        ndk {
//            if (project.hasProperty("arm64-only")) {
//                abiFilters.add("arm64-v8a")
//            } else {
//                abiFilters.add("armeabi-v7a")
//                abiFilters.add("arm64-v8a")
//                abiFilters.add("x86_64")
//            }
            abiFilters.add("arm64-v8a")
        }

        externalNativeBuild {
            cmake {
                val cmakeArgs: String? = project.findProperty("cmakeArgs") as String?
                arguments.add("-DANDROID_STL=c++_static")
                if (!cmakeArgs.isNullOrEmpty()) {
                    arguments.addAll(cmakeArgs.split(" "))
                }
            }
        }
    }

    externalNativeBuild {
        cmake {
            path("../../../CMakeLists.txt")
            version = "3.22.1"
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules.pro")

            ndk {
               debugSymbolLevel = "none"
            }

            externalNativeBuild {
                cmake {
                    arguments("-DCMAKE_BUILD_TYPE=Release")
                }
            }
        }

        debug {
            isJniDebuggable = true

            ndk {
                debugSymbolLevel = "full"
            }

            externalNativeBuild {
                cmake {
                    arguments("-DCMAKE_BUILD_TYPE=RelWithDebInfo")
                    arguments("-DENABLE_TIME_PROFILER=ON")
                }
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
}

dependencies {

    implementation(libs.androidx.core.ktx)
    implementation(libs.androidx.appcompat)
    implementation(libs.material)
    testImplementation(libs.junit)
    androidTestImplementation(libs.androidx.junit)
    androidTestImplementation(libs.androidx.espresso.core)
}