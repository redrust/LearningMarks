#include <initializer_list>
#include <string.h>
#include <string>
#include <stdint.h>
#include <vector>
// 定义注册数据类型
enum EPrimitiveType
{
    EPT_Int,
    EPT_String,
    EPT_Float
};
// 数据类型信息储存，包括变量名，变量类型，变量在类中的偏移值
class MemberVariable
{
public:
    MemberVariable(const char* inName,
        EPrimitiveType inPrimitiveType, uint32_t inOffset):
    mName(inName),
    mPrimitiveType(inPrimitiveType),
    mOffset(inOffset)
    {}

    EPrimitiveType GetPrimitiveType() const
    {
        return mPrimitiveType;
    }

    uint32_t GetOffset() const
    {
        return mOffset;
    }
protected:
private:
    std::string     mName;
    EPrimitiveType  mPrimitiveType;
    uint32_t        mOffset;
};
// 储存一个类的所有变量信息
class DataType
{
public:
    DataType(std::initializer_list<const MemberVariable> inMVs)
    {
        for(auto& mv : inMVs)
        {
            mMemberVariables.emplace_back(mv);
        }
    }

    const std::vector<MemberVariable>& GetMemberVariables()
    {
        return mMemberVariables;
    }
protected:
private:
    std::vector<MemberVariable> mMemberVariables;
};

#define OffsetOf(c, mv) ((size_t) & (static_cast<c*>(nullptr)->mv))

class MouseStatus
{
public:
    std::string      mName;
    int              mLegCount, mHeadCount;
    float            mHealth;
    static DataType* sDataType; // 侵入式反射系统
    static void InitDataType()
    {
        sDataType = new DataType(
        {
            MemberVariable("mName", EPT_String, OffsetOf(MouseStatus, mName)),
            MemberVariable("mLegCount", EPT_Int, OffsetOf(MouseStatus, mLegCount)),
            MemberVariable("mHeadCount", EPT_Int, OffsetOf(MouseStatus, mHeadCount)),
            MemberVariable("mHealth", EPT_Float, OffsetOf(MouseStatus, mHealth))
        });
    }
protected:
private:
};

inline uint16_t ByteSwap2(uint16_t inData)
{
    return (inData >> 8) | (inData << 8);
}

template<typename T, size_t tSize>
class ByteSwapper;

template<typename T, size_t tSize>
class ByteSwapper<T, 2>
{
public:
    T Swap(T inData)const
    {
        uint16_t result = ByteSwap2(TypeAliaser<T, uint16_t>(inData).Get());
        return TypeAliaser<uint16_t, T>(inData).Get();
    }
protected:
private:
};

template<typename T>
T ByteSwap(T inData)
{
    return ByteSwapper<T, sizeof(T)>().Swap(inData);
}

class MemoryStream
{
public:
    virtual void Serialize(void* inData, uint32_t inByteCount) = 0;
    virtual bool IsInput() const = 0;

    template<typename T>
    void Serialize(T& ioData)
    {
        static_assert(std::is_arithmetic<T>::value||std::is_enum<T>::value,
                      "Generic Serialize only supports primitive data types");
        if(STREAM_ENDIANNESS == PLATFORM_ENDIANNESS)
        {
            Serialize(&ioData, sizeof(ioData));
        }
        else
        {
            if(IsInput())
            {
                T data;
                Serialize(&data, sizeof(T));
                ioData = ByteSwap(data);
            }
            else
            {
                T swappedData = ByteSwap(ioData);
                Serialize(&swappedData, sizeof(swappedData));
            }
        }
    }
protected:
private:
};

class InputMemoryStrema : public MemoryStream
{
public:
    virtual void Serialize(void* inData, uint32_t inByteCount)
    {
        Read(inData, inByteCount);
    }
    virtual bool IsInput() const
    {
        return true;
    }
protected:
private:
};

class OutputMemoryStream : public MemoryStream
{
public:
    virtual void Serialize(void* inData, uint32_t inByteCount)
    {
        Write(inData, inByteCount);
    }
    virtual bool IsInput() const
    {
        return false;
    }
protected:
private:
};

void Serialize(MemoryStream* inMemoryStream, const DataType* inDataType, uint8_t* inData)
{
    for(auto& mv : inDataType->GetMemberVariables())
    {
        void* mvData = inData + mv.GetOffset();
        switch (mv.GetPrimitiveType())
        {
        case EPT_Int:
            inMemoryStream->Serialize(*(int*)mvData);
            break;
        case EPT_String:
            inMemoryStream->Serialize(*(std::string*)mvData);
            break;
        case EPT_Float:
            inMemoryStream->Serialize(*(float*)mvData);
            break;
        default:
            break;
        }
    }
}
