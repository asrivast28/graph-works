cxxFlags = [
            '-std=c++0x',
            ]

debug = ARGUMENTS.get('DEBUG', 0)
buildDir = '.'
if debug in [0, '0']:
    cxxFlags.append('-O3')
    buildDir = 'release'
else:
    cxxFlags.append('-g')
    buildDir = 'debug'

env = Environment(CXX = 'mpicxx', CXXFLAGS = cxxFlags)

SConscript('SConscript', variant_dir = buildDir, src_dir = '.', duplicate = 0, exports = 'env')
