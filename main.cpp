#include <iostream>
#include <vector>
#include <cassert>
#include <fstream>

uint32_t pack_color(const uint8_t r,const uint8_t g, const uint8_t b, const uint8_t a = 255){
    return (a<<24) + (b<<16) + (g<<8) + r;
}

void unpack_color(const uint32_t &color, uint8_t &r, uint8_t &g, uint8_t &b, uint8_t &a){
    r = (color >> 0) & 255;
    g = (color >> 8) & 255;
    b = (color >> 16) & 255;
    a = (color >> 24) & 255;
}

void gen_ppm(const std::string file, const std::vector<uint32_t> &img, const size_t w, const size_t h){
    assert(img.size() == w*h);
    std::ofstream ofs(file);
    // ppm signature
    ofs << "P6\n" << w << " " << h << "\n255" << std::endl;

    for(size_t i = 0; i < img.size(); ++i){
        uint8_t r,g,b,a;
        unpack_color(img[i],r,g,b,a);
        ofs << static_cast<char>(r) << static_cast<char>(g) << static_cast<char>(b);
    }
    ofs.close();
}

int main() {
    const size_t w = 512, h = 512;
    std::vector<uint32_t> framebuff(w*h, 255);

    for(size_t i = 0; i < h; ++i){
        for(size_t j = 0; j < h; ++j){
            uint8_t r = 255 * j / float(h),
                    g = (255 * i / float(w))/2,
                    b = 255 * i / float(w);
            framebuff[i*w + j] = pack_color(r,g,b);
        }
    }

    gen_ppm("test.ppm", framebuff, w,h);
    return 0;
}
