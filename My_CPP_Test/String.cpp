#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <iostream>
class String
{
public:
    const static size_t MAXLEN = 9223372036854775807;
    String():len(0)
    {
        this->str = (char*)::malloc(1);
        *this->str = '\0';
    }

    String(const char* s)
    {
        if(!s)
        {
            this->len = 0;
            this->str = (char*)::malloc(1);
            *this->str = '\0';
            return;
        }
        int len = ::strnlen(s,MAXLEN);
        if(len != MAXLEN)
        {
            this->len = len;
            this->str = (char*)::malloc(this->len + 1);
            ::strncpy(str,s,len);
            *(this->str + this->len) = '\0';
        }
        else
        {
            this->len = 0;
            this->str = (char*)::malloc(1);
            *this->str = '\0';
        }
    }

    String(const String& s)
    {
        this->len = s.len;
        this->str = (char*)malloc(this->len + 1);
        ::strncpy(this->str,s.str,s.len);
        *(this->str + this->len) = '\0';
    }

    ~String()
    {
        ::free(str);
    }

    String& operator=(const String& s)
    {
        if(this != &s)
        {
            this->len = s.len;
            this->str = (char*)::malloc(this->len + 1);
            ::strcpy(str,s.str);
            *(this->str + this->len) = '\0';
        }
        return *this;
    }

    String& operator+=(const char* s)
    {
        int len = ::strnlen(s,MAXLEN);
        if(len != MAXLEN)
        {
            char* t = this->str;
            this->str = (char*)::malloc(this->len + len + 1);
            ::strncpy(this->str,t,this->len);
            ::strncpy(this->str + this->len,s,len);
            this->len += len;
            *(this->str + this->len) = '\0';
        }
        else
        {
            this->str = (char*)::malloc(1);
            *this->str = '\0';
            this->len = 0;
        }
        return *this;
    }

    String& operator+=(const String& s)
    {
        return this->operator+=(s.C_STR());
    }

    String operator+(const String& s)
    {
        String t(*this);
        t.operator+=(s.C_STR());
        return t;
    }

    String operator+(const char* s)
    {
        String t(*this);
        t.operator+=(s);
        return t;
    }

    int Length()
    {
        return len;
    }

    char* Data()const
    {
        return str;
    }

    const char* C_STR()const
    {
        return str;
    }
protected:
private:
    char* str;
    int len;
};

int main(int argc, char const *argv[])
{
    String s("ABC");
    assert(::strcmp(s.C_STR(),"ABC") == 0);
    assert(s.Length() == 3);
    String t(s);
    assert(::strcmp(t.C_STR(),"ABC") == 0);
    assert(t.Length() == 3);

    String s1;
    assert(::strcmp(s1.C_STR(),"") == 0);
    assert(s1.Length() == 0);

    String s2;
    s2 += "ABC";
    assert(::strcmp(s2.C_STR(),"ABC") == 0);
    assert(s2.Length() == 3);

    String s3 = s2 + s2;
    assert(::strcmp(s3.C_STR(),"ABCABC") == 0);
    assert(s3.Length() == 6);
    assert(::strcmp(s2.C_STR(),"ABC") == 0);
    assert(s2.Length() == 3);

    String s4(s2+s3);
    assert(::strcmp(s4.C_STR(),"ABCABCABC") == 0);
    assert(s4.Length() == 9);

    String s5 = s2;
    s5 += s3;
    assert(::strcmp(s5.C_STR(),"ABCABCABC") == 0);
    assert(s5.Length() == 9);
    return 0;
}
