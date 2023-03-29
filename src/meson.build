sthread_inc = include_directories('../include')

sthread_srcs = [
  'Lock.cpp',
  'Timer.cpp',
  'Thread.cpp',
  'ThreadDriver.cpp',
  'QueueThread.cpp',
]

system_has_pthread = [
  'openbsd', 'netbsd', 'freebsd', 'gnu/kfreebsd', 'dragonfly', 'sunos',
  'linux', 
  'android', 
  'darwin'
  ].contains(host_machine.system())

if system_has_pthread
    sthread_srcs += [
      'PThread/PThreadThreadDriver.cpp',
    ]
    thread_dep = dependency('threads')
elif ['windows'].contains(host_machine.system())
    sthread_srcs += [
      'W32/W32ThreadThreadDriver.cpp',
    ]
else
    error('Unknown Platform @0@.'.format(host_machine.system()))
endif

sthread_lib = static_library('sthread',
                sthread_srcs,
                install: true,
                include_directories: sthread_inc,
                cpp_args: cpp_define_args
                )
