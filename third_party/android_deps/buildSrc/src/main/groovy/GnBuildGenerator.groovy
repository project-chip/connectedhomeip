import java.util.regex.Pattern
import org.gradle.api.DefaultTask
import org.gradle.api.tasks.TaskAction
import org.gradle.api.artifacts.ResolvedDependency

/** Task that generates a BUILD.gn file based on dependencies in the chipDeps configuration. */
class GnBuildGenerator extends DefaultTask {
  private static final GENERATED_WARNING =
    "# === This BUILD file is auto-generated, modify the GnBuildGenerator task instead of editing this file manually. ==="
  private static final COPIED_ARTIFACTS_DIR = "artifacts"
  private static final ANDROID_DEPS_DIR = "/third_party/android_deps"

  @TaskAction
  void main() {
    def chipDepsConfiguration = project.configurations.chipDeps
    def chipRoot = System.getenv("PW_PROJECT_ROOT");
    def absoluteAndroidDepsDir = "$chipRoot/$ANDROID_DEPS_DIR";

    def buildFile = new File("$absoluteAndroidDepsDir/BUILD.gn");
    if (!buildFile.exists()) {
      buildFile.createNewFile()
    }

    def stringBuilder = new StringBuilder()
    def copyrightHeader = new File("$absoluteAndroidDepsDir/copyright_header.txt")
    stringBuilder.append(copyrightHeader.text + "\n\n")

    stringBuilder.append(GENERATED_WARNING + "\n\n")

    stringBuilder.append("""\
    import("//build_overrides/chip.gni")
    import("\${chip_root}/build/chip/java/rules.gni")
    """.stripIndent())

    def allDeps = new HashSet<ResolvedDependency>()
    chipDepsConfiguration.resolvedConfiguration.firstLevelModuleDependencies.each {
      allDeps += it
      it.children.each {
        allDeps += it
      }
    }

    allDeps.each {
      def artifact = it.moduleArtifacts[0]
      if (it.moduleArtifacts.size() > 1 || artifact.extension != "jar") {
        throw new IllegalStateException(
          "Multiple top-level module artifacts or unsupported"
            + " artifact extension (artifact count = $it.moduleArtifacts.size(), extension"
            + " = $artifact.extension).")
      }

      def deps = []
      it.children.each {
        deps += "\":$it.module.id.name\""
      }

      stringBuilder.append("\n")
      stringBuilder.append("""\
      java_prebuilt("$it.module.id.name") {
        jar_path = "$COPIED_ARTIFACTS_DIR/${artifact.file.name}"
      """.stripIndent())

      if (!deps.isEmpty()) {
        stringBuilder.append("  deps = $deps\n")
      }
      stringBuilder.append("}\n")
    }

    buildFile.write(stringBuilder.toString())    
  }
}
