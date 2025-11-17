project(
  'SThread',
  'cpp',
  version: '0.1.0',
  default_options: [
    'cpp_std=c++20',
    'default_library=both',
  ],
  meson_version: '>=0.47.0',
)

compiler = meson.get_compiler('cpp')
compiler_id = compiler.get_id()
bt = get_option('buildtype')

cpp_args = []
link_args = []
static_link_args = []
cpp_defines = []
cpp_define_args = []
define_prefix = '-D'

posix_common_args = [
  '-Wall', '-Wextra', '-Wpedantic', '-Wshadow', '-Wstrict-aliasing',
  '-Wstrict-overflow=5', '-Wcast-align',
  '-Wpointer-arith', '-Wcast-qual', 
]
if compiler_id == 'clang'
  cpp_args += posix_common_args + [
    '-Wabi', '-flto',
  ]
  link_args += ['-flto']
  static_link_args = [
    '-lstdc++', '-lm',
  ]
  cpp_defines += ['PACKETEER_IS_BUILDING=1']
elif compiler_id == 'gcc'
  cpp_args += posix_common_args + [
    '-Wstrict-null-sentinel', '-flto',
  ]
  link_args += ['-flto']
  static_link_args = [
    '-lstdc++', '-lm',
  ]
  cpp_defines += ['PACKETEER_IS_BUILDING=1']
endif

if bt in ['debug', 'debugoptimized']
  cpp_defines += ['DEBUG=1']

  if compiler_id == 'clang'
    cpp_args += ['-ggdb']
  elif compiler_id == 'gcc'
    cpp_args += ['-g3']
  endif
else
  cpp_defines += ['NDEBUG=1']

  posix_release_args = [
    '-fvisibility=hidden', '-fvisibility-inlines-hidden',
  ]
  if compiler_id == 'clang'
    cpp_args += posix_release_args
  elif compiler_id == 'gcc'
    cpp_args += posix_release_args
  endif
endif

# cpp_args += ['-fpermissive', '-Wold-style-cast']

cpp_args += ['-DWLR_USE_UNSTABLE']

foreach def : cpp_defines
  cpp_define_args += [define_prefix + def]
endforeach

add_project_arguments(cpp_args, language: 'cpp')

subdir('src')

sthread_dep = declare_dependency(link_with: sthread_lib)

if get_option('examples')
  subdir('examples')
endif

