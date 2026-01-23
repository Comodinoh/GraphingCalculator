#ifndef GRAFIC_MENU_H
#define GRAFIC_MENU_H

#include "SFML/Graphics.hpp"
#include <cstdarg>

static sf::Vector2i pozitieVecheMouse;

static sf::Color backgroundColors[4] = {
    {250, 250, 250},
    {0, 0, 0},
    {204, 85, 0},
    {20, 35, 49}
};

static sf::Color lineColors[4] = {
    {0, 0, 0},
    {250, 250, 250},
    {10, 30, 50},
    {186, 18, 0}
};

static sf::Color buttonColors[4] = {
    {200, 50, 50},
    {250, 250, 250},
    {10, 60, 80},
    {200, 224, 244}
};

static sf::Color buttonHoverColors[4] = {
    {120, 120, 120},
    {120, 120, 120},
    {10, 40, 60},
    {80, 138, 168}
};

static sf::Color textColors[4] = {
    {0, 0, 0},
    {200, 50, 50},
    {250, 250, 250},
    {0, 0, 0}
};

static size_t themes = 4;
static size_t currentTheme = 0;

struct StringBuffer {

    char* buf = NULL;
    size_t len = 0;
    size_t capacity = 0;

    void append(const char* str) {
        size_t str_len = strlen(str);

        if (len + str_len >= capacity) {
            if (capacity != 0) {
                while (len + str_len >= capacity) {
                    capacity *= 2;
                }
            }
            else {
                capacity = str_len + 1;
            }

            buf = (char*)realloc(buf, sizeof(char) * capacity);

        }
        strncpy(&buf[len], str, str_len + 1);
        len += str_len;
    }

    void appendf(const char* str, ...) {
        va_list args;
        va_list args2;

        va_start(args, strlen(str));
        va_copy(args2, args);

        size_t len = vsnprintf(nullptr, 0, str, args);

        char* fmtstr = (char*)malloc(sizeof(char) * len);
        vsnprintf(fmtstr, sizeof(char) * len, str, args2);

        va_end(args);
        va_end(args2);

        append(fmtstr);

        free(fmtstr);
    }

    void append_char(char c) {
        if (len >= capacity) {
            if (capacity != 0) {
                capacity *= 2;
            }
            else {
                capacity = 256;
            }

            buf = (char*)realloc(buf, sizeof(char) * capacity);
        }

        buf[len] = c;
        len++;
    }

    void append_char_and_check_null_term(char c) {
        append_char(c);
        check_null_term();
    }

    void remove_head() {
        if (len != 0) {
            buf[len - 1] = '\0';
            len--;
        }

    }

    void remove_at(size_t pos, size_t amount) {
        if (len == 0) return;

        size_t final_pos = pos + amount - 1;
        size_t amount_to_move = len - final_pos + 1; // #0 1 2# 3 4


    }


    void check_null_term() {
        if (len != 0 && buf[len] != '\0') buf[len] = '\0';
    }

    void reset() {
        if (len == 0) return;

        len = 0;

        buf[len] = '\0';
    }

};

static sf::Vector2f UI_BASE_SIZE = { 1366, 768 };

enum class UIAnchor {
    None,
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight
};

struct Button {
    sf::RectangleShape shape;
    sf::Text text;

    sf::Vector2f basePos{ 0.f, 0.f };
    sf::Vector2f baseSize{ 90.f, 30.f };

    UIAnchor anchor = UIAnchor::None;
    sf::Vector2f marginPx{ 10.f, 10.f };

    bool integerScale = true;

    bool isHovered(sf::Vector2f coords) const {
        return shape.getGlobalBounds().contains(coords);
    }

    void setAnchor(UIAnchor a, sf::Vector2f margin = { 10.f, 10.f }) {
        anchor = a;
        marginPx = margin;
    }

    void setBase(sf::Vector2f pos, sf::Vector2f size) {
        basePos = pos;
        baseSize = size;
    }

    void updateLayout(const sf::Vector2u& winSize) {
        float sx = (float)winSize.x / UI_BASE_SIZE.x;
        float sy = (float)winSize.y / UI_BASE_SIZE.y;
        float s = std::min(sx, sy);
        if (integerScale) s = std::max(1.f, std::floor(s));

        sf::Vector2f sizePx = baseSize * s;

        sf::Vector2f posPx = basePos * s;

        switch (anchor) {
        case UIAnchor::TopLeft:
            posPx = { marginPx.x, marginPx.y };
            break;
        case UIAnchor::TopRight:
            posPx = { (float)winSize.x - sizePx.x - marginPx.x, marginPx.y };
            break;
        case UIAnchor::BottomLeft:
            posPx = { marginPx.x, (float)winSize.y - sizePx.y - marginPx.y };
            break;
        case UIAnchor::BottomRight:
            posPx = { (float)winSize.x - sizePx.x - marginPx.x,
                      (float)winSize.y - sizePx.y - marginPx.y };
            break;
        case UIAnchor::None:
        default:
            break;
        }

        shape.setPosition(posPx);
        shape.setSize(sizePx);

        sf::FloatRect lb = text.getLocalBounds();
        text.setOrigin({ lb.position.x + lb.size.x / 2.f, lb.position.y + lb.size.y / 2.f });

        text.setScale({ s, s });
        text.setPosition(posPx + sizePx / 2.f);
    }


    void draw(sf::RenderWindow* window, sf::View* view) {
        updateLayout(window->getSize());

        if (isHovered(window->mapPixelToCoords(sf::Mouse::getPosition(*window), *view))) {
            shape.setFillColor(buttonHoverColors[currentTheme]);
        }
        else {
            shape.setFillColor(buttonColors[currentTheme]);
        }

        text.setFillColor(textColors[currentTheme]);

        
        window->draw(shape);
        window->draw(text);
    }
};

static sf::Font font("MinecraftDefault-regular.ttf");

inline Button make_button(const char* txt,
    sf::Vector2f pos,
    sf::Vector2f size)
{
    sf::RectangleShape shape(size);
    shape.setPosition(pos);
    sf::Text text(font, txt);

    return {shape, text, pos, size};
}

struct TextDisplay {
    StringBuffer *buffer;
    sf::Text text;

    void draw(sf::RenderWindow* window) {
        text.setString(buffer->buf);
        window->draw(text);
    }
};

TextDisplay create_text_display(StringBuffer* buffer) {
    return { buffer, sf::Text(font) };
}

#endif
