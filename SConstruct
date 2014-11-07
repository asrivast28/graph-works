import os

topDir = os.getcwd()

cppPaths = [
            topDir,
            ]

cxxFlags = [
            '-std=c++0x',
            ]

debug = ARGUMENTS.get('DEBUG', 0)
buildDir = 'build'
if debug in [0, '0']:
    cxxFlags.append('-O3')
    buildDir = 'release'
else:
    cxxFlags.append('-g')
    buildDir = 'debug'

buildDir = os.path.join('builds', buildDir)

env = Environment(CXX = 'mpicxx', CXXFLAGS = cxxFlags, CPPPATH = cppPaths)

SConscript('src/SConscript', exports = 'env', variant_dir = buildDir, src_dir = 'src', duplicate = 0)
