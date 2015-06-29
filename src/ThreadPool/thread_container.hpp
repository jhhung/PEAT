#ifndef THREAD_CONTAINER_HPP
#define THREAD_CONTAINER_HPP
#include <utility>
#include <iostream>
class ThreadContainer
{
    ThreadAdapter* head;
    size_t length;
public:
    ThreadContainer(size_t thread_num)
    : length ( thread_num )
    , head ( new ThreadAdapter[thread_num] )
    {}

    ~ThreadContainer()
    {
        clear();
    }

    ThreadAdapter& operator[] (int index)
    {
        if(index >= length)
        {
            std::cerr << "index out of range\n";
            abort();
        }
        else
        {
            return *(head + index);
        }
    }

    template<class TC>
    void swap ( TC&& tc)
    {
        std::swap(length, tc.length);
        std::swap(head, tc.head);
    }

    void clear()
    {
        length = 0;
        for( size_t i = 0; i < length; i++ )
            delete (head + (int)i);
    }

    size_t size()
    {
        return length;
    }

    ThreadAdapter* begin()
    {
        return head;
    }

    ThreadAdapter* end()
    {
        return head + length;
    }

};
#endif
