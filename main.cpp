#include <iostream>
#include <vector>
#include <cassert>
#include <fstream>

// ##############################   COLOR   ################################
uint32_t pack_color(const uint8_t r,const uint8_t g, const uint8_t b, const uint8_t a = 255){
    return (a<<24) + (b<<16) + (g<<8) + r;
}

void unpack_color(const uint32_t &color, uint8_t &r, uint8_t &g, uint8_t &b, uint8_t &a){
    r = (color >> 0) & 255;
    g = (color >> 8) & 255;
    b = (color >> 16) & 255;
    a = (color >> 24) & 255;
}
// ##############################   END COLOR   ############################

void draw_rectangle(std::vector<uint32_t> &img, const size_t img_w, const size_t img_h, const size_t x, const size_t y, const size_t w, const size_t h, const uint32_t color){
    assert(img.size()==img_w*img_h);
    for (size_t i=0; i<w; i++) {
        for (size_t j=0; j<h; j++) {
            img[x+i + (y+j)*img_w] = color;
        }
    }
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
    const size_t w = 512, h = 512, map_w = 16, map_h = 16;
    std::vector<uint32_t> framebuff(w*h, 255);
    const char map[] = "0000222222220000"\
                       "1              0"\
                       "1      11111   0"\
                       "1     0        0"\
                       "0     0  1110000"\
                       "0     3        0"\
                       "0   10000      0"\
                       "0   0   11100  0"\
                       "0   0   0      0"\
                       "0   0   1  00000"\
                       "0       1      0"\
                       "2       1      0"\
                       "0       0      0"\
                       "0 0000000      0"\
                       "0              0"\
                       "0002222222200000";

    // DRAW BACKGROUND
    for(size_t i = 0; i < h; ++i){
        for(size_t j = 0; j < h; ++j){
            uint8_t r = 255 * j / float(h),
                    g = (255 * i / float(w))/2,
                    b = 255 * i / float(w);
            framebuff[i*w + j] = pack_color(r,g,b);
        }
    }

    // DRAW MAP WALLS
    const size_t block_w = w/map_w,
                 block_h = h/map_h;
    for(size_t i = 0; i < map_h; i++){
        for(size_t j = 0; j < map_w; j++){
            if (map[ i*map_w + j ] == ' ') continue;
            // Render deep purple rectangle where map need isnt ' '
            draw_rectangle(framebuff, w,h, block_w*j, block_h*i, block_w, block_h, pack_color(216,191,216));
        }
    }

    gen_ppm("test.ppm", framebuff, w,h);
    return 0;
}
