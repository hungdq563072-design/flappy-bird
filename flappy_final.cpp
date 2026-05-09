/*
 * ============================================================
 *  FLAPPY BIRD  -  C++ + SDL2  (Windows)
 *  Biên dịch:
 *    g++ flappy_final.cpp -o flappy_bird.exe -lSDL2main -lSDL2 -mwindows
 * ============================================================
 */

#include <SDL2/SDL.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>
#include <algorithm>

// ─── Kích thước cửa sổ ──────────────────────────────────────
static constexpr int W = 45;
static constexpr int H = 110;

// ─── Hằng số gameplay ───────────────────────────────────────
static constexpr float GRAVITY       = 0.3f;
static constexpr float JUMP_VY       = -6.5f;
static constexpr float PIPE_SPEED    = 1.8f;
static constexpr int   PIPE_W        = 60;
static constexpr int   PIPE_GAP      = 160;
static constexpr int   PIPE_INTERVAL = 170;
static constexpr int   GROUND_H      = 60;
static constexpr int   BIRD_R        = 10;

// ─── Màu sắc nền ────────────────────────────────────────────
static SDL_Color skyTop    = {91,  200, 245, 255};
static SDL_Color skyBot    = {200, 237, 255, 255};
static SDL_Color colGrass  = {107, 191,  58, 255};
static SDL_Color colGround = {212, 168,  67, 255};
static SDL_Color colPipe   = {58,  206,  90, 255};
static SDL_Color colPipeDk = {40,  160,  64, 255};
static SDL_Color colPipeCp = {47,  184,  74, 255};
static SDL_Color colHill1  = {126, 200,  80, 255};
static SDL_Color colHill2  = { 91, 170,  50, 255};
static SDL_Color colWhite  = {255, 255, 255, 255};
static SDL_Color colBlack  = {  0,   0,   0, 255};
static SDL_Color colSun    = {255, 229, 102, 255};

// ─── Skin con chim ──────────────────────────────────────────
struct BirdSkin {
    const char* name;
    SDL_Color   body;
    SDL_Color   bodyDark;
    SDL_Color   belly;
    SDL_Color   beak;
};

static const BirdSkin SKINS[] = {
    { "VANG",    {255, 215,   0, 255}, {220, 170,   0, 255}, {255, 240, 140, 255}, {255, 123,   0, 255} },
    { "XANH LA", { 80, 200,  80, 255}, { 40, 150,  40, 255}, {160, 230, 130, 255}, {255, 180,   0, 255} },
    { "XANH DA", { 60, 160, 255, 255}, { 30, 110, 210, 255}, {160, 210, 255, 255}, {255, 100,  50, 255} },
    { "DO",      {230,  60,  60, 255}, {180,  20,  20, 255}, {255, 150, 130, 255}, {255, 200,   0, 255} },
    { "TIM",     {180,  80, 220, 255}, {130,  40, 170, 255}, {220, 170, 255, 255}, {255, 160,  50, 255} },
    { "CAM",     {255, 140,   0, 255}, {210,  90,   0, 255}, {255, 200, 120, 255}, { 80,  60, 200, 255} },
    { "HONG",    {255, 130, 180, 255}, {210,  70, 130, 255}, {255, 200, 220, 255}, {255,  80,  80, 255} },
    { "TRANG",   {240, 240, 240, 255}, {180, 180, 180, 255}, {255, 255, 255, 255}, {255, 140,   0, 255} },
};
static constexpr int SKIN_COUNT = 8;

// ─── Tiện ích vẽ ────────────────────────────────────────────
static void setColor(SDL_Renderer* r, SDL_Color c, Uint8 alpha = 255) {
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, alpha);
}
static void fillRect(SDL_Renderer* r, int x, int y, int w, int h, SDL_Color c, Uint8 a = 255) {
    setColor(r, c, a);
    SDL_Rect rect{x, y, w, h};
    SDL_RenderFillRect(r, &rect);
}
static void fillCircle(SDL_Renderer* r, int cx, int cy, int radius, SDL_Color c, Uint8 a = 255) {
    setColor(r, c, a);
    for (int dy = -radius; dy <= radius; dy++) {
        int dx = (int)sqrt((double)(radius * radius - dy * dy));
        SDL_RenderDrawLine(r, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}
static void fillEllipse(SDL_Renderer* r, int cx, int cy, int rx, int ry, SDL_Color c, Uint8 a = 255) {
    setColor(r, c, a);
    for (int dy = -ry; dy <= ry; dy++) {
        float ratio = (float)dy / ry;
        int dx = (int)(rx * sqrt(1.0f - ratio * ratio));
        SDL_RenderDrawLine(r, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}
static void fillTriangle(SDL_Renderer* r, int x0,int y0,int x1,int y1,int x2,int y2, SDL_Color c) {
    setColor(r, c);
    if (y0 > y1) { std::swap(x0,x1); std::swap(y0,y1); }
    if (y0 > y2) { std::swap(x0,x2); std::swap(y0,y2); }
    if (y1 > y2) { std::swap(x1,x2); std::swap(y1,y2); }
    for (int y = y0; y <= y2; y++) {
        float t02 = (y0==y2)?1.f:(float)(y-y0)/(y2-y0);
        int xa = x0+(int)(t02*(x2-x0)), xb;
        if (y < y1) { float t01=(y0==y1)?1.f:(float)(y-y0)/(y1-y0); xb=x0+(int)(t01*(x1-x0)); }
        else        { float t12=(y1==y2)?1.f:(float)(y-y1)/(y2-y1); xb=x1+(int)(t12*(x2-x1)); }
        if (xa>xb) std::swap(xa,xb);
        SDL_RenderDrawLine(r, xa, y, xb, y);
    }
}

// ─── Bitmap font 5×7 ────────────────────────────────────────
static void drawDigit(SDL_Renderer* r, int digit, int x, int y, int size, SDL_Color c) {
    const int segs[10] = {
        0b1110111,0b0010010,0b1011101,0b1011011,0b0111010,
        0b1101011,0b1101111,0b1010010,0b1111111,0b1111011
    };
    if (digit<0||digit>9) return;
    int s=segs[digit], t=size, h2=size/2, th=size/6;
    setColor(r,c);
    if (s&0b1000000){SDL_Rect rc{x,y,t,th};        SDL_RenderFillRect(r,&rc);}
    if (s&0b0100000){SDL_Rect rc{x,y,th,h2};        SDL_RenderFillRect(r,&rc);}
    if (s&0b0010000){SDL_Rect rc{x+t-th,y,th,h2};   SDL_RenderFillRect(r,&rc);}
    if (s&0b0001000){SDL_Rect rc{x,y+h2-th/2,t,th}; SDL_RenderFillRect(r,&rc);}
    if (s&0b0000100){SDL_Rect rc{x,y+h2,th,h2};      SDL_RenderFillRect(r,&rc);}
    if (s&0b0000010){SDL_Rect rc{x+t-th,y+h2,th,h2};SDL_RenderFillRect(r,&rc);}
    if (s&0b0000001){SDL_Rect rc{x,y+t-th,t,th};    SDL_RenderFillRect(r,&rc);}
}
static void drawNumber(SDL_Renderer* r, int num, int cx, int y, int size, SDL_Color c) {
    std::string s = std::to_string(num);
    int totalW = (int)s.size()*(size+4);
    int startX = cx-totalW/2;
    for (char ch:s) { drawDigit(r,ch-'0',startX,y,size,c); startX+=size+4; }
}

// ─── Structs ─────────────────────────────────────────────────
struct Particle { float x,y,vx,vy,life,radius; SDL_Color color; };
struct Pipe {
    float x; int topH; bool scored; float shake;
    int botY() const { return topH+PIPE_GAP; }
    int botH() const { return H-GROUND_H-botY(); }
};
struct Bird  { float x,y,vy,angle; int flapFrame; };
struct Cloud { float x,y,w,speed,alpha; };

// ─── Trạng thái ──────────────────────────────────────────────
enum class State { MENU, IDLE, PLAY, DEAD };

// ════════════════════════════════════════════════════════════
//  Lớp Game
// ════════════════════════════════════════════════════════════
class FlappyGame {
public:
    SDL_Renderer* ren;
    State  state;
    Bird   bird;
    std::vector<Pipe>     pipes;
    std::vector<Particle> particles;
    std::vector<Cloud>    clouds;
    int   score, best, frame, pipeTimer;
    float bgOffset;
    int   skinIndex;   // skin đang chọn

    // ── Glyph font ────────────────────────────────────────
    struct Glyph { Uint8 rows[7]; };
    static const Glyph& getGlyph(char c);

    // ── Constructor ───────────────────────────────────────
    FlappyGame(SDL_Renderer* r) : ren(r), best(0), skinIndex(0) {
        srand((unsigned)time(nullptr));
        reset();
        state = State::MENU;   // bắt đầu bằng menu
    }

    // ── Reset game ────────────────────────────────────────
    void reset() {
        state     = State::IDLE;
        score     = 0;
        frame     = 0;
        pipeTimer = 0;
        bgOffset  = 0;
        bird = {90.f, H/2.f, 0.f, 0.f, 0};   // giữa màn hình
        pipes.clear();
        particles.clear();
        clouds.clear();
        for (int i=0;i<5;i++) {
            clouds.push_back({
                (float)(i*110+rand()%40),
                (float)(30+rand()%70),
                (float)(60+rand()%50),
                0.18f+(rand()%10)*0.015f,
                0.7f+(rand()%3)*0.1f
            });
        }
    }

    // ── Điều khiển menu ───────────────────────────────────
    void prevSkin() { skinIndex=(skinIndex-1+SKIN_COUNT)%SKIN_COUNT; }
    void nextSkin() { skinIndex=(skinIndex+1)%SKIN_COUNT; }
    void startFromMenu() {
        if (state==State::MENU) { reset(); state=State::IDLE; }
    }

    // ── Vỗ cánh ──────────────────────────────────────────
    void flap() {
        if (state==State::MENU)  return;
        if (state==State::IDLE)  state=State::PLAY;
        if (state==State::PLAY)  { bird.vy=JUMP_VY; bird.flapFrame=frame; }
        if (state==State::DEAD)  state=State::MENU;
    }

    // ── Spawn pipe ────────────────────────────────────────
    void spawnPipe() {
        int minTop=80, maxTop=H-GROUND_H-PIPE_GAP-80;
        pipes.push_back({(float)(W+20), minTop+rand()%(maxTop-minTop), false, 0.f});
    }

    // ── Spawn particles ───────────────────────────────────
    void spawnParticles(float px, float py) {
        SDL_Color cols[]={{255,229,102,255},{255,140,66,255},{255,92,92,255},{126,245,162,255},{200,237,255,255}};
        for (int i=0;i<18;i++) {
            float ang=(float)(rand()%628)/100.f, sp=1.5f+(rand()%30)/10.f;
            particles.push_back({px,py,cosf(ang)*sp,sinf(ang)*sp-2.f,1.f,2.f+(rand()%3)*1.f,cols[rand()%5]});
        }
    }

    // ── Va chạm ───────────────────────────────────────────
    bool hitTest() {
        float bx=bird.x, by=bird.y;
        if (by+BIRD_R>=H-GROUND_H||by-BIRD_R<=0) return true;
        for (auto& p:pipes) {
            if (bx+BIRD_R-4>p.x+4&&bx-BIRD_R+4<p.x+PIPE_W-4)
                if (by-BIRD_R+4<p.topH||by+BIRD_R-4>p.botY()) return true;
        }
        return false;
    }

    // ── Update ────────────────────────────────────────────
    void update() {
        frame++;
        bgOffset += (state==State::PLAY ? PIPE_SPEED : 0.4f);
        for (auto& cl:clouds) {
            cl.x -= cl.speed*(state==State::PLAY?1.f:0.15f);
            if (cl.x+cl.w<-20) cl.x=W+20;
        }
        if (state==State::MENU) {
            bird.y = H/2.f + sinf(frame*0.05f)*12.f;   // lơ lửng ở menu
            return;
        }
        if (state!=State::PLAY) {
            if (state==State::IDLE) bird.y+=sinf(frame*0.05f)*0.6f;
            return;
        }
        bird.vy   += GRAVITY;
        bird.y    += bird.vy;
        bird.angle = std::max(-0.6f,std::min(1.0f,bird.vy*0.07f));
        pipeTimer++;
        if (pipeTimer>=PIPE_INTERVAL) { spawnPipe(); pipeTimer=0; }
        for (auto& p:pipes) { p.x-=PIPE_SPEED; if(p.shake>0)p.shake*=0.7f; }
        pipes.erase(std::remove_if(pipes.begin(),pipes.end(),[](const Pipe& p){return p.x+PIPE_W<-20;}),pipes.end());
        for (auto& p:pipes) if (!p.scored&&p.x+PIPE_W<bird.x) { p.scored=true; score++; if(score>best)best=score; }
        for (auto& pt:particles) { pt.x+=pt.vx; pt.y+=pt.vy; pt.vy+=0.15f; pt.life-=0.04f; }
        particles.erase(std::remove_if(particles.begin(),particles.end(),[](const Particle& p){return p.life<=0;}),particles.end());
        if (hitTest()) { spawnParticles(bird.x,bird.y); state=State::DEAD; }
    }

    // ════════════════════════════════════════════════════
    //  VẼ
    // ════════════════════════════════════════════════════

    void drawSky() {
        for (int y=0;y<H-GROUND_H;y++) {
            float t=(float)y/(H-GROUND_H);
            SDL_SetRenderDrawColor(ren,
                (Uint8)(skyTop.r+t*(skyBot.r-skyTop.r)),
                (Uint8)(skyTop.g+t*(skyBot.g-skyTop.g)),
                (Uint8)(skyTop.b+t*(skyBot.b-skyTop.b)),255);
            SDL_RenderDrawLine(ren,0,y,W,y);
        }
    }

    void drawSun() {
        int sx=W-80,sy=55;
        SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_BLEND);
        for (int rr=36;rr>=28;rr--) fillCircle(ren,sx,sy,rr,colSun,(Uint8)(30+(36-rr)*5));
        SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_NONE);
        fillCircle(ren,sx,sy,24,colSun);
        fillCircle(ren,sx-5,sy-5,8,{255,245,180,255});
    }

    void drawClouds() {
        SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_BLEND);
        for (auto& cl:clouds) {
            int cx=(int)cl.x,cy=(int)cl.y,rw=(int)(cl.w*0.5f),rh=(int)(cl.w*0.22f);
            Uint8 a=(Uint8)(cl.alpha*230);
            SDL_Color wh={255,255,255,255};
            fillEllipse(ren,cx,cy,rw,rh,wh,a);
            fillEllipse(ren,cx-rw/2,cy+7,rw*2/3,rh*4/5,wh,a);
            fillEllipse(ren,cx+rw/2,cy+6,rw*3/5,rh*4/5,wh,a);
        }
        SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_NONE);
    }

    void drawHills() {
        int off=(int)(bgOffset*0.35f)%200;
        for (int i=0;i<7;i++) fillEllipse(ren,i*100-off,H-GROUND_H,70,45,colHill1);
        for (int i=0;i<7;i++) fillEllipse(ren,i*100-off+50,H-GROUND_H,50,32,colHill2);
    }

    void drawGround() {
        fillRect(ren,0,H-GROUND_H,W,10,colGrass);
        fillRect(ren,0,H-GROUND_H+10,W,GROUND_H-10,colGround);
        SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_BLEND);
        int lineOff=(int)(bgOffset*1.8f)%28;
        for (int i=-1;i<W/28+2;i++) {
            SDL_SetRenderDrawColor(ren,184,138,42,80);
            SDL_Rect rc{i*28+lineOff,H-GROUND_H+14,14,3};
            SDL_RenderFillRect(ren,&rc);
        }
        SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_NONE);
    }

    void drawPipe(const Pipe& p) {
        int px=(int)p.x+(p.shake>0.5f?rand()%3-1:0);
        int capW=PIPE_W+10, capH=18;
        fillRect(ren,px,0,PIPE_W,p.topH-capH,colPipeDk);
        fillRect(ren,px+4,0,7,p.topH-capH,colPipe);
        fillRect(ren,px-5,p.topH-capH,capW,capH,colPipeCp);
        fillRect(ren,px-3,p.topH-capH+4,9,capH-6,{112,232,128,255});
        int botY=p.botY(),botH=p.botH();
        fillRect(ren,px-5,botY,capW,capH,colPipeCp);
        fillRect(ren,px-3,botY+4,9,capH-6,{112,232,128,255});
        fillRect(ren,px,botY+capH,PIPE_W,botH-capH,colPipeDk);
        fillRect(ren,px+4,botY+capH,7,botH-capH,colPipe);
    }

    // ── Vẽ chim với skin bất kỳ (dùng cho cả game lẫn menu preview) ──
    void drawBirdSkin(int bx, int by, const BirdSkin& sk, float wingPhase, bool miniSize=false) {
        int wOff = (int)(sinf(wingPhase)*10);
        if (miniSize) {
            // Phiên bản thu nhỏ cho menu preview
            fillEllipse(ren,bx,by,10,8,sk.bodyDark);
            fillEllipse(ren,bx-1,by-1,9,7,sk.body);
            fillEllipse(ren,bx+2,by+2,5,4,sk.belly);
            fillEllipse(ren,bx-1,by+5+(int)(sinf(wingPhase)*5),7,4,sk.bodyDark);
            fillCircle (ren,bx+4,by-3,4,colWhite);
            fillCircle (ren,bx+5,by-3,2,colBlack);
            fillCircle (ren,bx+6,by-4,1,colWhite);
            fillTriangle(ren,bx+9,by-1,bx+14,by+1,bx+9,by+4,sk.beak);
        } else {
            // Phiên bản đầy đủ cho gameplay
            fillEllipse(ren,bx-2,by+7+wOff,12,7,sk.bodyDark);
            fillEllipse(ren,bx,by,16,13,sk.bodyDark);
            fillEllipse(ren,bx-2,by-2,14,11,sk.body);
            fillEllipse(ren,bx+3,by+3,9,7,sk.belly);
            fillCircle (ren,bx+7,by-4,6,colWhite);
            fillCircle (ren,bx+9,by-4,3,colBlack);
            fillCircle (ren,bx+10,by-5,1,colWhite);
            fillTriangle(ren,bx+14,by,bx+22,by+3,bx+14,by+7,sk.beak);
            SDL_Color beakDk={(Uint8)(sk.beak.r*0.7f),(Uint8)(sk.beak.g*0.7f),(Uint8)(sk.beak.b*0.7f),255};
            fillRect(ren,bx+14,by+3,8,2,beakDk);
        }
    }

    void drawBird() {
        float wp = (frame-bird.flapFrame)*(state==State::PLAY?0.45f:0.15f);
        drawBirdSkin((int)bird.x,(int)bird.y,SKINS[skinIndex],wp,false);
    }

    void drawParticles() {
        SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_BLEND);
        for (auto& pt:particles) {
            fillCircle(ren,(int)pt.x,(int)pt.y,(int)(pt.radius*pt.life+0.5f),pt.color,(Uint8)(pt.life*255));
        }
        SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_NONE);
    }

    void drawScoreInGame() {
        if (state!=State::PLAY) return;
        drawNumber(ren,score,W/2+2,52,36,{0,0,0,120});
        drawNumber(ren,score,W/2,  50,36,colWhite);
    }

    // ── Text helper ───────────────────────────────────────
    void drawChar(int x, int y, char c, int scale, SDL_Color col) {
        const Glyph& g=getGlyph(c);
        setColor(ren,col);
        for (int row=0;row<7;row++)
            for (int col2=0;col2<5;col2++)
                if (g.rows[row]&(1<<(4-col2))) {
                    SDL_Rect rc{x+col2*scale,y+row*scale,scale,scale};
                    SDL_RenderFillRect(ren,&rc);
                }
    }
    void drawText(int cx, int y, const std::string& s, int scale, SDL_Color col) {
        int totalW=(int)s.size()*(5+1)*scale;
        int startX=cx-totalW/2;
        for (char c:s) { drawChar(startX,y,c,scale,col); startX+=(5+1)*scale; }
    }
    void drawBanner(int cx, int y, const std::string& s) {
        drawText(cx+2,y+2,s,3,{0,0,0,255});
        drawText(cx,y,s,3,colWhite);
    }
    void drawSmall(int cx, int y, const std::string& s, SDL_Color col={220,220,220,255}) {
        drawText(cx+1,y+1,s,2,{0,0,0,180});
        drawText(cx,y,s,2,col);
    }

    // ── Menu chọn màu ─────────────────────────────────────
    void drawMenu() {
        // Vẽ nền game phía sau (mây, đồi, chim lơ lửng) trước
        // Overlay tối
        SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(ren,0,0,0,140);
        SDL_Rect full{0,0,W,H};
        SDL_RenderFillRect(ren,&full);

        // Hộp menu
        int boxW=340, boxH=420;
        int boxX=W/2-boxW/2, boxY=H/2-boxH/2;

        // Bóng hộp
        SDL_SetRenderDrawColor(ren,0,0,0,80);
        SDL_Rect shadow{boxX+6,boxY+6,boxW,boxH};
        SDL_RenderFillRect(ren,&shadow);

        // Nền hộp
        SDL_SetRenderDrawColor(ren,15,25,45,235);
        SDL_Rect box{boxX,boxY,boxW,boxH};
        SDL_RenderFillRect(ren,&box);

        // Viền vàng
        SDL_SetRenderDrawColor(ren,255,200,0,200);
        SDL_RenderDrawRect(ren,&box);
        SDL_SetRenderDrawColor(ren,255,200,0,80);
        SDL_Rect boxIn{boxX+2,boxY+2,boxW-4,boxH-4};
        SDL_RenderDrawRect(ren,&boxIn);

        SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_NONE);

        // Tiêu đề
        drawBanner(W/2, boxY+18, "CHON MAU CHIM");

        // Đường kẻ ngang dưới tiêu đề
        SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(ren,255,200,0,100);
        SDL_RenderDrawLine(ren,boxX+20,boxY+52,boxX+boxW-20,boxY+52);
        SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_NONE);

        // Lưới skin: 2 cột × 4 hàng
        int cols=2, cellW=152, cellH=76;
        int gridX=boxX+(boxW-cols*cellW)/2;
        int gridY=boxY+62;

        for (int i=0;i<SKIN_COUNT;i++) {
            int col=i%cols, row=i/cols;
            int cx=gridX+col*cellW, cy=gridY+row*cellH;
            bool sel=(i==skinIndex);

            // Nền ô
            SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_BLEND);
            if (sel) {
                SDL_SetRenderDrawColor(ren,255,200,0,50);
            } else {
                SDL_SetRenderDrawColor(ren,255,255,255,15);
            }
            SDL_Rect cell{cx+4,cy+4,cellW-8,cellH-8};
            SDL_RenderFillRect(ren,&cell);

            // Viền ô
            if (sel) SDL_SetRenderDrawColor(ren,255,215,0,255);
            else     SDL_SetRenderDrawColor(ren,100,120,160,120);
            SDL_RenderDrawRect(ren,&cell);
            SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_NONE);

            // Preview chim mini — chim lớn hơn ở giữa ô
            int previewX = cx + 36;
            int previewY = cy + cellH/2;
            float wPhase = frame*0.12f + i*0.8f;
            drawBirdSkin(previewX, previewY, SKINS[i], wPhase, true);

            // Tên màu
            SDL_Color nameCol = sel ? SDL_Color{255,215,0,255} : SDL_Color{180,200,220,255};
            drawSmall(cx+cellW/2+14, cy+cellH/2-7, SKINS[i].name, nameCol);

            // Dấu check nếu đang chọn
            if (sel) {
                drawSmall(cx+cellW-22, cy+8, ">", {255,215,0,255});
            }
        }

        // Hướng dẫn phím ở dưới
        int tipY = boxY+boxH-52;
        SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(ren,255,255,255,20);
        SDL_Rect tipBg{boxX+10,tipY-8,boxW-20,48};
        SDL_RenderFillRect(ren,&tipBg);
        SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_NONE);

        drawSmall(W/2, tipY,    "< > DE CHON MAU",   {160,180,210,255});
        drawSmall(W/2, tipY+22, "SPACE / ENTER DE CHOI", {255,215,0,255});
    }

    // ── Overlay IDLE / DEAD ───────────────────────────────
    void drawOverlay() {
        SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_BLEND);
        if (state==State::IDLE) {
            SDL_SetRenderDrawColor(ren,0,0,0,70);
            SDL_Rect bg{0,0,W,H}; SDL_RenderFillRect(ren,&bg);
            SDL_SetRenderDrawColor(ren,255,255,255,30);
            SDL_Rect box{W/2-180,H/2-90,360,140}; SDL_RenderFillRect(ren,&box);
            drawBanner(W/2,H/2-45,"FLAPPY BIRD");
            drawSmall(W/2,H/2+5, "NHAN SPACE HOAC CLICK");
            drawSmall(W/2,H/2+30,"TRANH CAC ONG DE GHI DIEM!");
        }
        if (state==State::DEAD) {
            SDL_SetRenderDrawColor(ren,0,0,0,110);
            SDL_Rect bg{0,0,W,H}; SDL_RenderFillRect(ren,&bg);
            SDL_SetRenderDrawColor(ren,40,10,10,180);
            SDL_Rect box{W/2-190,H/2-100,380,170}; SDL_RenderFillRect(ren,&box);
            drawNumber(ren,score,W/2-80,H/2-70,28,{255,215,0,255});
            drawNumber(ren,best, W/2+60,H/2-70,28,{200,200,200,255});
            drawBanner(W/2,H/2-15,"GAME OVER");
            drawSmall(W/2-80,H/2+20,"DIEM");
            drawSmall(W/2+60,H/2+20,"KY LUC");
            drawSmall(W/2,H/2+55,"NHAN SPACE DE TIEP TUC");
        }
        SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_NONE);
    }

    // ── Render toàn frame ─────────────────────────────────
    void render() {
        drawSky();
        drawSun();
        drawClouds();
        drawHills();
        for (auto& p:pipes) drawPipe(p);
        drawGround();
        drawBird();
        drawParticles();
        drawScoreInGame();
        if (state==State::MENU) drawMenu();
        else drawOverlay();
    }
};

// ─── Bitmap font data 5×7 ────────────────────────────────────
static const FlappyGame::Glyph FONT_DATA[] = {
    {{0,0,0,0,0,0,0}},                                          // ' '
    {{0b00100,0b00100,0b00100,0b00100,0b00000,0b00100,0b00000}},// '!'
    {{0b01010,0b01010,0b00000,0b00000,0b00000,0b00000,0b00000}},// '"'
    {{0b01010,0b11111,0b01010,0b01010,0b11111,0b01010,0b00000}},// '#'
    {{0b00100,0b01110,0b10000,0b01110,0b00001,0b01110,0b00100}},// '$'
    {{0b11000,0b11001,0b00010,0b00100,0b01000,0b10011,0b00011}},// '%'
    {{0b01100,0b10010,0b10100,0b01000,0b10101,0b10010,0b01101}},// '&'
    {{0b00100,0b00100,0b00000,0b00000,0b00000,0b00000,0b00000}},// '\''
    {{0b00010,0b00100,0b01000,0b01000,0b01000,0b00100,0b00010}},// '('
    {{0b01000,0b00100,0b00010,0b00010,0b00010,0b00100,0b01000}},// ')'
    {{0b00000,0b10101,0b01110,0b11111,0b01110,0b10101,0b00000}},// '*'
    {{0b00000,0b00100,0b00100,0b11111,0b00100,0b00100,0b00000}},// '+'
    {{0b00000,0b00000,0b00000,0b00000,0b00110,0b00100,0b01000}},// ','
    {{0b00000,0b00000,0b00000,0b11111,0b00000,0b00000,0b00000}},// '-'
    {{0b00000,0b00000,0b00000,0b00000,0b00000,0b00110,0b00110}},// '.'
    {{0b00001,0b00010,0b00100,0b01000,0b10000,0b00000,0b00000}},// '/'
    {{0b01110,0b10001,0b10011,0b10101,0b11001,0b10001,0b01110}},// '0'
    {{0b00100,0b01100,0b00100,0b00100,0b00100,0b00100,0b01110}},// '1'
    {{0b01110,0b10001,0b00001,0b00110,0b01000,0b10000,0b11111}},// '2'
    {{0b11111,0b00010,0b00100,0b00010,0b00001,0b10001,0b01110}},// '3'
    {{0b00010,0b00110,0b01010,0b10010,0b11111,0b00010,0b00010}},// '4'
    {{0b11111,0b10000,0b11110,0b00001,0b00001,0b10001,0b01110}},// '5'
    {{0b00110,0b01000,0b10000,0b11110,0b10001,0b10001,0b01110}},// '6'
    {{0b11111,0b00001,0b00010,0b00100,0b01000,0b01000,0b01000}},// '7'
    {{0b01110,0b10001,0b10001,0b01110,0b10001,0b10001,0b01110}},// '8'
    {{0b01110,0b10001,0b10001,0b01111,0b00001,0b00010,0b01100}},// '9'
    {{0b00000,0b00110,0b00110,0b00000,0b00110,0b00110,0b00000}},// ':'
    {{0b00000,0b00110,0b00110,0b00000,0b00110,0b00100,0b01000}},// ';'
    {{0b00010,0b00100,0b01000,0b10000,0b01000,0b00100,0b00010}},// '<'
    {{0b00000,0b00000,0b11111,0b00000,0b11111,0b00000,0b00000}},// '='
    {{0b10000,0b01000,0b00100,0b00010,0b00100,0b01000,0b10000}},// '>'
    {{0b01110,0b10001,0b00001,0b00110,0b00100,0b00000,0b00100}},// '?'
    {{0b01110,0b10001,0b10111,0b10101,0b10111,0b10000,0b01111}},// '@'
    {{0b01110,0b10001,0b10001,0b11111,0b10001,0b10001,0b10001}},// 'A'
    {{0b11110,0b10001,0b10001,0b11110,0b10001,0b10001,0b11110}},// 'B'
    {{0b01110,0b10001,0b10000,0b10000,0b10000,0b10001,0b01110}},// 'C'
    {{0b11100,0b10010,0b10001,0b10001,0b10001,0b10010,0b11100}},// 'D'
    {{0b11111,0b10000,0b10000,0b11110,0b10000,0b10000,0b11111}},// 'E'
    {{0b11111,0b10000,0b10000,0b11110,0b10000,0b10000,0b10000}},// 'F'
    {{0b01110,0b10001,0b10000,0b10111,0b10001,0b10001,0b01111}},// 'G'
    {{0b10001,0b10001,0b10001,0b11111,0b10001,0b10001,0b10001}},// 'H'
    {{0b01110,0b00100,0b00100,0b00100,0b00100,0b00100,0b01110}},// 'I'
    {{0b00111,0b00010,0b00010,0b00010,0b10010,0b10010,0b01100}},// 'J'
    {{0b10001,0b10010,0b10100,0b11000,0b10100,0b10010,0b10001}},// 'K'
    {{0b10000,0b10000,0b10000,0b10000,0b10000,0b10000,0b11111}},// 'L'
    {{0b10001,0b11011,0b10101,0b10001,0b10001,0b10001,0b10001}},// 'M'
    {{0b10001,0b11001,0b10101,0b10011,0b10001,0b10001,0b10001}},// 'N'
    {{0b01110,0b10001,0b10001,0b10001,0b10001,0b10001,0b01110}},// 'O'
    {{0b11110,0b10001,0b10001,0b11110,0b10000,0b10000,0b10000}},// 'P'
    {{0b01110,0b10001,0b10001,0b10001,0b10101,0b10010,0b01101}},// 'Q'
    {{0b11110,0b10001,0b10001,0b11110,0b10100,0b10010,0b10001}},// 'R'
    {{0b01111,0b10000,0b10000,0b01110,0b00001,0b00001,0b11110}},// 'S'
    {{0b11111,0b00100,0b00100,0b00100,0b00100,0b00100,0b00100}},// 'T'
    {{0b10001,0b10001,0b10001,0b10001,0b10001,0b10001,0b01110}},// 'U'
    {{0b10001,0b10001,0b10001,0b10001,0b01010,0b01010,0b00100}},// 'V'
    {{0b10001,0b10001,0b10001,0b10101,0b10101,0b11011,0b10001}},// 'W'
    {{0b10001,0b10001,0b01010,0b00100,0b01010,0b10001,0b10001}},// 'X'
    {{0b10001,0b10001,0b01010,0b00100,0b00100,0b00100,0b00100}},// 'Y'
    {{0b11111,0b00001,0b00010,0b00100,0b01000,0b10000,0b11111}},// 'Z'
};

const FlappyGame::Glyph& FlappyGame::getGlyph(char c) {
    static const Glyph space{{0,0,0,0,0,0,0}};
    int idx=(int)c-32;
    if (idx<0||idx>=(int)(sizeof(FONT_DATA)/sizeof(FONT_DATA[0]))) return space;
    return FONT_DATA[idx];
}

// ════════════════════════════════════════════════════════════
//  main()
// ════════════════════════════════════════════════════════════
int main(int /*argc*/, char* /*argv*/[]) {
    if (SDL_Init(SDL_INIT_VIDEO)!=0) { SDL_Log("SDL_Init: %s",SDL_GetError()); return 1; }

    SDL_Window* window=SDL_CreateWindow(
        "Flappy Bird - C++ SDL2",
        SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,
        W,H,SDL_WINDOW_SHOWN);
    if (!window) { SDL_Log("Window: %s",SDL_GetError()); SDL_Quit(); return 1; }

    SDL_Renderer* renderer=SDL_CreateRenderer(window,-1,
        SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) { SDL_Log("Renderer: %s",SDL_GetError()); SDL_DestroyWindow(window); SDL_Quit(); return 1; }

    FlappyGame game(renderer);
    bool running=true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type==SDL_QUIT) running=false;

            if (event.type==SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:
                        if (game.state==State::MENU) game.prevSkin();
                        break;
                    case SDLK_RIGHT:
                        if (game.state==State::MENU) game.nextSkin();
                        break;
                    case SDLK_RETURN:
                    case SDLK_KP_ENTER:
                        game.startFromMenu();
                        break;
                    case SDLK_SPACE:
                        if (game.state==State::MENU) game.startFromMenu();
                        else game.flap();
                        break;
                    case SDLK_r:
                        game.state=State::MENU;
                        break;
                    case SDLK_ESCAPE:
                        running=false;
                        break;
                }
            }
            if (event.type==SDL_MOUSEBUTTONDOWN&&event.button.button==SDL_BUTTON_LEFT) {
                if (game.state==State::MENU) game.startFromMenu();
                else game.flap();
            }
        }

        game.update();
        SDL_SetRenderDrawColor(renderer,0,0,0,255);
        SDL_RenderClear(renderer);
        game.render();
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
