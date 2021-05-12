#include <iostream>
#include <vector>
#include <cassert>
#include <fstream>
#include <cmath>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <chrono>
#include <thread>

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

void updateSDL(SDL_Renderer* ren,const std::vector<uint32_t> &img, const size_t w, const size_t h){
    assert(img.size() == w*h);
    for(size_t i = 0; i < img.size(); ++i){
        uint8_t r,g,b,a;
        unpack_color(img[i],r,g,b,a);
        SDL_SetRenderDrawColor(ren,r,g,b,a);
        SDL_RenderDrawPoint(ren, i%w, int(i/w));
    }
}

void keyPressed(SDL_Event& e, float &player_a, float &player_x, float &player_y){
    switch (e.key.keysym.sym) {
        case SDLK_z:
            player_x += 0.1 * cos(player_a);
            player_y += 0.1 * sin(player_a);
            break;
        case SDLK_q:
            player_a -= 0.1;
            break;
        case SDLK_s:
            player_x -= 0.1 * cos(player_a);
            player_y -= 0.1 * sin(player_a);
            break;
        case SDLK_d:
            player_a += 0.1;
            break;
    }
}

void drawBg(std::vector<uint32_t> &framebuff, const size_t w, const size_t h){
    for(size_t i = 0; i < h; ++i){
        for(size_t j = 0; j < h; ++j){
            uint8_t r = 255 * j / float(h),
                    g = (255 * i / float(w))/2,
                    b = 255 * i / float(w);
            uint32_t color =  pack_color(r,g,b);
            framebuff[i*w + j] = color;
            framebuff[i*w + j + w/2] = color;
        }
    }
}

void drawMap(std::vector<uint32_t> &framebuff, const char map[], const size_t w, const size_t h, const size_t map_w, const size_t map_h, const size_t block_w, const size_t block_h){
    for(size_t i = 0; i < map_h; i++){
        for(size_t j = 0; j < map_w; j++){
            if (map[ i*map_w + j ] == ' ') continue;
            // Render deep purple rectangle where map need isnt ' '
            draw_rectangle(framebuff, w,h, block_w*j, block_h*i, block_w, block_h, pack_color(216,191,216));
        }
    }
}

void drawRayCast(std::vector<uint32_t> &framebuff, const char map[], const size_t w, const size_t h, const size_t map_w, const size_t map_h, const size_t block_w, const size_t block_h, const float fov, float player_a, float player_x, float player_y){
    for (int j = 0; j < w/2; ++j) {
        float angle = player_a-fov/2 + fov*j/float(w/2);
        for (float i = 0; i < 20; i+=0.1) {
            float   cx = player_x + i*cos(angle),
                    cy = player_y + i*sin(angle);

            size_t px_x = cx*block_w, px_y = cy*block_h;
            framebuff[px_x + px_y*w] = pack_color(255,255,255);

            if(map[int(cx) + int(cy)*map_w] != ' ') {
                size_t col_h = h/i;
                draw_rectangle(framebuff, w, h, w/2+j, h/2-col_h/2, 1, col_h, pack_color(0, 255, 255));
                break;
            }
        }
    }
}

void updateView(SDL_Renderer *renderer,std::vector<uint32_t> &framebuff, const char map[], const size_t w, const size_t h, const size_t map_w, const size_t map_h, const size_t block_w, const size_t block_h, const float fov, float player_a, float player_x, float player_y){
    drawBg(framebuff, w, h);
    drawMap(framebuff, map, w, h, map_w,map_h,block_w, block_h);
    drawRayCast(framebuff, map, w, h, map_w, map_h, block_w,block_h,fov,player_a,player_x, player_y);

    SDL_RenderClear(renderer);
    updateSDL(renderer,framebuff, w, h);
    SDL_RenderPresent(renderer);
}

void gameLoopWait(std::chrono::system_clock::time_point &a, std::chrono::system_clock::time_point &b){
    a = std::chrono::system_clock::now();
    std::chrono::duration<double, std::milli> work_t = a-b;
    if (work_t.count() < 200.0)
    {
        std::chrono::duration<double, std::milli> delta_ms(200.0 - work_t.count());
        auto delta_ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
        std::this_thread::sleep_for(std::chrono::milliseconds(delta_ms_duration.count()));
    }

    b = std::chrono::system_clock::now();
    std::chrono::duration<double, std::milli> sleep_time = b - a;
}

int main() {
    const size_t w = 1024, h = 512, map_w = 16, map_h = 16;
    const float fov = M_PI/3.0;
    float player_x = 3.5, player_y = 2.5, player_a = 1.5;
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
    const size_t block_w = w/(map_w*2),
            block_h = h/map_h;
    std::chrono::system_clock::time_point timer_a = std::chrono::system_clock::now();
    std::chrono::system_clock::time_point timer_b = std::chrono::system_clock::now();

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("error initializing SDL: %s\n", SDL_GetError());
    }
    SDL_Window* win;
    SDL_Renderer* renderer;
    SDL_CreateWindowAndRenderer(w,h,0, &win, &renderer);

    SDL_Event event;
    while (true) {
        gameLoopWait(timer_a, timer_b);
        if(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_QUIT:
                    exit(0);
                case SDL_KEYDOWN:
                    keyPressed(event, player_a, player_x, player_y);
                    break;
            }
        }
        updateView(renderer,framebuff, map, w, h, map_w, map_h, block_w,block_h,fov,player_a,player_x, player_y);
    }

    return 0;
}
