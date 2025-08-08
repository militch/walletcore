plugins {
    id("java-library")
    id("maven-publish")
}

group = "com.github.militch.walletcore"
version = "1.0.0"

repositories {
    mavenCentral()
}

dependencies {
    testImplementation(platform("org.junit:junit-bom:5.10.0"))
    testImplementation("org.junit.jupiter:junit-jupiter")
}

tasks.test {
    useJUnitPlatform()
}
tasks.withType<JavaCompile> {
    options.encoding = "UTF-8"
}

val nativeBuildDir = "nativeBuild"
val nativeBuildTarget = "walletcore"

tasks.clean {
    dependsOn("cleanNativeBuildCache")
}

tasks.register("cleanNativeBuildCache") {
    group = "build"
    delete("buildNative")
    delete(nativeBuildDir)
}

tasks.register("configureCMake") {
    exec {
        val cMakeFileDir = ".."
        commandLine("cmake", "-S", cMakeFileDir, "-B", nativeBuildDir, "-DBUILD_JNI_LIB=ON")
    }
}

tasks.register("assembleNativeLibrary") {
    dependsOn("configureCMake")
    group = "build"
    doLast{
        exec {
            commandLine("cmake", "--build", nativeBuildDir, "--config","Release","--target", nativeBuildTarget)
        }
    }
}

tasks.register("copyNativeLibraryToMetaInf", Copy::class) {
    dependsOn("assembleNativeLibrary")
    val nativeLibDir = file("$nativeBuildDir/Release")
    from(nativeLibDir) {
        include("*.dll", "*.so", "*.dylib")
    }
    into(layout.buildDirectory.dir("resources/main/META-INF/native").get().asFile)
}

tasks.processResources {
    dependsOn("copyNativeLibraryToMetaInf")
}

publishing {
    publications {
        create<MavenPublication>("gpr") {
            from(components["java"])
            groupId = group.toString()
            artifactId = "walletcorej"
            version = version
            pom {
                name.set("数字货币分层确定性钱包库")
                description.set("")
                url.set("https://github.com/militch/walletcore")
            }
        }
    }
    repositories {
        maven {
            name = "GitHubPackages"
            url = uri("https://maven.pkg.github.com/militch/walletcore")
            credentials {
                username =  findProperty("gpr.user") as String? ?: ""
                password =  findProperty("gpr.key") as String? ?: ""
            }
        }
    }
}
repositories {
    maven {
        name = "GitHubPackages"
        url = uri("https://maven.pkg.github.com/militch/walletcore")
        credentials {
            username =  findProperty("gpr.user") as String? ?: ""
            password =  findProperty("gpr.key") as String? ?: ""
        }
    }
}
