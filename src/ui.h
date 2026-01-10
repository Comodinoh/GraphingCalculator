#ifndef GRAFIC_MENU_H
#define GRAFIC_MENU_H

#include "SFML/Graphics.hpp"

struct Button {
    sf::Vector2f size;
    sf::Vector2f pos;
    sf::Color color;
    sf::Texture texture;
    
    bool is_pressed(sf::Vector2f coords) {
        return coords.x > pos.x && coords.y > pos.y && coords.x < size.x && coords.y < size.y;
    }

    void draw(sf::RenderWindow *window) {
        sf::RectangleShape shape(size);
        shape.setPosition(pos);
        shape.setOutlineColor(sf::Color::Red);
        shape.setOutlineThickness(5);
        shape.setFillColor(color);
        shape.setTexture(&texture);
        window->draw(shape);
    }
    
};

struct ButtonList {
    Button *buttons;
    size_t len;
    size_t capacity;

    void append(Button *button) {
        if(len >= capacity) {
            capacity = capacity != 0 ? capacity * 2 : 16;

            buttons = (Button*)realloc(buttons, sizeof(Button)*capacity);
        }

        buttons[len] = *button;
        len++;
    }

    void remove_head() {
        if(buttons != NULL && len != 0) {
            len--;
        }
    }
    
};

#endif
