#include <iostream>
#include <thread>
#include <mutex>
class HierarchicalMutex
{
    std::mutex m;
    unsigned long const hierarchyValue;
    unsigned long previousHierarchyValue;
    static thread_local unsigned long thisThreadHierarchyValue;
public:
    explicit HierarchicalMutex(unsigned long value) : hierarchyValue(value), previousHierarchyValue(0) {}
    void lock()
    {
        checkForHierarchyViolation();
        m.lock();
        updateHierarchyValue();
    }
    void unlock()
    {
        if (thisThreadHierarchyValue != hierarchyValue)
            throw std::logic_error("mutex hierarchy violated");
        thisThreadHierarchyValue = previousHierarchyValue;
        m.unlock();
    }
    bool try_lock()
    {
        checkForHierarchyViolation();
        if (!m.try_lock())
            return false;
        updateHierarchyValue();
        return true;
    }
    void checkForHierarchyViolation()
    {
        if (thisThreadHierarchyValue <= hierarchyValue)
            throw std::logic_error("mutex hierarchy violated");
    }
    void updateHierarchyValue()
    {
        previousHierarchyValue = thisThreadHierarchyValue;
        thisThreadHierarchyValue = hierarchyValue;
    }
};

thread_local unsigned long HierarchicalMutex::thisThreadHierarchyValue(ULONG_MAX);

void doWork()
{
    std::cout << "doWork" << std::endl;
}

void doAnotherWork()
{
    std::cout << "doAnotherWork" << std::endl;
}

int main()
{
    HierarchicalMutex highLevelMutex(10000);
    HierarchicalMutex lowLevelMutex(5000);
    HierarchicalMutex otherMutex(6000);
    std::thread t1([&]() {
        highLevelMutex.lock();
        lowLevelMutex.lock();
        doWork();
        lowLevelMutex.unlock();
        highLevelMutex.unlock();
    });
    doAnotherWork();
    t1.join();
    return 0;
}