
inc = include_directories([
  '../include',
  ])

sthread_dep = declare_dependency(link_with: sthread_lib)

deps = [
  sthread_dep,
]

thread_dep = dependency('threads')

deps += [thread_dep]

srcs = [
  'lock.cpp',
]

executable(
  'sthread_lock',
  srcs,
  install: false,
  include_directories: inc,
  dependencies: deps,
  cpp_args: cpp_define_args,
)

srcs = [
  'thread.cpp',
]

executable(
  'sthread_thread',
  srcs,
  install: false,
  include_directories: inc,
  dependencies: deps,
  cpp_args: cpp_define_args,
)

srcs = [
  'queuethread.cpp',
]

executable(
  'sthread_queuethread',
  srcs,
  install: false,
  include_directories: inc,
  dependencies: deps,
  cpp_args: cpp_define_args,
)

srcs = [
  'misc.cpp',
]

executable(
  'sthread_misc',
  srcs,
  install: false,
  include_directories: inc,
  dependencies: deps,
  cpp_args: cpp_define_args,
)


