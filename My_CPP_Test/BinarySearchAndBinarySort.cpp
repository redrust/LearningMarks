#include <vector>
#include <functional>
#include <algorithm>
#include <iostream>
template<typename T>
void BinarySortCore(std::vector<T>& nums,
                    int left,int right,
                    std::function<bool(const T&,const T&)> func)
{
    if(left<right)
    {
        T i=left;
        T j=right;
        T pivot=nums[left];
        while(i<j)
        {
            while(i<j&&!func(nums[j],pivot))
            {
                --j;
            }
            if(i<j)
            {
                nums[i++]=nums[j];
            }
            while(i<j&&func(nums[i],pivot))
            {
                ++i;
            }
            if(i<j)
            {
                nums[j--]=nums[i];
            }
        }
        nums[i]=pivot;
        BinarySortCore(nums,left,i-1,func);
        BinarySortCore(nums,i+1,right,func);
    }
}

template<typename T>
void BinarySort(std::vector<T>& nums,
    std::function<bool(const T&,const T&)> func=[](const auto& l,const auto& r){return l<r;})
{
    BinarySortCore(nums,0,nums.size()-1,func);
}

template<typename T>
int BinarySearch(std::vector<T>& nums,T target)
{
    int begin=0,end=nums.size()-1,mid=0;
    while(begin<=end)
    {
        mid=begin+(end-begin)/2;
        if(nums[mid]==target)
        {
            return mid;
        }
        else if(nums[mid]<target)
        {
            begin=mid+1;
        }
        else
        {
            end=mid-1;
        }
    }
    return -1;
}

bool DoubleEqual(double& l,double& r)
{
    return abs(l-r) < 0.00000001;
}

template<>
int BinarySearch(std::vector<double>& nums,double target)
{
    int begin=0,end=nums.size()-1,mid=0;
    while(begin<=end)
    {
        mid=begin+(end-begin)/2;
        if(DoubleEqual(nums[mid],target))
        {
            return mid;
        }
        else if(nums[mid]<target)
        {
            begin=mid+1;
        }
        else
        {
            end=mid-1;
        }
    }
    return -1;
}

int main(int argc, char const *argv[])
{
    std::vector<int> nums{9,5,7,1,2,4,3,8};
    BinarySort(nums);
    std::for_each(nums.begin(),nums.end(),[](auto& iter){ std::cout << iter << " "; });
    std::cout<<std::endl;
    std::cout<<BinarySearch(nums,9)<<std::endl;
    std::cout<<BinarySearch(nums,1)<<std::endl;
    std::cout<<BinarySearch(nums,8)<<std::endl;

    BinarySort<int>(nums,[](const auto& l,const auto& r){ return l>r;});
    std::for_each(nums.begin(),nums.end(),[](auto& iter){ std::cout << iter << " "; });
    std::cout<<std::endl;
    
    std::vector<double> nums1{1.1,8.8,7.9,2.5,6.3,3.4,4.5};
    BinarySort(nums1);
    std::for_each(nums1.begin(),nums1.end(),[](auto& iter){ std::cout << iter << " "; });
    std::cout << std::endl;
    std::cout<<BinarySearch(nums1,4.5)<<std::endl;
    return 0;
}