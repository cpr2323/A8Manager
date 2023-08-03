#pragma once

template <typename T>
class Crc
{
public:
    Crc () = default;
    virtual ~Crc () = default;

    void reset () noexcept
    {
        crc = 0;
    }

    T getCrc () noexcept
    {
        return crc;
    }

    T updateBuffer (uint8_t* data, int len)
    {
        while (len > 0)
        {
            update (*data);
            ++data;
            --len;
        }

        return crc;
    }

    virtual T update (uint8_t byte) = 0;

protected:
    T crc { 0 };
};

class Crc16 : public Crc<uint16_t>
{
public:
    Crc16 () = default;
    virtual ~Crc16 () = default;
    uint16_t update (uint8_t byte) noexcept override
    {
        // C implementation of CRC16
        crc ^= byte;
        for (int i { 0 }; i < 8; ++i)
        {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc = (crc >> 1);
        }

        return crc;
    }
};

class Crc32 : public Crc<uint32_t>
{
public:
    static constexpr uint32_t DELOREAN_CRC32_SEED = 0xEDB88320U;

    virtual ~Crc32 () = default;
    uint32_t update (uint8_t byte) noexcept override
    {
        // C implementation of CRC32
        crc ^= byte;
        for (int j = 7; j >= 0; j--)
        {    // Do eight times.
            //const uint32_t mask = -(crc & 1U);
            // the previous line was producing a warning under VS
            //   warning C4146: unary minus operator applied to unsigned type, result still unsigned
            // I wrote a test and verified the following line produces the same values, and no warning
            const uint32_t mask = (uint32_t)-(int32_t)(crc & 1U);
            crc = (crc >> 1U) ^ (DELOREAN_CRC32_SEED & mask);
        }

        return crc;
    }
};

/*
#include <iostream>

template <typename T>
class SumHelper
{
public:
    // actually a ctor that has a suitable static_assert (etc)
    SumHelper() noexcept
    : accum{}
    {

    }

    using type = T;

    void updateBuffer(uint8_t* data, int len)
    {
        assert(false);
    };
protected:
    T accum;

};

template <>
void SumHelper<uint8_t>::updateBuffer(uint8_t* data, int len)
{
    for (int i = 0; i < len; ++i)
    {
        accum += data[i];
    }
}


template <>
void SumHelper<uint32_t>::updateBuffer(uint8_t* data, int len)
{
    for (int i = 0; i < len; ++i)
    {
        accum += data[i];
    }
}

template <typename T>
class FakeCrc : public SumHelper<T>
{
public:
    FakeCrc(uint8_t* data, int len)
    : SumHelper<T>()
    {
        SumHelper<T>::updateBuffer(data, len);
    }

    using type = typename SumHelper<T>::type;

    type getSum() const { return SumHelper<T>::accum; }

private:


};

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";

    uint8_t data[] = {100, 101, 102, 103, 104};

    using Fake8 = SumHelper<uint8_t>;
    using Fake32 = SumHelper<uint32_t>;

    Fake8 eight;
    eight.updateBuffer(data, 5);


    FakeCrc<uint32_t> crc1(data, 5);

    return 0;
}
*/