/**
 *  Function.h
 *
 *  When you want to retrieve a function from a dynamicallly loaded
 *  library, you normally use the dlsym() function for that. The
 *  Function object is a little more convenient:
 *
 *      // open the library
 *      void *lib = dlopen("library.so");
 *
 *      // get the function object
 *      Function func<int(int)> magic_open(library, "my_function");
 *
 *      // call the function
 *      int result = func(123);
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2018 Copernica BV
 */


/**
 * Include guard
 */
#pragma once

/**
 * Dependencies
 */
#include <cstddef>
#include <functional>

/**
 *  Namespace
 */
namespace AMQP { 

/**
 *  Make the Function class a templated class
 */
template <class T> class Function {};

/**
 *  So that we can write a partial specialisation
 *  using a function prototype, indicating the
 *  prototype usable for the given callback
 */
template <class T, class ...Arguments>
class Function<T(Arguments...)>
{
private:
    /**
     *  Store pointer to the actual dynamically loaded
     *  function. This is done in a union because the
     *  result from a dlsym call is a void pointer which
     *  cannot be legally cast into a function pointer.
     *
     *  We therefore always set the first type (which is
     *  void* making it legal) and, because all members
     *  in a union share the same address, we can then
     *  read the second type and actually call it.
     *
     *  @var Callable
     */
    union Callable {

        /**
         *  Property for getting and setting the return
         *  value from dlsym. This is always a void*
         *  @var    void
         */
        void *ptr;

        /**
         *  Property for executing the mapped function
         *
         *  @param  mixed,...   function parameters
         *  @return mixed
         *
         *  @var    function
         */
        T (*func)(Arguments...);


        /**
         *  Constructor
         */
        Callable() : ptr(nullptr) {}

        /**
         *  We may be moved
         *
         *  @param  callable    the callable we are moving
         */
        Callable(Callable&& callable) :
            ptr(callable.ptr)
        {
            // the other callable no longer has a handle
            callable.ptr = nullptr;
        }

        /**
         *  Copy construtor
         *  @param  callable    the callable we are moving
         */
        Callable(const Callable &callable) :
            ptr(callable.ptr) {}

        /**
         *  Constructor
         *
         *  @param  function    the mapped function
         */
        Callable(void *function) : ptr(function) {}

    } _method;

public:
    /**
     *  Constructor
     *  @param  library     The library to load the function from
     *  @param  name        Name of the function
     */
    Function(void *library, const char *name) :
        _method(dlsym(library, name)) {}

    /**
     *  Destructor
     */
    virtual ~Function() {}

    /**
     *  Is this a valid function or not?
     *  @return bool
     */
    bool valid() const
    {
        return _method.ptr != nullptr;
    }

    /**
      *  The library object can also be used as in a boolean context,
      *  for that there is an implementation of a casting operator, and
      *  the negate operator
      *  @return bool
      */
     operator bool () const { return valid(); }
     bool operator ! () const { return !valid(); }

    /**
      *  Test whether we are a valid object
      *  @param  nullptr test if we are null
      */
    bool operator==(std::nullptr_t /* nullptr */) const { return !valid(); }
    bool operator!=(std::nullptr_t /* nullptr */) const { return valid(); }

    /**
     *  Invoke the function
     *
     *  @param  mixed,...
     */
    T operator()(Arguments... parameters) const
    {
        // check whether we have a valid function
        if (!valid()) throw std::bad_function_call();

        // execute the method given all the parameters
        return _method.func(std::forward<Arguments>(parameters)...);
    }
};

/**
 *  End of namespace
 */
}

