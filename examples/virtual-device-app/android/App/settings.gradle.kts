pluginManagement {
    repositories {
        google()
        mavenCentral()
        gradlePluginPortal()
    }
}
dependencyResolutionManagement {
    repositoriesMode.set(RepositoriesMode.FAIL_ON_PROJECT_REPOS)
    repositories {
        google()
        mavenCentral()
    }
}
rootProject.name = "VirtualDeviceApp"
include(":app")
include(":core:common")
include(":core:data")
include(":core:domain")
include(":core:matter")
include(":core:model")
include(":core:ui")
include(":feature:control")
include(":feature:closure")
include(":feature:main")
include(":feature:qrcode")
include(":feature:setup")
