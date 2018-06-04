#!/usr/bin/env python
#
# Copyright (C) 2018 Wind River Systems, Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at:
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#
# --------------------------------------------------------------------
# Description:
# This script downloads, builds and installs project dependencies for the
# device-cloud-lib into sub-directory called "deps".  For more information see
# README.md.
#
# Usage:
#  $ ./build.py --help
# --------------------------------------------------------------------

from __future__ import print_function
import argparse
import os
import shutil
import subprocess
import sys
import urllib

# ----------------
# GLOBAL VARIABLES
# ----------------
PROJECT_NAME = "device-cloud-lib"

# ----------------
# GLOBAL FUNCTIONS
# ----------------

# for printing errors (works in python2 & python 3)
def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

# Determines path to a system file
# (similar to "which" command Linux or "where" command on Windows)
def which(file):
    exts=[""]
    dirs=[""]
    if os.environ.get("PATHEXT") is not None:
        for pathext in os.environ["PATHEXT"].split(os.pathsep):
            exts.append(pathext)

    if os.environ.get("PATH") is not None:
        for path in os.environ["PATH"].split(os.pathsep):
            dirs.append(path)

    for dir in dirs:
        for ext in exts:
            test_path=os.path.join(dir, file)
            if ext:
                test_path = test_path + ext
            if os.path.exists(test_path):
                return test_path
    return None

# ----------------
# OBJECTS
# ----------------
class BuildProject():
    def __init__(self, project_name,
            download_dir=None,
            download_url=None,
            download_tag=None,
            patch_dir=None,
            patches=None,
            config_pre=None,
            config_tool=None,
            config_flags=[],
            build_dir=None,
            build_tool=None,
            build_type="Release",
            build_target="all",
            build_flags=None,
            install_dir=None,
            install_flags=None,
            install_target=None,
            test_flags=None,
            test_target=None):
        self.project_name = project_name
        self.download_dir = download_dir
        self.download_url = download_url
        self.download_tag = download_tag
        self.patch_dir = patch_dir
        self.patches = patches
        self.config_pre = config_pre
        self.config_tool = config_tool
        self.config_flags = config_flags
        self.build_dir = build_dir
        self.build_tool = build_tool
        self.build_type = build_type
        self.build_flags = build_flags
        self.build_target = build_target
        self.install_dir = install_dir
        self.install_flags = install_flags
        self.install_target = install_target
        self.test_flags = test_flags
        self.test_target = test_target

    @staticmethod
    def __get_path(exe_name):
        known_paths = { }
        if exe_name not in known_paths:
            known_paths[exe_name] = which(exe_name)
            if known_paths[exe_name] is None:
                raise RuntimeError( "Failed to find %s" % exe_name )
        return known_paths[exe_name]

    def __do_stage(self, stage):
        args = None
        target = None
        if stage == "build":
            target = self.build_target
            args = self.build_flags
        elif stage == "install":
            target = self.install_target
            args = self.install_flags
        elif stage == "test":
            target = self.test_target
            args = self.test_flags

        if isinstance(args, basestring):
            args = [ args ]

        if self.build_tool is not None and self.download_dir is not None and target is not None:
            print("%sing %s..." % (stage.capitalize(), self.project_name))
            cur_dir = os.getcwd()

            if self.download_dir is not None:
                os.chdir(self.download_dir)

            # out-of-source build support
            if self.build_dir:
                os.chdir(self.build_dir)

            if callable(target):
                target(self.install_dir)
            else:
                cmd = []
                cmd.append(self.__get_path(self.build_tool))
                if self.build_tool is "cmake":
                    cmd.append("--build")
                    cmd.append(".")
                    cmd.append("--config")
                    cmd.append(self.build_type)
                    if stage != "build":
                        cmd.append("--target")
                        cmd.append(target)
                    if args is not None:
                        cmd.append("--")
                elif (self.build_tool == "make" or self.build_tool == "nmake"):
                    if stage != "build":
                        cmd.append(stage)

                if args is not None:
                    if self.build_tool == "make":
                        cmd.append(" ".join(args))
                    else:
                        cmd += args
                subprocess.check_call(cmd)

            os.chdir(cur_dir)

    def build(self):
        self.__do_stage("build")

    def configure(self):
        cur_dir = os.getcwd()
        if self.download_dir is not None:
            os.chdir(self.download_dir)
        src_dir = os.getcwd()

        # Support callable pre-config method
        if self.config_pre:
            self.config_pre(self.install_dir)

        if self.config_tool is not None:
            print("Configuring %s..." % self.project_name)

            # out-of-source build support
            if self.build_dir:
                if not os.path.exists(self.build_dir):
                    os.mkdir(self.build_dir)
                os.chdir(self.build_dir)

            cmd = [self.__get_path(self.config_tool)]
            # common cmake configuration options
            if self.config_tool == "cmake":
                if os.path.isfile("CMakeCache.txt"):
                    os.remove("CMakeCache.txt")
                cmd += self.config_flags
                if self.build_type is not None:
                    cmd.append("-DCMAKE_BUILD_TYPE:STRING=%s" % self.build_type)
                if self.install_dir is not None:
                    cmd.append("-DCMAKE_INSTALL_PREFIX:PATH=%s" % self.install_dir)
                cmd.append(os.path.relpath(src_dir, os.getcwd()))
            elif self.config_flags is not None:
                cmd += self.config_flags
            subprocess.check_call(cmd)

        os.chdir(cur_dir)

    def download(self):
        if self.download_url is not None:
            print( "Downloading %s..." % self.project_name )

            cmd = []
            download_dir = self.download_dir
            if download_dir is None:
                download_dir = os.path.splitext(self.download_url.rsplit('/', 1)[-1])[0]
                if download_dir.endswith(".tar"):
                        download_dir = download_dir.rsplit('.',1)[0]

            if self.download_url.endswith(".git") or self.download_url.startswith("git://"):
                if not os.path.exists(download_dir):
                    # Clone repository, if it doesn't exist
                    cmd.append(self.__get_path("git"))
                    cmd.append("clone")
                    if (self.download_tag is not None and
                        self.download_tag.startswith("branch/")):
                        cmd.append("-b")
                        cmd.append(self.download_tag[7:])
                    cmd.append(self.download_url)
                    cmd.append(download_dir)
                    subprocess.check_call(cmd)
                    os.chdir(download_dir)

                    # if there's a tag, change to that
                    if (self.download_tag is not None and
                        not self.download_tag.startswith("branch/")):
                        subprocess.check_call([self.__get_path("git"),
                            "checkout", "-b", "build_branch",
                            self.download_tag])
                else:
                    # Fetch latest changes, if it exists
                    os.chdir(download_dir)

                    cmd.append(self.__get_path("git"))
                    cmd.append("fetch")
                    cmd.append("origin")
                    subprocess.check_call(cmd)

                    cmd = []
                    cmd.append(self.__get_path("git"))
                    cmd.append("reset")
                    cmd.append("--hard")
                    if self.download_tag is not None:
                        if self.download_tag.startswith("branch/"):
                            cmd.append("origin/%s" % self.download_tag[7:])
                        else:
                            cmd.append(self.download_tag)
                    else:
                        cmd.append("origin/master")
                    subprocess.check_call(cmd)

                    # Remove any new files
                    #cmd = []
                    #cmd.append(self.__get_path("git"))
                    #cmd.append("clean")
                    #cmd.append("-f")
                    #subprocess.check_call(cmd)

                os.chdir( ".." )

            else:
                file_name = self.download_url.rsplit('/', 1)[-1]
                urllib.urlretrieve(self.download_url, file_name)

                if (self.download_url.endswith( ".tar" ) or
                    self.download_url.endswith( ".tar.gz" ) or
                    self.download_url.endswith( ".tar.xz" )):

                    # move un-tared directory is not same as name of
                    # default tar location
                    tar_dir = os.path.splitext(self.download_url.rsplit('/', 1)[-1])[0]
                    if tar_dir.endswith(".tar"):
                        tar_dir = tar_dir.rsplit('.',1)[0]

                    # Remove destination directory, if it exists
                    if not os.path.exists(download_dir):
                        if os.name == "nt": # on Windows use 7-zip to extract
                            zip_path = os.path.join("%HOMEDRIVE%", "Program Files", "7-zip", "7z.EXE")
                            zip_path = os.path.expandvars(zip_path)
                            if not os.path.exists(zip_path):
                                zip_path = self.__get_path("7z")
                            # Recursively untar archives (.tar.gz -> .tar)
                            while ".tar" in file_name:
                                cmd = [zip_path]
                                cmd.append("x")
                                cmd.append(file_name)
                                subprocess.check_call(cmd)
                                file_name = file_name.rsplit('.',1)[0]
                        else: # everything else use 'tar' cmd
                            cmd.append(self.__get_path("tar"))
                            if self.download_url.endswith( ".tar.xz" ):
                                cmd.append("-xvf")
                            elif self.download_url.endswith( ".tar.gz" ):
                                cmd.append("-xzf")
                            else:
                                cmd.append("-xf")
                            cmd.append(file_name)
                            subprocess.check_call(cmd)

                        if self.download_dir is not None and download_dir != tar_dir:
                            shutil.move(tar_dir, download_dir)
                else:
                    raise RuntimeError(
                        "unable to determine extraction method for: %s" % self.download_url )

            # Save the directory where the code now is
            if self.download_dir is None:
                self.download_dir = download_dir

    def install(self):
        self.__do_stage( "install" )

    def patch(self):
        if self.patches is not None:
            print( "Patching %s..." % self.project_name )

            cur_dir = os.getcwd()
            if self.download_dir is not None:
                os.chdir(self.download_dir)

            if not os.path.exists(".git"):
                cmd = [self.__get_path("git"), "init"]
                subprocess.check_call(cmd)

            git_patch_flags = ["--ignore-space-change", "--ignore-whitespace",
                    "--unsafe-paths", "--whitespace=nowarn"]
            for patch_file in self.patches:
                patch_path = patch_file
                if self.patch_dir is not None:
                    patch_path = os.path.join(self.patch_dir, patch_file)

                # Test if patch already applied
                cmd = [self.__get_path("git"), "apply", "--reverse", "--check"]
                cmd += git_patch_flags
                cmd.append(patch_path)
                if subprocess.call(cmd) != 0:
                    # Apply patch
                    cmd = [self.__get_path("git"), "apply", "--verbose"]
                    cmd += git_patch_flags
                    cmd.append(patch_path)
                    subprocess.check_call(cmd)
                else:
                    print("Assuming patch \"%s\" is already applied" % patch_file)

            os.chdir(cur_dir)

    def test(self):
        self.__do_stage( "test" )

    def execute(self):
        self.download()
        self.patch()
        self.configure()
        self.build()
        self.install()
        self.test()

class EnsureOnce(argparse.Action):
    def __call__(self, parser, namespace, values, option_string=None):
        if getattr(namespace, self.dest, None) is not None:
            raise argparse.ArgumentError(self, 'Can only be specified once.')
        setattr(namespace, self.dest, values)

def civetweb_preconfig( dest_dir ):
    # Make a shared library on windows
    if os.name == "nt":
        in_file = open("src/CMakeLists.txt", "r")
        lines = in_file.readlines()
        in_file.close()

        out_file = open("src/CMakeLists.txt", "w")
        for line in lines:
            if line.startswith("add_library(civetweb-c-library"):
                index = len("add_library(civetweb-c-library")
                line = line[:index] + " SHARED " + line[index:]
            out_file.write(line)
        out_file.close()

def jsmn_preconfig( dest_dir ):
    # We need a Makefile for compling on Windows
    if os.name == "nt":
        def_file = open("jsmn.def", "w")
        def_file.write("EXPORTS\n")
        def_file.write("jsmn_init\n")
        def_file.write("jsmn_parse\n")
        def_file.close()
        out_file = open("Makefile.nmake", "w")
        out_file.write("LDFLAGS = \n")
        out_file.write("\n")
        out_file.write("all: jsmn-static.lib jsmn.dll\n")
        out_file.write("\n")
        out_file.write("jsmn.dll: jsmn.obj jsmn.h\n")
        out_file.write("\t$(CC) /nologo jsmn.obj /link $(LDFLAGS) /def:jsmn.def /dll /out:$@\n")
        out_file.write("\n")
        out_file.write("jsmn-static.lib: jsmn.obj jsmn.h\n")
        out_file.write("\tlib /nologo /out:$@ jsmn.obj\n")
        out_file.write("\n")
        out_file.write(".c.obj:\n")
        out_file.write("\t$(CC) /c /nologo /EHsc $(CFLAGS) $**\n")
        out_file.write("\n")
        out_file.write("test: test_default test_strict test_links test_strict_links\n")
        out_file.write("\n")
        out_file.write("test_default: test/tests.c\n")
        out_file.write("\t$(CC) $** /nologo /link $(LDFLAGS) /out:$*.exe\n")
        out_file.write("\t$@\n")
        out_file.write("\n")
        out_file.write("test_strict: test/tests.c\n")
        out_file.write("\t$(CC) -DJSMN_STRICT=1 $** /nologo /link $(LDFLAGS) /out:$*.exe\n")
        out_file.write("\t$@\n")
        out_file.write("\n")
        out_file.write("test_links: test/tests.c\n")
        out_file.write("\t$(CC) -DJSMN_PARENT_LINKS=1 $** /nologo /link $(LDFLAGS) /out:$*.exe\n")
        out_file.write("\t$@\n")
        out_file.write("\n")
        out_file.write("test_strict_links: test/tests.c\n")
        out_file.write("\t$(CC) -DJSMN_PARENT_LINKS=1 -DJSMN_PARENT_LINKS=1 $** /nologo /link $(LDFLAGS) /out:$*.exe\n")
        out_file.write("\t$@\n")
        out_file.write("\n")
        out_file.write("simple_example: example/simple.obj\n")
        out_file.write("\t$(CC) $** jsmn.lib /nologo /link $(LDFLAGS) /out:$*.exe\n")
        out_file.write("\n")
        out_file.write("jsondump: example/jsondump.obj\n")
        out_file.write("\t$(CC) $** jsmn.lib /nologo /link $(LDFLAGS) /out:$*.exe\n")
        out_file.write("\n")
        out_file.write("clean:\n")
        out_file.write("\t-del *.obj exmple\\*.obj\n")
        out_file.write("\t-del *.lib *.dll\n")
        out_file.write("\t-del simple_example.exe\n")
        out_file.write("\t-del jsondump.exe\n")
        out_file.write("\t-del test_default.exe test_strict.exe test_links.exe test_strict_links.exe\n")
        out_file.write("\n")
        out_file.close()

def mosquitto_preconfig(dest_dir):
    include_dir = os.path.join(dest_dir, "include")
    lib_dir = os.path.join(dest_dir, "lib")
    if os.name == "nt":
        websocket_lib_path = os.path.join(lib_dir, "websockets.lib")
    else:
        websocket_lib_path = os.path.join(lib_dir, "libwebsockets.a")
    websocket_lib_path = websocket_lib_path.replace( "\\", "\\\\")
    lib_dir = lib_dir.replace( "\\", "\\\\")
    include_dir = include_dir.replace( "\\", "\\\\")

    cmake_changes = {
        "ldconfig": "ldconfig ARGS -N",
        "add_subdirectory(man)": "#add_subdirectory(man)",
        "set (BINDIR .)": "set (BINDIR bin)",
        "set (SBINDIR .)": "set (SBINDIR bin)",
        "set (SYSCONFDIR .)": "set (SYSCONFDIR etc)",
        "set (LIBDIR .)": "set (LIBDIR lib)",
        "TARGETS libmosquitto" : "TARGETS libmosquitto ARCHIVE DESTINATION \"${LIBDIR}\"",
        "C:\\\\pthreads\\\\Pre-built.2\\\\lib\\\\x86": lib_dir,
        "C:\\\\pthreads\\\\Pre-built.2\\\\lib\\\\x64": lib_dir,
        "C:\\\\pthreads\\\\Pre-built.2\\\\include": include_dir,
        "websockets)": "\"%s\")" % websocket_lib_path
    }

    # find all CMakeLists.txt files under the current directory
    cmake_files = []
    dirs = ['.']
    while len(dirs) > 0:
        cur_dir = dirs.pop(0)
        for file_name in os.listdir(cur_dir):
            if file_name == "CMakeLists.txt":
                cmake_files.append(os.path.join(cur_dir,file_name))
            elif file_name != "." and file_name != ".." and os.path.isdir(file_name):
                dirs.append(os.path.join(cur_dir,file_name))

    for file_path in cmake_files:
        in_file = open(file_path, "r")
        lines = in_file.readlines()
        in_file.close()

        for key, value in cmake_changes.iteritems():
            out_lines = []
            for line in lines:
                out_lines.append(line.replace(key,value))
            lines = out_lines

        out_file = open(file_path, "w")
        for line in lines:
            out_file.write(line)
        if file_path == os.path.join(".","src","CMakeLists.txt"):
            out_file.write("if (${WITH_WEBSOCKETS} STREQUAL ON)\n")
            out_file.write("\tinclude_directories(SYSTEM \"%s\")\n" % include_dir)
            out_file.write("endif (${WITH_WEBSOCKETS} STREQUAL ON)\n")
        out_file.close()


def pthreads_preconfig( dest_dir ):
    in_file = open("Makefile", "r")
    lines = in_file.readlines()
    in_file.close()

    out_file = open("Makefile", "w")
    for line in lines:
        if line.startswith("DEVROOT"):
            line = "DEVROOT = %s\n" % dest_dir
        out_file.write(line)
    out_file.close()

def jsmn_install( dest_dir ):
    shutil.copyfile("jsmn.h", os.path.join(dest_dir, "include", "jsmn.h"))
    if os.name == "nt":
        shutil.copyfile("jsmn-static.lib", os.path.join(dest_dir, "lib", "jsmn-static.lib"))
        shutil.copyfile("jsmn.lib", os.path.join(dest_dir, "lib", "jsmn.lib"))
        shutil.copyfile("jsmn.dll", os.path.join(dest_dir, "bin", "jsmn.dll"))
        #shutil.copyfile("jsmn.pdb", os.path.join(dest_dir, "bin", "jsmn.pdb"))
    else:
        shutil.copyfile("libjsmn.a", os.path.join(dest_dir, "lib", "libjsmn.a"))

def libcurl_install( dest_dir ):
    libcurl_subdir = os.path.join("..", "builds",
        "libcurl-vc-x86-release-dll-ssl-dll-zlib-dll-ipv6-sspi")
    libcurl_src_dir = os.path.join(libcurl_subdir, "include", "curl")
    libcurl_dst_dir = os.path.join(dest_dir, "include", "curl")
    for item in os.listdir(libcurl_src_dir):
        s = os.path.join(libcurl_src_dir, item)
        d = os.path.join(libcurl_dst_dir, item)
        if os.path.isfile(s):
            if not os.path.exists(libcurl_dst_dir):
                os.mkdir(libcurl_dst_dir)
            shutil.copyfile(s, d)
    shutil.copyfile(os.path.join(libcurl_subdir, "lib", "libcurl.lib"),
        os.path.join(dest_dir, "lib", "libcurl.lib"))
    shutil.copyfile(os.path.join(libcurl_subdir, "bin", "libcurl.dll"),
        os.path.join(dest_dir, "bin", "libcurl.dll"))
    #shutil.copyfile(os.path.join(libcur_subdir, "bin", "libcurl.pdb"),
    #    os.path.join(dest_dir, "bin", "libcurl.pdb"))
    shutil.copyfile(os.path.join(libcurl_subdir, "bin", "curl.exe"),
        os.path.join(dest_dir, "bin", "curl.exe"))

# ----------------
# STARTING POINT
# ----------------
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--deps", help="dependency directory to use", type=str)
    parser.add_argument("--type", choices=['Debug', 'Release'], help="Build type" )
    parser.add_argument("--test", help="Build with test support", action='store_true')
    parser.add_argument("--stack_only", help="Build with stack only support", action='store_true')
    parser.add_argument("--thread_support", help="Build with threading support", action='store_true')
    parser.add_argument("--json", choices=['jansson','json-c', 'jsmn'], help="JSON library to use", action=EnsureOnce)
    parser.add_argument("--mqtt", choices=['mosquitto','paho'], help="MQTT library to use", action=EnsureOnce)
    parser.add_argument("--websockets", choices=['civetweb', 'libwebsockets'], help="Websocket library to use", action=EnsureOnce)
    args = parser.parse_args(sys.argv[1:])

    # Required system dependencies:
    #    See README.md for list of dependencies from the build host

    # Determine target build type
    if args.type is None:
        args.type = "Release"

    cwd = os.getcwd()

    cmake_args = []
    try:
        script_path = os.path.dirname(os.path.realpath(__file__))
        patch_dir = os.path.join(script_path, "build-sys", "patches")

        # Create dependency directoary
        deps_dir = args.deps
        if not deps_dir:
            deps_dir = os.path.join(os.getcwd(), "deps")

        if not os.path.exists( deps_dir ):
            os.makedirs( deps_dir )
        elif not os.path.isdir( deps_dir ):
            raise RuntimeError( "Target dependency destination exists but is not a directory: %s" % deps_dir )

        os.chdir( deps_dir )

        include_dir = "include"
        if not os.path.exists( include_dir ):
            os.makedirs( include_dir )
        elif not os.path.isdir( include_dir ):
            raise RuntimeError( "Include install destination exists but is not a directory: %s" % include_dir )

        lib_dir = "lib"
        if not os.path.exists( lib_dir ):
            os.makedirs( lib_dir )
        elif not os.path.isdir( lib_dir ):
            raise RuntimeError( "Include library destination exists but is not a directory: %s" % lib_dir )

        # operating system abstraction layer (required)
        osal_flags = []
        if args.thread_support:
            osal_flags.append( "-DOSAL_THREAD_SUPPORT:BOOL=ON" )
        osal_flags.append( "-DOSAL_WRAP:BOOL=ON" )

        osal_project = BuildProject("operating system abstraction layer",
            download_dir = "osal",
            download_url = "https://github.com/Wind-River/device-cloud-osal.git",
            config_tool = "cmake",
            config_flags = osal_flags,
            build_tool = "cmake",
            build_type = args.type,
            install_dir = deps_dir,
            install_target = "install"
        )

        osal_project.execute()

        if args.test:
            # cmocka (optional)
            cmocka_project = BuildProject("cmocka",
                download_dir = "cmocka-1.1.1",
                download_url = "https://cmocka.org/files/1.1/cmocka-1.1.1.tar.xz",
                patch_dir = patch_dir,
                patches = [ "cmocka-patch-to-build-on-visual-studio-2012.patch" ],
                config_tool = "cmake",
                build_tool = "cmake",
                build_dir = "cmake_build",
                build_type = args.type,
                install_dir = deps_dir,
                install_target = "install"
            )
            cmocka_project.execute()

        # Zlib (Windows only)
        zlib_include_dir=""
        zlib_library=""
        if os.name == "nt":
            zlib_project = BuildProject("zlib",
                download_dir = "zlib-1.2.11",
                download_url = "http://zlib.net/zlib-1.2.11.tar.gz",
                config_tool = "cmake",
                build_tool = "cmake",
                build_type = args.type,
                install_dir = deps_dir,
                install_target = "install"
            )
            zlib_project.execute()
            zlib_include_dir = os.path.join(deps_dir, "include")
            if args.type == "Release":
                zlib_library = os.path.join(deps_dir, "lib", "zlib.lib")
            else:
                zlib_library = os.path.join(deps_dir, "lib", "zlibd.lib" )

        # OpenSSL (Windows only)
        if os.name == "nt":
            if args.type != "Release":
                openssl_target = "debug-VC-WIN32"
            else:
                openssl_target = "VC-WIN32"
            openssl_project = BuildProject("openssl",
                download_url = "https://github.com/openssl/openssl.git",
                download_tag = "tags/OpenSSL_1_1_0h",
                config_tool = "perl",
                config_flags = ["Configure", openssl_target, "no-asm",
                    "--prefix=%s" % deps_dir,
                    "--openssldir=%s" % "ssl"
                ],
                build_tool = "nmake",
                build_type = args.type,
                install_dir = deps_dir,
                install_target = "install"
            )
            cmake_args += [
                "-DOPENSSL_INCLUDE_DIR:PATH=%s" % os.path.join(deps_dir, "include"),
                "-DOPENSSL_ROOT_DIR:PATH=%s" % deps_dir
            ]
            openssl_project.execute()

        # Libarchive (Windows only)
        if os.name == "nt":
            libarchive_project = BuildProject("libarchive",
                download_url = "https://github.com/libarchive/libarchive.git",
                download_tag = "tags/v3.3.1",
                config_tool = "cmake",
                config_flags = [
                    "-DOPENSSL_INCLUDE_DIR:PATH=%s" % os.path.join(deps_dir, "include"),
                    "-DOPENSSL_ROOT_DIR:PATH=%s" % deps_dir,
                    "-DZLIB_INCLUDE_DIR:PATH=%s" % zlib_include_dir,
                    "-DZLIB_LIBRARY:FILEPATH=%s" % zlib_library ],
                build_tool = "cmake",
                build_type = args.type,
                install_dir = deps_dir,
                install_target = "install"
            )
            cmake_args += [
                "-DLibArchive_INCLUDE_DIR:PATH=%s" % os.path.join(deps_dir, "include"),
                "-DLibArchive_LIBRARY:FILEPATH=%s" % os.path.join(deps_dir, "lib", "archive.lib")
            ]
            libarchive_project.execute()

        # JSON library
        if args.json == "jansson":
            # jansson
            json_project = BuildProject("jansson",
                download_dir = "jansson",
                download_url = "http://www.digip.org/jansson/releases/jansson-2.11.tar.gz",
                config_tool = "cmake",
                config_flags = [ "-DJANSSON_BUILD_DOCS:BOOL=NO",
                    "-DJANSSON_BUILD_SHARED_LIBS:BOOL=ON" ],
                build_tool = "cmake",
                build_type = args.type,
                install_dir = deps_dir,
                install_target = "install"
            )
            cmake_args.append("-DIOT_JSON_LIBRARY:STRING=jansson")
        elif args.json == "json-c":
            # json-c
            json_project = BuildProject("json-c",
                download_dir = "json-c",
                download_url = "https://github.com/json-c/json-c.git",
                donwload_tag = "tags/json-c-0.13.1-20180305",
                config_tool = "cmake",
                build_tool = "cmake",
                build_type = args.type,
                install_dir = deps_dir,
                install_target = "install"
            )
            cmake_args.append("-DIOT_JSON_LIBRARY:STRING=json-c")
        else:
            # jsmn (default)
            jsmn_flags = []
            jsmn_build_flags = ["CFLAGS=-DJSMN_PARENT_LINKS=1 -DJSMN_STRICT=1"]
            if os.name == "nt":
                make_tool = "nmake"
                jsmn_flags.append("/F")
                jsmn_flags.append("Makefile.nmake")
            else:
                make_tool = "make"
                jsmn_build_flags.append("-fPIC")

            json_project = BuildProject("jsmn",
                download_dir = "jsmn",
                download_url = "https://github.com/zserge/jsmn.git",
                config_pre = jsmn_preconfig,
                config_tool = make_tool,
                config_flags = jsmn_flags + [ "clean" ],
                build_flags = jsmn_flags + jsmn_build_flags,
                build_tool = make_tool,
                build_type = args.type,
                test_flags = jsmn_flags + [ "test" ],
                install_dir = deps_dir,
                install_target = jsmn_install
            )
        json_project.execute()

        # WEBSOCKETS library
        if args.websockets == "civetweb":
            # civetweb
            websockets_project = BuildProject("civetweb",
                download_dir = "civetweb",
                download_url = "https://github.com/lt-holman/civetweb.git",
                download_tag = "branch/vs2015_fixes2",
                #download_url = "https://github.com/civetweb/civetweb.git",
                #download_tag = "f0ae9d573ab58d12e3dea126454d58dba9d9660",
                # download_tag = "tags/v1.11",  # Version 1.11 doesn't exist yet
                #config_pre = civetweb_preconfig,
                config_tool = "cmake",
                config_flags = [ "-DBUILD_SHARED_LIBS:BOOL=ON",
                    "-DCIVETWEB_ENABLE_IPV6:BOOL=ON",
                    "-DCIVETWEB_ENABLE_SSL:BOOL=ON",
                    "-DCIVETWEB_ENABLE_WEBSOCKETS:BOOL=ON",
                    "-DCIVETWEB_SSL_OPENSSL_API_1_1:BOOL=ON" ],
                build_tool = "cmake",
                build_dir = "cmake_build",
                build_type = args.type,
                install_dir = deps_dir,
                install_target = "install"
            )
            cmake_args.append("-DIOT_WEBSOCKET_LIBRARY:STRING=civetweb")
        else:
            lws_opts = [ "-DLWS_WITHOUT_TEST_APPS:BOOL=ON",
                    "-DLWS_WITH_SOCKS5:BOOL=ON"]

            if os.name == "nt":
                lws_opts.append( "-DOPENSSL_ROOT_DIR:PATH=%s" % deps_dir )
                lws_opts.append( "-DZLIB_INCLUDE_DIR:PATH=%s" % zlib_include_dir ),
                lws_opts.append( "-DZLIB_LIBRARY:FILEPATH=%s" % zlib_library )
                lws_opts.append( "-DLWS_USE_BUNDLED_ZILB:BOOL=OFF" )

            # libwebsockets (default)
            websockets_project = BuildProject("libwebsockets",
                download_dir = "libwebsockets",
                download_url = "https://github.com/warmcat/libwebsockets.git",
                download_tag = "tags/v2.3.0",
                config_tool = "cmake",
                config_flags = lws_opts,
                build_tool = "cmake",
                build_type = args.type,
                install_dir = deps_dir,
                install_target = "install"
            )
        websockets_project.execute()

        # MQTT library
        if args.mqtt == "mosquitto":
            if os.name == "nt":
                # pthreads-win32
                pthreads_type = ""
                if args.type != "Release":
                    pthreads_type = "-debug"

                pthreads_project = BuildProject( "pthreads-win32",
                    download_url = "ftp://sourceware.org/pub/pthreads-win32/pthreads-w32-2-9-1-release.tar.gz",
                    download_dir = "pthreads-w32-2-9-1-release",
                    patch_dir = patch_dir,
                    patches = ["pthreads-w32-patch-to-build-on-visual-studio-2015.patch"],
                    config_pre = pthreads_preconfig,
                    build_flags = ["clean", "VC%s" % pthreads_type],
                    build_tool = "nmake",
                    build_type = args.type,
                    install_dir = deps_dir,
                    install_target = "install"
                )
                pthreads_project.execute()

            # mosquitto
            mosquitto_flags = ["-DWITH_SRV:BOOL=OFF"]
            if os.name == "nt":
                mosquitto_flags += [
                    "-DOPENSSL_INCLUDE_DIR:PATH=%s" % os.path.join(deps_dir, "include"),
                    "-DOPENSSL_ROOT_DIR:PATH=%s" % deps_dir
                ]
                if args.websockets != "civetweb":
                    mosquitto_flags.append("-DWITH_WEBSOCKETS:BOOL=ON")

            mqtt_project = BuildProject( "mosquitto",
                download_dir = "mosquitto",
                download_url = "https://github.com/eclipse/mosquitto.git",
                download_tag = "tags/v1.4.14",
                patch_dir = patch_dir,
                patches = [ "mosquitto_passwd-add-symbols-defined-in-windows.h.patch" ],
                config_pre = mosquitto_preconfig,
                config_tool = "cmake",
                config_flags = mosquitto_flags,
                build_tool = "cmake",
                build_type = args.type,
                install_dir = deps_dir,
                install_target = "install"
            )
            cmake_args.append("-DIOT_MQTT_LIBRARY:STRING=mosquitto")
        else:
            # paho (default)
            paho_flags = [ "-DPAHO_WITH_SSL:BOOL=TRUE",
                    "-DPAHO_BUILD_STATIC:BOOL=TRUE" ]
            if os.name == "nt":
                paho_flags += [
                    "-DOPENSSL_INCLUDE_DIR:PATH=%s" % os.path.join(deps_dir, "include"),
                    "-DOPENSSL_ROOT_DIR:PATH=%s" % deps_dir
                ]
            else:
                paho_flags.append( "-DCMAKE_C_FLAGS:STRING=-fPIC" )
            mqtt_project = BuildProject( "paho-c",
                download_dir = "paho",
                download_url = "https://github.com/eclipse/paho.mqtt.c.git",
                download_tag = "tags/v1.2.0",
                config_tool = "cmake",
                config_flags = paho_flags,
                build_tool = "cmake",
                build_type = args.type,
                install_dir = deps_dir,
                install_target = "install"
            )
        mqtt_project.execute()

        # libcurl (Windows only)
        if os.name == "nt":
            libcurl_project = BuildProject("libcurl",
                download_url = "https://github.com/curl/curl.git",
                download_tag = "tags/curl-7_60_0",
                config_tool = os.path.join("..", "buildconf.bat"),
                build_dir = "winbuild",
                build_flags = [ "/f", "Makefile.vc", "mode=dll",
                    "WITH_SSL=dll", "WITH_ZLIB=dll",
                    "WITH_DEVEL=%s" % deps_dir,
                    "MACHINE=x86", "MAKE=NMAKE /e",
                    "SSL_LIBS=libssl.lib libcrypto.lib" ],
                build_tool = "nmake",
                build_type = args.type,
                install_dir = deps_dir,
                install_target = libcurl_install
            )
            cmake_args += [
                "-DCURL_INCLUDE_DIR:PATH=%s" % os.path.join(deps_dir, "include"),
                "-DCURL_LIBRARY:FILEPATH=%s" % os.path.join(deps_dir, "lib", "libcurl.lib")
            ]

            libcurl_project.execute()

        # Main project
        os.chdir(cwd)
        if args.thread_support:
            cmake_args.append( "-DIOT_THREAD_SUPPORT:BOOL=ON" )
        else:
            cmake_args.append( "-DIOT_THREAD_SUPPORT:BOOL=OFF" )
        if args.stack_only:
            cmake_args.append( "-DIOT_STACK_ONLY:BOOL=ON" )
        else:
            cmake_args.append( "-DIOT_STACK_ONLY:BOOL=OFF" )
        cmake_args.append( "-DDEPENDS_ROOT_DIR:PATH=%s" % deps_dir )

        main_project = BuildProject( PROJECT_NAME,
            config_tool = "cmake",
            config_flags = cmake_args,
            download_dir = script_path,
            build_tool = "cmake",
            build_dir = os.getcwd(),
            build_type = args.type,
            install_dir = "out",
            install_target = "install",
            test_target = "check"
        )
        main_project.execute()

        print( "%s is ready to build." % PROJECT_NAME )

    except Exception, e:
        eprint("Error: %s" % str(e))
    finally:
        os.chdir(cwd)
    return 0

if __name__ == "__main__":
    sys.exit(main())
