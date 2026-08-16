#ifndef __RgbCOLOR_HPP__
#define __RgbCOLOR_HPP__

namespace Utils {
template<typename T> T clamp(T const& min, T const& val, T const& max) { return std::max(min, std::min(max, val)); }
}

class RgbColor {
public:
    RgbColor() {}
    RgbColor(uint8_t r, uint8_t g, uint8_t b) : _r(r), _g(g), _b(b) {}

    RgbColor& operator +=(RgbColor const& other) {
        _r = addComponent(_r, other._r);
        _g = addComponent(_g, other._g);
        _b = addComponent(_b, other._b);

        return *this;
    }

    RgbColor operator *(float multiplier) const {
        int32_t r = _r * multiplier;
        int32_t g = _g * multiplier;
        int32_t b = _b * multiplier;
        r = std::max((int32_t)0, std::min((int32_t)0xff, r));
        g = std::max((int32_t)0, std::min((int32_t)0xff, g));
        b = std::max((int32_t)0, std::min((int32_t)0xff, b));
        return {(uint8_t)r, (uint8_t)g, (uint8_t)b};
    }

    uint8_t r() const { return _r; }
    uint8_t g() const { return _g; }
    uint8_t b() const { return _b; }

    static uint8_t addComponent(uint8_t l, uint8_t r) {
        uint32_t sum = l + r;
        return std::min((uint32_t)(0xff), sum);
    }

private:
    uint8_t _r = 0;
    uint8_t _g = 0;
    uint8_t _b = 0;
};

#endif
