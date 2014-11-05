
sources = [
           'driver.cpp',
           ]

cxxFlags = [
            '-std=c++0x',
            ]

debug = ARGUMENTS.get('DEBUG', 0)
if debug in [0, '0']:
    cxxFlags.append('-g')
else:
    cxxFlags.append('-O3')

env = Environment(CXX = 'mpicxx', CXXFLAGS = cxxFlags)

env.Program(sources)
