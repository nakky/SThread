
sthread_inc = include_directories('include')

sthread_srcs = [
  'src/Lock.cpp',
  'src/Timer.cpp',
  'src/Thread.cpp',
  'src/ThreadDriver.cpp',
  'src/QueueThread.cpp',
]

system_has_pthread = [
  'openbsd', 'netbsd', 'freebsd', 'gnu/kfreebsd', 'dragonfly', 'sunos',
  'linux', 
  'android', 
  'darwin'
  ].contains(host_machine.system())

if system_has_pthread
    sthread_srcs += [
      'src/PThread/PThreadThreadDriver.cpp',
    ]
    thread_dep = dependency('threads')
elif ['windows'].contains(host_machine.system())
    sthread_srcs += [
      'src/W32/W32ThreadThreadDriver.cpp',
    ]
else
    error('Unknown Platform @0@.'.format(host_machine.system()))
endif

sthread_lib = static_library('sthread',
                sthread_srcs,
                install: false,
                include_directories: sthread_inc,
                cpp_args: cpp_define_args
                )
