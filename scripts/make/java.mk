#
#    Copyright (c) 2014-2017 Nest Labs, Inc.
#    Copyright (c) 2018 Google LLC
#	   Copyright (c) 2020 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

#
#    Description:
#      This file contains make rules for compiling Java code and building jars.
#

#
#    Java Rules Usage
#    ================
#
#    Projects wishing to build Java code should include this file at the *end*
#    of their Makefile.am files. Additionally, projects must declare a set of
#    automake local dependencies to cause Java code to be compile for standard
#    targets. E.g.:
#
#        include $(abs_top_srcdir)/scripts/make/java.mk
#
#        all-local: all-JARS
#        install-data-local: install-data-JARS
#        uninstall-local: uninstall-JARS
#        mostlyclean-local: mostlyclean-local-JARS
#
#    Jars to be built must be declared in the JARS variable *before* java.mk is
#    included, e.g.:
#
#        JARS                        = \
#            SetupPayloadParser.jar    \
#            $(NULL)
#
#    The building of individual jars is controlled by variables that begin
#    with the name of the jar file.  E.g. SetupPayloadParser_jar_JFLAGS.  These
#    variable are:
#
#    <name>_JAVA_SRCS -- The list of java source files to be compile and
#        included in the jar. The file paths are relative to the "src"
#        subdirectory, which by default is located in the same directory as the
#        Makefile.am.  E.g.:
#
#        SetupPayloadParser_jar_JAVA_SRCS                = \
#            chip/setuppayload/SetupPayloadParser.java     \
#            chip/setuppayload/SetupPayload.java           \
#            $(NULL)
#
#    <name>_JFLAGS -- Flags to be passed to the Java compiler.
#
#    <name>_JAVA_MAIN_CLASS -- Specifies the application entry point class
#        for the jar.  Must be a fully qualified class name.
#
#    <name>_JAVA_CLASSPATHS -- Additional class paths and/or jars to be passed
#        to the Java compiler.  Multiple entries must be separated by colons.
#
#    <name>_JAVA_EXTRA_SRC_DIRS -- Additional directories containing Java source
#        files that are used to satisfy references during compilation, but are
#        not included in the compile output.  Multiple entries must be separated
#        by colons.
#
#    <name>_JAVA_NATIVE_LIB -- The name of a native library to be included in a
#        platform-specific native subdirectory with the jar file.  The name given
#        must be both a make target, on which the jar will depend, and the name
#        of a shared-library file located in the .libs subdirectory of the module's
#        build directory.  This should typically be the name of a libtool .la file
#        which is built by other rules in the module's Makefile.am.
#
#    <name>_JAVA_STRIP_NATIVE_LIB -- Set to "true" to cause the native library to
#        be stripped before being included in the jar file.
#
#    <name>_JAVA_SRC_DIR -- The directory containing the Java source files.
#        Defaults to the global value $(JAVA_SRC_DIR), which in turn defaults to
#        the module's "src" directory, i.e. $(srcdir)/src.
#
#    <name>_JAVA_BIN_DIR -- The directory into which the compile Java class files.
#        should be placed. Defaults to the global value $(JAVA_BIN_DIR), which in
#        turn defaults to the "bin" subdirectory in the module's build directory,
#        i.e. $(builddir)/bin.
#
#    <name>_JAVA_INSTALL_DIR -- The directory into which the generated jar file is
#        installed.  Defaults to the global value $(JAVA_INSTALL_DIR), which in turn
#        defaults to $(datadir)/java.
#
#    <name>_MAVEN_PROJECT_FILE -- The name of a Maven project file (pom) to be
#        included in the jar file.  The pom file will be scanned for groupId and
#        artifactId fields, which will be used to form the path of the pom file
#        within the jar file; i.e. META-INF/maven/<groupId>/<artifactId>/pom.xml.
#

AM_V_JAR      = $(am__v_JAR_$(V))
am__v_JAR_    = $(am__v_JAR_$(AM_DEFAULT_VERBOSITY))
am__v_JAR_0   = @echo "  JAR     " $@;
am__v_JAR_1   =

AM_V_JAVAC    = $(am__v_JAVAC_$(V))
am__v_JAVAC_  = $(am__v_JAVAC_$(AM_DEFAULT_VERBOSITY))
am__v_JAVAC_0 = @echo "  JAVAC   ";
am__v_JAVAC_1 =

JAVA_SRC_DIR ?= $(srcdir)/src

JAVA_BIN_DIR ?= $(builddir)/bin

JAVA_INSTALL_DIR ?= $(datadir)/java

JAVA_NATIVE_LIB_HOST_OS = $(shell echo $(host_os) | sed -n -e 's/linux-gnu/linux/; s/darwin[0-9.]*/darwin/; p' )

JAVA_NATIVE_LIB_HOST_CPU = $(host_cpu)

JAVA_NATIVE_BIN_DIR ?= $(builddir)/bin/native/$(JAVA_NATIVE_LIB_HOST_OS)/$(JAVA_NATIVE_LIB_HOST_CPU)

HOST_SHARED_LIB_EXT = $(shell $(LIBTOOL) --config | sed -n -e '/^shrext_cmds=/ { s/^shrext_cmds=/echo /; s/"//g; s/\\`/`/g; p; }' | sh )

JAVA_STRIP_NATIVE_LIB = false

ifeq ($(JAVA_NATIVE_LIB_HOST_OS),darwin)
else
endif

define JarRules
# Arguments:
#    $(1): jar variable prefix
#    $(2): jar file name

$(1)_JAVA_SRC_DIR ?= $(JAVA_SRC_DIR)
$(1)_JAVA_BIN_DIR ?= $(JAVA_BIN_DIR)
$(1)_JAVA_INSTALL_DIR ?= $(JAVA_INSTALL_DIR)
$(1)_JAVA_NATIVE_BIN_DIR ?= $(JAVA_NATIVE_BIN_DIR)

ifeq ($(JAVA_NATIVE_LIB_HOST_OS),darwin)
$(1)_JAVA_STRIP_NATIVE_LIB = false
else
$(1)_JAVA_STRIP_NATIVE_LIB ?= $(JAVA_STRIP_NATIVE_LIB)
endif

$(1)_JAVA_ABS_SRCS ?= $$(addprefix $$($(1)_JAVA_SRC_DIR)/,$$($(1)_JAVA_SRCS))

ifdef $(1)_MAVEN_PROJECT_FILE
$(1)_ABS_MAVEN_PROJECT_FILE := $$(addprefix $(srcdir)/,$$($(1)_MAVEN_PROJECT_FILE))
$(1)_MAVEN_GROUP_ID ?= $$(shell sed -n -e '/<groupId>/ { s@.*<groupId>\(.*\)</groupId>.*@\1@; p; }' $$($(1)_ABS_MAVEN_PROJECT_FILE) )
$(1)_MAVEN_ARTIFACT_ID ?= $$(shell sed -n -e '/<artifactId>/ { s@.*<artifactId>\(.*\)</artifactId>.*@\1@; p; }' $$($(1)_ABS_MAVEN_PROJECT_FILE) )
endif

# Compile Java source files
$(2).classes.stamp: $$($(1)_JAVA_ABS_SRCS) | $$($(1)_JAVA_BIN_DIR)
	$$(AM_V_JAVAC)$$(JAVAC) $$(JFLAGS) $$($(1)_JFLAGS) -d $$($(1)_JAVA_BIN_DIR) -cp :$$($(1)_JAVA_CLASSPATHS) -sourcepath $$($(1)_JAVA_SRC_DIR):$$($(1)_JAVA_EXTRA_SRC_DIRS) $$($(1)_JAVA_ABS_SRCS)
	$$(AM_V_at)touch $$(@)

# Create jar
$(2): $(2).classes.stamp $$($(1)_JAVA_NATIVE_LIB) | $$(if $$($(1)_JAVA_NATIVE_LIB),$$($(1)_JAVA_NATIVE_BIN_DIR))
ifdef $(1)_JAVA_NATIVE_LIB
	cp .libs/$$(patsubst %.la,%$$(HOST_SHARED_LIB_EXT),$$($(1)_JAVA_NATIVE_LIB)) $$($(1)_JAVA_NATIVE_BIN_DIR)
	if $$($(1)_JAVA_STRIP_NATIVE_LIB); then $(STRIP) $$($(1)_JAVA_NATIVE_BIN_DIR)/$$(patsubst %.la,%$$(HOST_SHARED_LIB_EXT),$$($(1)_JAVA_NATIVE_LIB)); fi
endif
ifdef $(1)_MAVEN_PROJECT_FILE
	mkdir -p $$($(1)_JAVA_BIN_DIR)/META-INF/maven/$$($(1)_MAVEN_GROUP_ID)/$$($(1)_MAVEN_ARTIFACT_ID)
	cp $$($(1)_ABS_MAVEN_PROJECT_FILE) $$($(1)_JAVA_BIN_DIR)/META-INF/maven/$$($(1)_MAVEN_GROUP_ID)/$$($(1)_MAVEN_ARTIFACT_ID)/pom.xml
endif
	$$(AM_V_JAR)$$(JAR) c$$(if $$($(1)_JAVA_MAIN_CLASS),e)f $$($(1)_JAVA_MAIN_CLASS) $$(@) -C $$($(1)_JAVA_BIN_DIR) .

# Create bin directory
$$($(1)_JAVA_BIN_DIR):
	$$(create-directory)

# Create native bin directory
$$($(1)_JAVA_NATIVE_BIN_DIR):
	$$(create-directory)

# Create install directory
$$(DESTDIR)$$($(1)_JAVA_INSTALL_DIR):
	$$(create-directory)

.PHONY: all-JARS install-data-JARS install-data-$(1) \
        uinstall-data-JARS uninstall-data-$(1) \
        mostlyclean-local-JARS mostlyclean-local-$(1)

# Install jar
install-data-$(1): $(2) | $$(DESTDIR)$$($(1)_JAVA_INSTALL_DIR)
	@$$(NORMAL_INSTALL)
	echo "  INSTALL  $(2)"; \
	$$(INSTALL_DATA) "$(2)" "$$(DESTDIR)$$($(1)_JAVA_INSTALL_DIR)/$(2)"; \

# Uninstall jar
uninstall-data-$(1):
	@$$(NORMAL_UNINSTALL)
	rm -f "$$(DESTDIR)$$($(1)_JAVA_INSTALL_DIR)/$(2)"; \

# Clean
mostlyclean-local-$(1):
	-$$(AM_V_at)rm -rf $$($(1)_JAVA_BIN_DIR) $(2) $(2).classes.stamp

all-JARS: $(2)

install-data-JARS: install-data-$(1)

uninstall-JARS: uninstall-data-$(1)

mostlyclean-local-JARS: mostlyclean-local-$(1)

endef # JarRules

# Instantiate jar build rules for each declared jar.
$(foreach jar,$(JARS),$(eval $(call JarRules,$(basename $(jar))_jar,$(jar))))
