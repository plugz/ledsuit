#ifndef __RgbCOLOR_HPP__
#define __RgbCOLOR_HPP__

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
