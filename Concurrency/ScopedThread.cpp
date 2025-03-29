#include <iostream>
#include <thread>

class ScopedThread
{
    std::thread t;
public:
    explicit ScopedThread(std::thread t_) : t(std::move(t_))
    {
        if (!t.joinable()) throw std::logic_error("No thread");
    }
    ~ScopedThread()
    {
        t.join();
    }
    ScopedThread(ScopedThread&& other) noexcept : t(std::move(other.t)) {}
    ScopedThread(ScopedThread const &) = delete;
    ScopedThread& operator=(ScopedThread const &) = delete;
};

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
    ScopedThread st{std::thread(doWork)};
    doAnotherWork();
    return 0;
}