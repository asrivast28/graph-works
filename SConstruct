import os

topDir = os.getcwd()

cppPaths = [
            topDir,
            ]

cxxFlags = [
            '-Wall',
            '-Wextra',
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

#generate a flags file for use with ycm
#with open(os.path.join(os.getcwd(), '.ycm_flags'), 'wb')  as f:
    #for flag in env.get('CXXFLAGS', []):
        #f.write(flag)
        #f.write('\n')
    #for path in env.get('CPPPATH', []):
        #f.write('-I' + path)
        #f.write('\n')
