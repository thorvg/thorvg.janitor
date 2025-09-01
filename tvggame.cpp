/*
 * Copyright (c) 2025 the ThorVG project. All rights reserved.

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "template.h"
#include "assets.h"

/************************************************************************/
/* Math Utility                                                         */
/************************************************************************/

static inline Point operator-(const Point& lhs, const Point& rhs)
{
    return {lhs.x - rhs.x, lhs.y - rhs.y};
}

static inline Point operator+(const Point& lhs, const Point& rhs)
{
    return {lhs.x + rhs.x, lhs.y + rhs.y};
}

static inline void operator+=(Point& lhs, const Point& rhs)
{
    lhs.x += rhs.x;
    lhs.y += rhs.y;
}

static float length2(const Point& pt)
{
    return (pt.x * pt.x + pt.y * pt.y);
}

static bool intersect(const Point& a, const Point& b, float dist)
{
    return (length2(a - b) < dist) ? true : false;
}

static void normalize(Point& pt)
{
    auto length = 1.0f / sqrtf(length2(pt));
    pt.x *= length;
    pt.y *= length;
}

static void extend(Point& pt, float length) {
    auto mag = std::sqrt(length2(pt));
    pt.x *= length / mag;
    pt.y *= length / mag;
}

template <typename T>
static inline T lerp(const T &start, const T &end, float t)
{
    return static_cast<T>(start + (end - start) * t);
}

/************************************************************************/
/* Core Game Logic                                                      */
/************************************************************************/

#define _S(a) ((a) * SCALE)     //scale multiplier
#define WIDTH 3840              //base resolution
#define HEIGHT 2160             //base resolution

static float SCALE;             //scale factor
static size_t SWIDTH, SHEIGHT;  //scaled resolution
static size_t LEVEL = 4;        //game level (0 ~ 9)

struct Tween {
    uint32_t at;
    float duration;
};

struct Color
{
    uint8_t r, g, b;
};

struct WarZone
{
    #define GALAXY_LAYER 4
    #define STARS_PER_LAYER 100

    Point min = {-2000, -1180}, max = {2000, 1180};
    Point bound = {(WIDTH - max.x) * 0.5f, (HEIGHT - max.y) * 0.5f};
    Scene* model;
    Shape* galaxy[GALAXY_LAYER];

    float w() { return max.x - min.x; }
    float h() { return max.y - min.y; }

    void star(Canvas* canvas, int i, int ox, int oy, int dx, int dy)
    {
        galaxy[i] = Shape::gen();
        auto size = _S(2 * (i+2));
        auto rx = (ox + dx * 2);
        auto ry = (oy + dy * 2);

        for (int s = 0; s < STARS_PER_LAYER; ++s) {
            galaxy[i]->appendRect(_S(rand() % rx - dx), _S(rand() % ry - dy), size, size);
        }
        auto c = 200 + rand() % 55;
        galaxy[i]->fill(c, c, c);
        canvas->push(galaxy[i]);
    }

    void init(Canvas* canvas)
    {
        auto halo = Picture::gen();
        halo->load((const char*)HALO_DATA, sizeof(HALO_DATA), "jpg");
        canvas->push(halo);

        //generate stars
        for (int i = 0; i < GALAXY_LAYER; ++i) {
            star(canvas, i, WIDTH, HEIGHT, _S(150) * i, _S(150) * i);
        }

        //blue grids
        model = Scene::gen();
        model->scale(SCALE);

        auto dx = (max.x - min.x) / 30;
        auto dy = HEIGHT / 18;
        auto lwidth = 2.0f;
        int i = 0;

        for (int x = min.x + dx; x < max.x; x += dx, ++i) {
            auto grid = Shape::gen();
            if ((i + 1) % 5 == 0) {
                grid->appendRect(x, min.y, lwidth * 3, h());
                grid->fill(50, 50, 175);
            } else {
                grid->appendRect(x, min.y, lwidth, h());
                grid->fill(50, 50, 125);
            }
            model->push(grid);
        }
        i = 0;
        for (int y = min.y + dy; y < max.y; y += dy, ++i) {
            auto grid = Shape::gen();
            if ((i + 1) % 5 == 0) {
                grid->appendRect(min.x, y, w(), lwidth * 2);
                grid->fill(50, 50, 175);
            } else {
                grid->appendRect(min.x, y, w(), lwidth);
                grid->fill(50, 50, 125);
            }
            model->push(grid);
        }

        Scene* wrapper;

        //ring border top
        wrapper = Scene::gen();
        wrapper->push(SceneEffect::GaussianBlur, _S(10.0f), 2, 0, 30);
        auto top = Shape::gen();
        top->appendRect(min.x, min.y, w(), 10);
        top->fill(255, 100, 100);
        wrapper->push(top);
        model->push(wrapper);

        //ring border left
        wrapper = Scene::gen();
        wrapper->push(SceneEffect::GaussianBlur, _S(10.0f), 1, 0, 30);
        auto left = Shape::gen();
        left->appendRect(min.x, min.y, 10, h());
        left->fill(0, 255, 255);
        wrapper->push(left);
        model->push(wrapper);

        //ring border right
        wrapper = Scene::gen();
        wrapper->push(SceneEffect::GaussianBlur, _S(10.0f), 1, 0, 30);
        auto right = Shape::gen();
        right->appendRect(max.x - 5, min.y, 10, h());
        right->fill(170, 255, 170);
        wrapper->push(right);
        model->push(wrapper);

        //ring border bottom
        wrapper = Scene::gen();
        wrapper->push(SceneEffect::GaussianBlur, _S(10.0f), 2, 0, 30);
        auto bottom = Shape::gen();
        bottom->appendRect(min.x, max.y, w(), 10);
        bottom->fill(255, 170, 255);
        wrapper->push(bottom);
        model->push(wrapper);

        canvas->push(model);
    }

    void shift(const Point& player)
    {
        auto x = player.x - SWIDTH/2;
        auto y = player.y - SHEIGHT/2;

        for (int i = 0; i < GALAXY_LAYER; ++i) {
            galaxy[i]->translate(-x * _S((i+1) * 0.2), -y * _S((i+1) * 0.2));
        }
    }

    void update(const Point& shift)
    {
        model->translate(shift.x, shift.y);
    }
};

struct Launcher
{
    #define FIRESPEED 500
    #define MISSLE_MAX 5
    #define FIREDISTANCE _S(2700)

    struct Fire
    {
        Scene* model;
        Point from, to, cur;
        float time;
        bool active;

        bool hit(const Point& target, float range)
        {
            if (active && intersect(cur, target, range)) {
                inactivate();
                return true;
            }
            return false;
        }

        void inactivate()
        {
            model->opacity(0);
            active = false;
        }
    };

    Shape* clipper;
    vector<Fire> missles;
    size_t actives = 0;
    uint32_t lastshot = 0;
    float fireRate = 150.0f;

    void init(Canvas* canvas, float offset, Shape* clipper)
    {
        missles.reserve(MISSLE_MAX);

        auto model = Scene::gen();
        model->clip(clipper);
        canvas->push(model);
        this->clipper = clipper;

        for (int i = 0; i < MISSLE_MAX; ++i) {
            auto wrapper = Scene::gen();
            wrapper->push(SceneEffect::DropShadow, 255, 255, 0, 255, 0.0f, 0.0f, _S(30), 30);
            auto shape = Shape::gen();
            shape->appendCircle(_S(-20), -offset, _S(10), _S(70));
            shape->appendCircle(_S(20), -offset, _S(10), _S(70));
            shape->fill(255, 255, 170);
            wrapper->push(shape);
            missles.push_back({wrapper, });
            model->push(wrapper);
        }
    }

    void update(const Point& pos, const Point& direction, float dir, uint32_t elapsed, const Point& shift, bool shoot)
    {
        clipper->translate(shift.x, shift.y);

        if (shoot && elapsed - lastshot > fireRate) lastshot = elapsed;
        else shoot = false;

        for (auto& fire : missles) {
            if (shoot && !fire.active) {
                fire.to = direction;
                extend(fire.to, FIREDISTANCE);
                fire.to += pos;
                fire.from = pos;
                fire.time = elapsed;
                fire.active = true;
                fire.model->opacity(255);
                fire.model->rotate(dir);
                shoot = false;
                ++actives;
            }
            if (fire.active) {
                auto progress = float(elapsed - fire.time) / FIRESPEED;
                if (progress <= 1.0f) {
                    fire.cur = {lerp(fire.from.x, fire.to.x, progress), lerp(fire.from.y, fire.to.y, progress)};
                    fire.model->translate(fire.cur.x, fire.cur.y);
                } else {
                    fire.inactivate();
                    --actives;
                }
            }
        }
    }
};

struct Player
{
    Launcher launcher;
    Scene* model;
    Point pos, ray, direction;
    float dir = 0.0f;
    float speed = 0.7f;
    float bound;
    bool shoot = false;

    void init(Canvas* canvas, const Point& pos, Shape* clipper)
    {
        bound = _S(40.0f);

        launcher.init(canvas, bound * 3, clipper);

        static const PathCommand cmds[] = {
            PathCommand::MoveTo,PathCommand::LineTo, PathCommand::LineTo, PathCommand::LineTo, PathCommand::LineTo,
            PathCommand::LineTo, PathCommand::LineTo, PathCommand::LineTo, PathCommand::LineTo, PathCommand::LineTo,
            PathCommand::Close
        };

        static const Point pts[] = {
            {0, -15}, {7, 0}, {25, -7}, {40, -30}, {30, 10}, {0, 30}, {-30, 10}, {-40, -30}, {-25, -7}, {-7, 0}
        };

        auto light = Shape::gen();
        light->appendCircle(0, 0, 95, 95);
        light->fill(255, 255, 255, 17);

        auto shape = Shape::gen();
        shape->appendPath(cmds, 11, pts, 10);
        shape->fill(255, 255, 255, 127);
        shape->strokeWidth(8.0f);
        shape->strokeFill(200, 200, 255);

        model = Scene::gen();
        model->push(light);
        model->push(shape);

        model->translate(pos.x, pos.y);
        model->scale(SCALE);
        canvas->push(model);

        this->pos = pos;
    }

    void forward(WarZone& zone, float multiplier)
    {
        auto radian = dir / 180.0f * M_PI;
        Point move = {_S(sinf(radian)), _S(cosf(radian))};
        normalize(move);
        extend(move, _S(multiplier * 0.4f));
        pos.x += move.x;
        pos.y -= move.y;

        //boundary limit
        auto& shift = zone.bound;
        if (pos.x - bound < _S(shift.x)) {
            pos.x = bound + _S(shift.x);
        } else if (pos.x + bound > _S(WIDTH - shift.x)) {
            pos.x = _S(WIDTH - shift.x) - bound;
        }
        if (pos.y - bound < _S(shift.y)) {
            pos.y = bound + _S(shift.y);
        } else if (pos.y + bound > _S(HEIGHT - shift.y)) {
            pos.y = _S(HEIGHT - shift.y) - bound;
        }

        zone.shift(pos);
    }

    void left(float multiplier)
    {
        dir -= _S(speed) * multiplier;
    }

    void right(float multiplier)
    {
        dir += _S(speed) * multiplier;
    }

    void update(uint32_t elapsed, const Point& shift)
    {
        auto radian = dir / 180.0f * M_PI;
        direction = {_S(sinf(radian)), -_S(cosf(radian))};
        normalize(direction);

        launcher.update(pos, direction, dir, elapsed, shift, shoot);
        model->push(SceneEffect::ClearAll);
        model->push(SceneEffect::DropShadow, 200, 200, 255, 255, dir + 180.0f, _S(20.0f), _S(30), 30);
        model->rotate(dir);
        model->translate(pos.x, pos.y);
    }
};

struct Enemy
{
    #define ENEMY_DURATION_LEVEL 1000

    static constexpr const size_t MAX_ROTATION = 20;
    static constexpr const size_t NUM_ENEMY_TYPE = 4;
    static constexpr const uint32_t BASETIME = 9200;
    static uint32_t DURATION;
    static float BOUND;

    Shape* model;
    float scale = 1.0f;
    int type;

    struct {
        Point from, to, cur;
    } pos;

    struct {
        float from, to;
    } dir;

    Tween time;

    Enemy(int type) : model(Shape::gen()), type(type)
    {
        model->ref();
        model->strokeWidth(8.0f);
        model->scale(SCALE);
        model->blend(BlendMethod::Add);
    }

    virtual ~Enemy()
    {
        model->unref();
    }

    void init(Scene* elayer, const Point shift, uint32_t elapsed)
    {
        if (type == 0 || type == 4) {         //top -> bottom
            pos = {{float(rand() % SWIDTH), -BOUND * 2}, {float(rand() % SWIDTH), SHEIGHT + BOUND * 2}};
        } else if (type == 1) {               //right -> left
            pos = {{float(SWIDTH + BOUND), float(rand() % SHEIGHT)}, {-BOUND, float(rand() % SHEIGHT)}};
        } else if (type == 2) {               //bottom -> top
            pos = {{float(rand() % SWIDTH), SHEIGHT + BOUND * 2}, {float(rand() % SWIDTH), -BOUND * 2}};
        } else if (type == 3) {               //left -> right
            pos = {{-BOUND, float(rand() % SHEIGHT)}, {float(SWIDTH + BOUND), float(rand() % SHEIGHT)}};
        }

        time = {elapsed, float(BASETIME + (rand() % DURATION))};
        dir = {float(rand() % 360), float(rand() % (360 * MAX_ROTATION))};
        model->rotate(dir.from);
        model->translate(pos.from.x, pos.from.y);
        elayer->push(model);
    }

    virtual int update(uint32_t elapsed, Launcher& launcher, const Point& p2o, Point& target)
    {
        auto progress = float(elapsed - time.at) / time.duration;
        if (progress > 1.0f) return 1;

        pos.cur = {lerp(pos.from.x, pos.to.x, progress), lerp(pos.from.y, pos.to.y, progress)};
        target = pos.cur + p2o;

        auto range = pow(BOUND + BOUND, 2);

        if (launcher.actives > 0) {
            for (auto& fire : launcher.missles) {
                if (fire.hit(target, range)) {
                    --launcher.actives;
                    return 2;
                }
            }
        }

        model->translate(pos.cur.x, pos.cur.y);
        model->rotate(lerp(dir.from, dir.to, progress));

        return 0;
    }

    virtual Color color() = 0;
};

uint32_t Enemy::DURATION = 9500 - (LEVEL * ENEMY_DURATION_LEVEL);
float Enemy::BOUND;

struct Boxer : Enemy
{
    static int type;

    Boxer(Scene* elayer, const Point& bound, uint32_t elapsed) : Enemy(Boxer::type)
    {
        model->appendRect(-40, -40, 80, 80);
        model->fill(50, 0, 0);
        model->strokeFill(255, 50, 50);

        init(elayer, bound, elapsed);
    }

    Color color() override { return {255, 50, 50}; }
};

struct Tripod : Enemy
{
    static int type;

    Tripod(Scene* elayer, const Point& bound, uint32_t elapsed) : Enemy(Tripod::type)
    {
        model->moveTo(0, -40);
        model->lineTo(40, 40);
        model->lineTo(-40, 40);
        model->close();
        model->strokeFill(170, 255, 170);
        model->fill(0, 50, 0);

        init(elayer, bound, elapsed);
    }

    Color color() override { return {170, 255, 170}; }
};

struct Sander : Enemy
{
    static int type;

    Sander(Scene* elayer, const Point& bound, uint32_t elapsed) : Enemy(Sander::type)
    {
        static const PathCommand cmds[] = {
            PathCommand::MoveTo,
            PathCommand::LineTo,
            PathCommand::LineTo,
            PathCommand::LineTo,
            PathCommand::LineTo,
            PathCommand::LineTo,
            PathCommand::Close
        };

        static const Point pts[] = {{0, -8}, {40, -40}, {40, 40}, {0, 8}, {-40, 40}, {-40, -40}};

        model->appendPath(cmds, 7, pts, 6);
        model->strokeFill(255, 120, 255);
        model->fill(50, 35, 50);

        init(elayer, bound, elapsed);
    }

    Color color() override { return {255, 120, 255}; }
};

struct Hexen : Enemy
{
    static int type;

    Hexen(Scene* elayer, const Point& bound, uint32_t elapsed) : Enemy(Hexen::type)
    {        
        static const PathCommand cmds[] = {
            PathCommand::MoveTo,
            PathCommand::LineTo,
            PathCommand::LineTo,
            PathCommand::LineTo,
            PathCommand::LineTo,
            PathCommand::LineTo,
            PathCommand::Close
        };

        static const Point pts[] = {{0, -40}, {40, -20}, {40, 20}, {0, 40}, {-40, 20}, {-40, -20}};

        model->appendPath(cmds, 7, pts, 6);
        model->strokeFill(0, 255, 255);
        model->fill(0, 50, 50);

        init(elayer, bound, elapsed);
    }

    Color color() override { return {0, 255, 255}; }
};

int Boxer::type = 0;
int Tripod::type = 1;
int Sander::type = 2;
int Hexen::type = 3;

struct Explosion
{
    #define PARTICLE_TIME 1000
    #define PARTICLE_NUM 6
    #define MAX_ROTATION 10
    #define PARTICLE_DIST 250
    #define PARTICLE_EXTRA 80

    Scene* model;
    uint32_t begin;   //begin tick
    Point pos;
    bool destroy = false;

    struct {
        Shape* shape;
        Point to;
        struct {
            float from, to;
        } dir;
    } particle[PARTICLE_NUM];

    struct {
        Shape* shape;
        Point to;
    } flashes[PARTICLE_EXTRA];

    ~Explosion()
    {
        model->unref();
    }

    Explosion() : model(Scene::gen())
    {
        model->ref();

        //destroy particle
        for (int i = 0; i < PARTICLE_NUM; ++i) {
            particle[i].shape = Shape::gen();
            particle[i].shape->appendRect(0, 0, _S(8.0f), _S(60.0f));
            model->push(particle[i].shape);
        }

        //flash particle
        for (int i = 0; i < PARTICLE_EXTRA; ++i) {
            flashes[i].shape = Shape::gen();
            flashes[i].shape->blend(BlendMethod::Add);
            model->push(flashes[i].shape);
        }
    }

    void init(const Point& pos, uint32_t elapsed)
    {
        auto w1 = _S(14.0f);
        for (int i = 0; i < PARTICLE_EXTRA / 2; ++i) {
            auto length = _S(rand() % 40 + 40);
            flashes[i].shape->reset();
            flashes[i].shape->appendRect(-w1, -length, w1 * 2.0f, length * 2, w1, length);
            auto dir = rand() % 360;
            flashes[i].shape->rotate(dir);
            auto to = length * 25.0f;
            auto rad = dir / 180.0f * M_PI;
            flashes[i].to.x = -to * sin(rad) + pos.x;
            flashes[i].to.y = +to * cos(rad) + pos.y;
        }

        auto w2 = _S(1.5f);
        for (int i = PARTICLE_EXTRA / 2; i < PARTICLE_EXTRA; ++i) {
            auto length = _S(rand() % 40 + 40);
            flashes[i].shape->reset();
            flashes[i].shape->appendRect(-w2, -length, w2 * 2.0f, length * 2);
            auto dir = rand() % 360;
            flashes[i].shape->rotate(dir);
            auto to = length * 30.0f;
            auto rad = dir / 180.0f * M_PI;
            flashes[i].to.x = -to * sin(rad) + pos.x;
            flashes[i].to.y = +to * cos(rad) + pos.y;
        }

        this->destroy = false;
        this->pos = pos;
        this->begin = elapsed;
    }

    void init(const Point& pos, Point dir, const Color& color, uint32_t elapsed)
    {
        extend(dir, _S(PARTICLE_DIST));

        for (int i = 0; i < PARTICLE_NUM; ++i) {
            particle[i].shape->fill(color.r, color.g, color.b);
            particle[i].to.x = _S(rand() % 1000) - _S(500) + pos.x + dir.x;
            particle[i].to.y = _S(rand() % 1000) - _S(500) + pos.y + dir.y;
            particle[i].dir.from = rand() % 360;
            particle[i].dir.to = rand() % MAX_ROTATION;
        }
        init(pos, elapsed);
        this->destroy = true;
    }

    bool update(uint32_t elapsed)
    {
        auto progress = float(elapsed - begin) / PARTICLE_TIME;
        if (progress > 1.0f) {
            for (int i = 0; i < PARTICLE_NUM; ++i) {
                particle[i].shape->opacity(0);
            }
            return true;
        }

        if (destroy) {
            auto c = 255 - 255 * progress;
            for (int i = 0; i < PARTICLE_NUM; ++i) {
                particle[i].shape->translate(lerp(pos.x, particle[i].to.x, progress), lerp(pos.y, particle[i].to.y, progress));
                particle[i].shape->rotate(lerp(particle[i].dir.from, particle[i].dir.to, progress));
                particle[i].shape->scale(1.0f - 0.25f * progress);
                particle[i].shape->opacity(c);
            }
        }

        auto scale = 1.0f - (0.75f * progress);
        auto sc = uint8_t(200.0f * cos(progress));

        for (int i = 0; i < PARTICLE_EXTRA / 2; ++i) {
            flashes[i].shape->translate(lerp(pos.x, flashes[i].to.x, progress), lerp(pos.y, flashes[i].to.y, progress));
            flashes[i].shape->fill(rand() % 255, rand() % 255, rand() % 255, sc);
            flashes[i].shape->scale(scale);
        }

        auto col = std::max(int(255 - 255 * progress * 2.0f), 0);

        for (int i = PARTICLE_EXTRA / 2; i < PARTICLE_EXTRA; ++i) {
            flashes[i].shape->translate(lerp(pos.x, flashes[i].to.x, progress), lerp(pos.y, flashes[i].to.y, progress));
            flashes[i].shape->fill(255, 255, col, col);
            flashes[i].shape->scale(scale);
        }

        return false;
    }
};

struct GarbageCollector
{
    Scene* elayer;
    vector<Enemy*> enemies[4];
    vector<Explosion*> explosions;

    GarbageCollector()
    {
        for (int i = 0; i < 4; ++i) {
            enemies[i].reserve(200);
        }

        explosions.reserve(300);
        for (int i = 0; i < 300; ++i) {
            explosions.push_back(new Explosion);
        }
    }

    ~GarbageCollector()
    {
        for (int i = 0; i < 4; ++i) {
            for (auto& e : enemies[i]) {
                delete(e);
            }
        }
        for (auto& e : explosions) {
            delete(e);
        }
    }

    template<class T>
    T* get(const Point& bound, uint32_t elapsed)
    {
        if (enemies[T::type].empty()) return new T(elayer, bound, elapsed);
        auto ret = enemies[T::type].back();
        enemies[T::type].pop_back();
        ret->init(elayer, bound, elapsed);
        return static_cast<T*>(ret);
    }

    void retrieve(Enemy* e)
    {
        enemies[e->type].push_back(e);
    }

    Explosion* get()
    {
        if (explosions.empty()) return new Explosion;
        auto ret = explosions.back();
        explosions.pop_back();
        return ret;
    }

    void retrieve(Explosion* e)
    {
        explosions.push_back(e);
    }
};

struct ComboMgr
{
    #define COMBO_TIME 750.0f

    struct Combo
    {
        ComboMgr* mgr;
        Text* text;
        Text* combo;
        float time;

        Combo(ComboMgr* mgr) : mgr(mgr), text(Text::gen())
        {
            text->ref();
            text->font(FONT_NAME);
            text->size(_S(50));
            text->fill(170, 255, 80);
        }

        ~Combo()
        {
            text->unref();
        }

        void init(const Point& pos, int counter, uint32_t elapsed)
        {
            char buf[20];
            snprintf(buf, sizeof(buf), "%dx combo!", counter);
            text->text(buf);
            text->translate(pos.x, pos.y);
            mgr->layer->push(text);
            time = (float)elapsed;
        }

        bool update(uint32_t elapsed)
        {
            auto progress = float(elapsed - time) / COMBO_TIME;
            if (progress <= 1.0f) {
                text->opacity(255 - (255 * progress));
                text->scale(1.0f + 0.2f * progress);
                return false;
            }
            return true;
        }
    };

    Scene* layer;
    list<Combo*> combos;
    vector<Combo*> recycle;
    int type = -1;
    int counter = 0;

    ~ComboMgr()
    {
        for (auto& combo : combos) {
            delete(combo);
        }
        for (auto& combo : recycle) {
            delete(combo);
        }
    }

    void init(Canvas* canvas)
    {
        layer = Scene::gen();
        canvas->push(layer);
        recycle.reserve(10);
    }

    int trigger(int type, const Point& pos, uint32_t elapsed)
    {
        if (this->type == type) {
            ++counter;
            Combo* combo;
            if (recycle.empty()) {
                combo = new Combo(this);
            } else {
                combo = recycle.back();
                recycle.pop_back();
            }
            combo->init(pos, counter, elapsed);            
            combos.push_back(combo);
        } else {
            this->type = type;
            counter = 1;
        }
        return counter;
    }

    void update(uint32_t elapsed)
    {
        for (auto it = combos.begin(); it != combos.end(); ) {
            auto combo = *it;
            if (combo->update(elapsed)) {
                layer->remove(combo->text);
                recycle.push_back(combo);
                it = combos.erase(it);
            } else {
                ++it;
            }
        }
    }
};

struct ThorWar : tvgdemo::Demo
{
    #define LIFE_CNT 3
    #define RESPAWN_LEVEL 100

    Player player;
    GarbageCollector gc;
    WarZone zone;
    list<Enemy*> enemies;
    list<Explosion*> explosions;
    Scene* elayer;
    Shape* clipper;
    ComboMgr combo;
    
    struct {
        uint32_t respawn = 0;      //elapsed time for respawn enemy
        uint32_t last = 0;         //elapsed time for frame update
        uint32_t end = 0;          //elapsed time for dead effect
    } tick;
    
    struct {
        size_t count = LIFE_CNT;
        Scene* icon[LIFE_CNT];
        Shape* flash;
        uint32_t last = 0;
        bool active = false;
    } lives;

    struct {
        Text *fps, *wipes, *lv;
    } gui;

    Point origin = {_S(WIDTH/2), _S(HEIGHT/2)};
    size_t respawnTime = 1000 - (LEVEL * RESPAWN_LEVEL);
    size_t wipesCnt = LEVEL * 100;
    bool gameplay = true;

    ~ThorWar()
    {
        for (auto e : enemies) {
            delete(e);
        }
        for (auto e : explosions) {
            delete(e);
        }

        lives.flash->unref();
        for (int i = 0; i < LIFE_CNT; ++i) {
            lives.icon[i]->unref();
        }
    }

    bool content(Canvas* canvas, uint32_t w, uint32_t h) override
    {
        Enemy::BOUND = _S(80.0f);

        zone.init(canvas);

        clipper = tvg::Shape::gen();
        clipper->appendRect(zone.min.x, zone.min.y, zone.w() + 10, zone.h() + 10);
        clipper->scale(SCALE);

        player.init(canvas, {float(w) * 0.5f, float(h) * 0.5f}, (Shape*)clipper->duplicate());

        gc.elayer = elayer = Scene::gen();
        elayer->clip(clipper);
        canvas->push(elayer);

        combo.init(canvas);

        //lives
        lives.flash = Shape::gen();
        lives.flash->ref();
        lives.flash->appendRect(0, 0, SWIDTH, SHEIGHT);
        lives.flash->fill(255, 255, 170);
        lives.flash->opacity(0);

        //life icon
        Point size = {_S(150), _S(150)};
        lives.icon[0] = Scene::gen();
        lives.icon[0]->ref();
        lives.icon[0]->push(SceneEffect::DropShadow, 170, 255, 80, 255, 0.0f, 0.0f, _S(15), 30);
        auto pic = Picture::gen();
        pic->load(LIFE_ICON, strlen(LIFE_ICON), "svg");
        pic->size(size.x, size.y);
        lives.icon[0]->translate(0, SHEIGHT - size.y);
        lives.icon[0]->push(pic);
        canvas->push(lives.icon[0]);

        for (int i = 1; i < LIFE_CNT; ++i) {
            lives.icon[i] = static_cast<Scene*>(lives.icon[0]->duplicate());
            lives.icon[i]->ref();
            lives.icon[i]->translate(size.x * i, SHEIGHT - size.y);            
            canvas->push(lives.icon[i]);
        }

        //gui texts - fps
        Text::load(FONT_NAME, (const char*)FONT_DATA, sizeof(FONT_DATA));
        gui.fps = tvg::Text::gen();
        gui.fps->font(FONT_NAME);
        gui.fps->size(25);
        gui.fps->translate(10, 10);
        gui.fps->fill(170, 255, 80);
        gui.fps->scale(SCALE);
        canvas->push(gui.fps);

        //gui texts - wipes
        auto wrapper = tvg::Scene::gen();
        wrapper->push(SceneEffect::DropShadow, 170, 255, 80, 255, 0.0f, 0.0f, _S(20), 30);
        gui.wipes = tvg::Text::gen();
        gui.wipes->font(FONT_NAME);
        gui.wipes->size(50);
        gui.wipes->text("0 Wipes");
        gui.wipes->fill(170, 255, 80);
        gui.wipes->translate(SWIDTH/2, 10);
        gui.wipes->align(0.5f, 0.0f);
        gui.wipes->scale(SCALE);
        wrapper->push(gui.wipes);
        canvas->push(wrapper);

        //gui texts - level
        gui.lv = tvg::Text::gen();
        gui.lv->font(FONT_NAME);
        gui.lv->size(40);
        gui.lv->fill(170, 255, 80);
        gui.lv->translate(SWIDTH -_S(20), _S(20));
        gui.lv->align(1.0f, 0.0f);
        gui.lv->scale(SCALE);
        char buf[30];
        snprintf(buf, sizeof(buf), "Level %ld", LEVEL + 1);
        gui.lv->text(buf);
        canvas->push(gui.lv);

        return true;
    }

    void updateGUI(bool updateFPS)
    {
        //update wipes count
        char buf[13];
        snprintf(buf, sizeof(buf), "%ld Wipes", wipesCnt);
        gui.wipes->text(buf);
        // update fps after a certan elapsed time, 
        // otherwise it's difficult to read if text is changed every frame.
        if (updateFPS) {
            snprintf(buf, sizeof(buf), "FPS: %d", tvgdemo::Demo::fps);
            gui.fps->text(buf);
        }
    }

    void destroy(Enemy* e, const Point& direction, uint32_t elapsed)
    {
        auto exp = gc.get();
        exp->init(e->pos.cur, direction, e->color(), elapsed);
        explosions.push_back(exp);
        elayer->push(exp->model);
    }

    void destroy(const Point& pos, uint32_t elapsed)
    {
        auto exp = gc.get();
        exp->init(pos, elapsed);
        explosions.push_back(exp);
        elayer->push(exp->model);
    }

    void input(Canvas* canvas, uint32_t elapsed)
    {
        player.shoot = false;

        const Uint8 *keystate = SDL_GetKeyboardState(NULL);
        if (keystate) {
            auto diff = elapsed - tick.last;
            if (keystate[SDL_SCANCODE_A]) player.shoot = true;
            if (keystate[SDL_SCANCODE_RIGHT]) player.right(diff);
            if (keystate[SDL_SCANCODE_LEFT]) player.left(diff);
            if (keystate[SDL_SCANCODE_UP]) player.forward(zone, diff);
        }
    }

    void gamelevel()
    {
        if (LEVEL < 9 && wipesCnt / 100 > LEVEL) {
            char buf[30];
            snprintf(buf, sizeof(buf), "Level %ld", ++LEVEL + 1);
            gui.lv->text(buf);
            respawnTime -= RESPAWN_LEVEL;
            Enemy::DURATION -= ENEMY_DURATION_LEVEL;
        }
    }

    void dead(Canvas* canvas, uint32_t elapsed)
    {
        gameplay = false;
        for (auto e : enemies) {
            destroy(e, player.direction, elapsed);
            elayer->remove(e->model);
            gc.retrieve(e);
        }
        enemies.clear();

        if (lives.count > 0 && elapsed - lives.last > 1000) {
            --lives.count;
            canvas->remove(lives.icon[lives.count]);
            lives.last = elapsed;
            lives.active = true;
            canvas->push(lives.flash);
        }

        for (auto& fire : player.launcher.missles) {
            player.launcher.actives = 0;
            fire.inactivate();
        }

        player.model->visible(false);
        tick.end = elapsed;
    }

    void reset(Canvas* canvas, uint32_t elapsed)
    {
        if (elapsed - tick.end < (lives.count == 0 ? 3000 : 1000)) return;

        //all life exhausted. total reset
        if (lives.count == 0) {
            fprintf(stdout, "Good Job!, Your Wipes: %ld\n", wipesCnt);

            LEVEL = 0;
            wipesCnt = 0;
            respawnTime = 1000;
            Enemy::DURATION = 10000;
            player.pos = {float(SWIDTH)/2, float(SHEIGHT)/2};
            zone.shift(player.pos);

            lives.count = LIFE_CNT;
            for (int i = 0; i < LIFE_CNT; i++) {
                canvas->push(lives.icon[i]);
            }

            char buf[30];
            snprintf(buf, sizeof(buf), "Level %ld", LEVEL + 1);
            gui.lv->text(buf);
        }

        player.model->visible(true);
        gameplay = true;
        tick.end = elapsed;
        combo.type = -1;
    }

    bool update(Canvas* canvas, uint32_t elapsed) override
    {
        auto shift = origin - (player.pos - origin);

        if (gameplay) {
            input(canvas, elapsed);
            player.update(elapsed, shift);
            zone.update(shift);
            clipper->translate(shift.x, shift.y);
        } else {
            //player dead flash effect
            if (lives.active) {
                auto progress = float(elapsed - lives.last) / 50;
                if (progress > 1.0f) {
                    canvas->remove(lives.flash);
                    lives.active = false;
                } else {
                    lives.flash->opacity((int)(255 * sin(3.14f * progress)));
                }
            }
            reset(canvas, elapsed);
        }

        auto p2o = origin - player.pos;
        elayer->translate(p2o.x, p2o.y);

        //enemies
        if (gameplay) {
            Point target;
            auto range = pow(player.bound + Enemy::BOUND, 2);
            for (auto it = enemies.begin(); it != enemies.end(); ) {
                auto e = *it;
                //collide with the player
                if (intersect(player.pos, e->pos.cur + p2o, range)) {
                    dead(canvas, elapsed);
                    break;
                //update enemies
                } else if (auto ret = e->update(elapsed, player.launcher, p2o, target)) {
                    if (ret == 2) {  //hit by missle
                        wipesCnt += combo.trigger(e->type, target, elapsed);
                        destroy(e, player.direction, elapsed);
                        gamelevel();
                    }
                    elayer->remove(e->model);
                    gc.retrieve(e);
                    it = enemies.erase(it);
                } else {
                    ++it;
                }
            }
        }

        //hit walls
        for (auto& fire : player.launcher.missles) {
            if (!fire.active) continue;
            if (fire.cur.x < _S(zone.min.x) + shift.x || fire.cur.x > _S(zone.max.x) + shift.x ||
                fire.cur.y < _S(zone.min.y) + shift.y || fire.cur.y > _S(zone.max.y) + shift.y) {
                destroy(fire.cur - p2o, elapsed);
                fire.inactivate();
            }
        }

        //explosions
        for (auto it = explosions.begin(); it != explosions.end(); ) {
            auto e = *it;
            if (e->update(elapsed)) {
                gc.retrieve(e);
                elayer->remove(e->model);
                it = explosions.erase(it);
                continue;
            }
            ++it;
        }

        combo.update(elapsed);

        updateGUI(respawn(elapsed));

        canvas->update();

        tick.last = elapsed;

        return true;
    }

    bool respawn(uint32_t elapsed)
    {
        if (!gameplay || elapsed - tick.respawn < respawnTime) return false;
        tick.respawn = elapsed;
        Point bound = {_S(800), _S(500)};

        //random enemy respawn
        if (rand() % 2) enemies.push_back(gc.get<Boxer>(bound, elapsed));
        if (rand() % 2) enemies.push_back(gc.get<Tripod>(bound, elapsed));
        if (rand() % 2) enemies.push_back(gc.get<Sander>(bound, elapsed));
        if (rand() % 2) enemies.push_back(gc.get<Hexen>(bound, elapsed));

        return true;
    }
};

int main(int argc, char** argv)
{
    SCALE = 0.5333333333333f;  //must be.
    SWIDTH = WIDTH * SCALE;
    SHEIGHT = HEIGHT * SCALE;

    return tvgdemo::main(new ThorWar, argc, argv, false, SWIDTH, SHEIGHT, 4);
}