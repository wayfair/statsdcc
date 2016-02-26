
#ifndef INCLUDE_STATSDCC_ATOMIC_H_
#define INCLUDE_STATSDCC_ATOMIC_H_

#ifdef HAVE_ATOMIC
  #include <atomic>
#endif

#ifdef HAVE_CSTDATOMIC
  #include <cstdatomic>
#endif

#endif  // INCLUDE_STATSDCC_ATOMIC_H_
