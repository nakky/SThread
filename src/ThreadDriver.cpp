
#include "SThread/ThreadDriver.h"

#if defined USE_WINDOWSTHREAD_INTERFACE
#include "SThread/W32/W32ThreadDriver.h"
#elif defined USE_PTHREAD_INTERFACE
#include "SThread/PThread/PThreadThreadDriver.h"
#endif

namespace SThread{

    //static
    ThreadDriver *ThreadDriver::createDriver(Thread *thread)
    {
#if defined USE_WINDOWSTHREAD_INTERFACE
        return new W32ThreadDriver(thread);
#elif defined USE_PTHREAD_INTERFACE
        return new PThreadThreadDriver(thread);
#endif
    }
    
    ThreadHandle ThreadDriver::getCurrentThreadHandle()
    {
#if defined USE_WINDOWSTHREAD_INTERFACE
        return GetCurrentThread();
#elif defined USE_PTHREAD_INTERFACE
        return pthread_self();
#endif
    }
    
}; //namespace SThread


