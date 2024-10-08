#ifndef GLOBAL_H
#define GLOBAL_H

#include <memory>
#include <string>

typedef std::shared_ptr<std::string> StringPtr;

#define UNUSED(x) (void)(x)
#define WEAK(TPtr) std::weak_ptr<typename TPtr::element_type>
#define SHARED_TO_WEAK(shr_ptr) std::weak_ptr<typename std::remove_reference<decltype(shr_ptr)>::type::element_type>(shr_ptr)
#define WEAK_TO_SHARED(weak_ptr) weak_ptr.lock()

#ifdef _MSC_VER
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

#endif