#include <vector>
#include <functional>
#include <algorithm>

#include <iostream>
template<typename T>
class Compare
{
public:
    bool operator()(const T& l,const T& r)
    {
        return l > r;
    }
};
template<typename T>
void BubbleSort(std::vector<T>& nums,
                const std::function<bool(const T&,const T&)>& func=Compare<T>())
{
    for(unsigned int i=0;i<nums.size();++i)
    {
        for(unsigned int j=i+1;j<nums.size();++j)
        {
            if(func(nums[i],nums[j]))
            {
                T t=nums[i];
                nums[i]=nums[j];
                nums[j]=t;
            }
        }
    }
}

int main(int argc, char const *argv[])
{
    std::vector<int> nums{9,5,7,1,2,4,3,8};
    BubbleSort(nums);
    std::for_each(nums.begin(),nums.end(),[](auto& iter){ std::cout << iter << " "; });
    std::cout << std::endl;
    
    BubbleSort<int>(nums,[](const auto&l,const auto&r)->bool{ return l < r; });
    std::for_each(nums.begin(),nums.end(),[](auto& iter){ std::cout << iter << " "; });
    std::cout << std::endl;

    std::vector<double> nums1{1.1,8.8,7.9,2.5,6.3,3.4,4.5};
    BubbleSort(nums1);
    std::for_each(nums1.begin(),nums1.end(),[](auto& iter){ std::cout << iter << " "; });
    std::cout << std::endl;

    return 0;
}
