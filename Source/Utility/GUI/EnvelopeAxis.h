#pragma once

class EnvelopeAxis
{
public:
    EnvelopeAxis () = default;

    double percentage { 0.0 };
    bool locked { false };
    bool followLeft { false };
    bool followRight { false };

private:
};
