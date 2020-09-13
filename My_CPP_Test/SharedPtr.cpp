#include <iostream>
template<typename T>
class SharedPtr
{
public:
    SharedPtr(T* ptr)
        :m_ptr(ptr),
         m_count(new unsigned int(1))
    {

    }

    SharedPtr(const SharedPtr<T>& rhs)
        :m_ptr(rhs.m_ptr),
         m_count(&(++*rhs.m_count))
    {

    }

    ~SharedPtr()
    {
        if(m_ptr&&--*m_count==0)
        {
            delete m_ptr;
            delete m_count;
        }
    }

    SharedPtr<T>& operator=(const SharedPtr& rhs)
    {
        if(*this!=rhs)
        {
            if(--*m_count==0)
            {
                delete m_ptr;
                delete m_count;
            }
            m_ptr = rhs.m_ptr;
            m_count = &(++*rhs.m_count);
        }
        return *this;
    }

    T operator*()
    {
        if(*m_count==0)
            return (T)0;
        return *m_ptr;
    }

    T* operator->()
    {
        if(*m_count==0)
            return nullptr;
        return m_ptr;
    }
    void Reset()
    {
        if(--*m_count == 0)
        {
            delete m_ptr;
            delete m_count;
        }
    }

    unsigned int UseCount()
    {
        return *m_count;
    }
protected:
private:
    T* m_ptr;
    unsigned int* m_count;
};

int main()
{
    SharedPtr<int> t(new int(1));
    std::cout << *t << " " << t.UseCount() << std::endl;
    SharedPtr<int> s(t);
    std::cout << *s << " " << s.UseCount() << std::endl;
    s.Reset();
    std::cout << *s << " " << s.UseCount() << std::endl;
    return 0;
}