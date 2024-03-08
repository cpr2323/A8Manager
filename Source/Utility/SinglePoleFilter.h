#pragma once

class SinglePoleFilter
{
public:
    SinglePoleFilter (float a)
    {
        config (a);
    }

    SinglePoleFilter ()
        : SinglePoleFilter (1.0f)
    {
    }

    float doFilter (float data)
    {
        mZ = (data * mB) + (mZ * mA);
        return mZ;
    }

    float getConfig ()
    {
        return mA;
    }

    void config (float a)
    {
        mA = a;
        mB = 1.0f - mA;
        mZ = 0.0f;
    }

    void setCurValue (float z)
    {
        mZ = z;
    }

    float getFilteredValue (void)
    {
        return mZ;
    }

private:
    float mA { 0.0f };
    float mB { 0.0f };
    float mZ { 0.0f };
};
