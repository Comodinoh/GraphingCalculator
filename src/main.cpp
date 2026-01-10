#include <SFML/Graphics.hpp>

#include "expressions.h"
#include "ui.h"

using Vector2d = sf::Vector2<double>;

using namespace std;
const int INALTIME_FEREASTRA = 600;
const int LATIME_FEREASTRA = INALTIME_FEREASTRA*(16.0f/9.0f);


struct StringBuffer {
    char* buf = NULL;
    size_t len = 0;
    size_t capacity = 0;

    void append(const char* str) {
        size_t str_len = strlen(str);

        if(len + str_len >= capacity) {
            if(capacity != 0) {
                while(len + str_len >= capacity) {
                    capacity *= 2;
                }
            }else {
                capacity = str_len+1;
            }
            
            buf = (char*)realloc(buf, sizeof(char)*capacity);
            
        }
        strncpy(&buf[len], str, str_len+1);
        len += str_len;
    }

    void append_char(char c) {
        if(len >= capacity) {
            if(capacity != 0) {
                capacity *= 2;
            }else {
                capacity = 256;
            }
            
            buf = (char*)realloc(buf, sizeof(char)*capacity);
        }

        buf[len] = c;
        len++;
    }

    void append_char_and_check_null_term(char c) {
        append_char(c);
        check_null_term();
    }

    void remove_head() {
        if(len != 0) {
            buf[len-1] = '\0';
            len--;
        }
        
    }

    void remove_at(size_t pos, size_t amount) {
        if(len == 0 ) return;

        size_t final_pos = pos + amount - 1;
        size_t amount_to_move = len-final_pos+1; // #0 1 2# 3 4

        for(size_t i = 0; i < amount_to_move; i++;) {
            
        }
        
        
        
        
    }
    
    
    void check_null_term() {
        if(len != 0 && buf[len] != '\0') buf[len] = '\0';
    }
    
};


int main()
{

    Tokenizer tokenizer;
    ParseTree parser;

    sf::ContextSettings settings;

    settings.antiAliasingLevel = sf::RenderTexture::getMaximumAntiAliasingLevel();
    sf::RenderWindow fereastra(sf::VideoMode({LATIME_FEREASTRA, INALTIME_FEREASTRA}), "Sistem de Coordonate XOY", sf::Style::Default, sf::State::Windowed, settings);

    fereastra.setVerticalSyncEnabled(true);
    sf::View camera(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(20.0f, 15.0f));

    bool tragDeEcran = false;
    sf::Vector2i pozitieVecheMouse(0, 0);

    Button button = {{200, 50}, {0, 0}, {100, 100, 100, 200}};

    StringBuffer buffer;
    sf::Font font;

    if(!font.openFromFile("MinecraftDefault-Regular.ttf")) {
        return 1;
    }

    font.setSmooth(true);
    
    sf::Text text(font);

    sf::View default_view = fereastra.getDefaultView();

    //sf::VertexBuffer buffer(sf::PrimitiveType::LineStrip, sf::VertexBuffer::Usage::Static);

    //size_t previousSize = 0;

    while (fereastra.isOpen())
    {
        while (std::optional event = fereastra.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                fereastra.close();


            if (sf::Event::MouseWheelScrolled* scroll = event->getIf<sf::Event::MouseWheelScrolled>())
            {
                if (scroll->delta > 0)
                {
                    camera.zoom(0.9f);
                }
                else
                {
                    camera.zoom(1.1f);
                }
            }


            if (sf::Event::MouseButtonPressed* pressed = event->getIf<sf::Event::MouseButtonPressed>())
            {
                sf::Vector2f mpos = fereastra.mapPixelToCoords(sf::Mouse::getPosition(fereastra), default_view);
                if(!button.is_pressed({mpos.x, mpos.y})) {
                    pozitieVecheMouse = sf::Mouse::getPosition(fereastra);
                    tragDeEcran = true;
                }
                
            }
            if (event->is<sf::Event::MouseButtonReleased>())
            {
                sf::Vector2f mpos = fereastra.mapPixelToCoords(pozitieVecheMouse, default_view);
                if(!tragDeEcran && button.is_pressed(mpos)) {
                    button.color = sf::Color::Green;
                }
                
                if(tragDeEcran) {
                    tragDeEcran = false;
                }
                

            }
            if (event->is<sf::Event::MouseMoved>() && tragDeEcran)
            {
                sf::Vector2i pozitieNouaMouse = sf::Mouse::getPosition(fereastra);

                sf::Vector2f delta = fereastra.mapPixelToCoords(pozitieVecheMouse, camera) - fereastra.mapPixelToCoords(pozitieNouaMouse, camera);
                
                camera.move(delta);
                                     
                pozitieVecheMouse = pozitieNouaMouse;
            }


            if (sf::Event::Resized* resized = event->getIf<sf::Event::Resized>())
            {
                float aspect = (float)resized->size.x / resized->size.y;
                camera.setSize({camera.getSize().y * aspect, camera.getSize().y});
                
                default_view.setSize({resized->size.x, resized->size.y});
                default_view.setCenter({resized->size.x/2, resized->size.y/2});
                
                //text.setPosition({default_view.getCenter().x - default_view.getSize().x/2, default_view.getCenter().y - default_view.getSize().y/2});
            }

            if(sf::Event::TextEntered* text = event->getIf<sf::Event::TextEntered>()) {
                char c = (char)text->unicode;
                if(c == '\r') {
                    if(tokenizer.expr != nullptr) tokenizer.destroy();
                    tokenizer.tokenize(buffer.buf);
                    parser.init(&tokenizer);
                    parser.parse();
                } else if(c == '\b') {
                    buffer.remove_head();
                } else{
                    buffer.append_char(c);
                    printf("%c\n", c);
                }
            }
            
        }
        

        fereastra.clear(sf::Color(20, 20, 30));
        fereastra.setView(default_view);

        //printf("%f %f\n", camera.getCenter().x, camera.getSize().x);
        
        buffer.check_null_term();
        text.setString(buffer.buf);

        
        button.draw(&fereastra);
        fereastra.draw(text);
        
        fereastra.setView(camera);

        sf::Vector2f centru = camera.getCenter();
        sf::Vector2f marime = camera.getSize();
        float stanga = centru.x - marime.x / 2;
        float dreapta = centru.x + marime.x / 2;
        float sus = centru.y - marime.y / 2;
        float jos = centru.y + marime.y / 2;

        sf::Vertex axaX[] =
        {
            sf::Vertex(sf::Vector2f(stanga, 0), sf::Color::White),
            sf::Vertex(sf::Vector2f(dreapta, 0), sf::Color::White)
        };
        sf::Vertex axaY[] =
        {
            sf::Vertex(sf::Vector2f(0, sus), sf::Color::White),
            sf::Vertex(sf::Vector2f(0, jos), sf::Color::White)
        };

        fereastra.draw(axaX, 2, sf::PrimitiveType::Lines);
        fereastra.draw(axaY, 2, sf::PrimitiveType::Lines);

        double distance = marime.x;

        //printf("Drawing at distance: %f\n", distance);

        if(parser.tokenizer != NULL) {

            size_t vertices = max((uint32_t)2, fereastra.getSize().x);
            sf::VertexArray array = sf::VertexArray{sf::PrimitiveType::LineStrip, vertices};
            //sf::Vertex* va = (sf::Vertex*)malloc(sizeof(sf::Vertex)*vertices);

            double dx = (marime.x) / (double)(vertices-1);

            for(size_t i = 0; i < vertices; i++) {
                double x = i*dx + stanga;
                array[i].position = sf::Vector2f(x, -parser.execute(x, parser.head));
            }


            fereastra.draw(array);

        }

        
        fereastra.display();
    }
    return 0;
}
