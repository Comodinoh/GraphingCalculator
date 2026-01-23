#include <SFML/Graphics.hpp>

#include <filesystem>
#include <unordered_set>

#include "expressions.h"
#include "ui.h"

using Vector2d = sf::Vector2<double>;

using namespace std;
const int INALTIME_FEREASTRA = 600;
const int LATIME_FEREASTRA = INALTIME_FEREASTRA*(16.0f/9.0f);


static bool VIEW_COORDS = false;

void deseneazaSageata(sf::RenderWindow& fereastra, sf::Vector2f varf, float rotatie, float marime)
{
    sf::ConvexShape sageata;
    sageata.setPointCount(3);
    sageata.setPoint(0, sf::Vector2f(0, 0));
    sageata.setPoint(1, sf::Vector2f(-marime, -marime / 3.0f));
    sageata.setPoint(2, sf::Vector2f(-marime, marime / 3.0f));

    sageata.setFillColor(lineColors[currentTheme]);
    sageata.setPosition(varf);
    sageata.setRotation(sf::degrees(rotatie));

    fereastra.draw(sageata);
}

static double clampd(double v, double lo, double hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

static int niceStep(double approx) {
    if (approx <= 1.0) return 1;
    double p = std::pow(10.0, std::floor(std::log10(approx)));
    double m = approx / p;
    if (m <= 1.0) return (int)(1.0 * p);
    if (m <= 2.0) return (int)(2.0 * p);
    if (m <= 5.0) return (int)(5.0 * p);
    return (int)(10.0 * p);
}

static int firstMultiple(int a, int step) {
    if (step <= 0) return a;
    int r = a % step;
    if (r == 0) return a;
    if (a >= 0) return a + (step - r);
    return a - r;
}

void drawTickX(sf::RenderWindow* w, double x, double tickLen) {
    
    sf::Vertex tick[] = {
        sf::Vertex(sf::Vector2f((float)x, (float)-tickLen), lineColors[currentTheme]),
        sf::Vertex(sf::Vector2f((float)x, (float)+tickLen), lineColors[currentTheme]),
    };
    w->draw(tick, 2, sf::PrimitiveType::Lines);
}

void drawTickY(sf::RenderWindow* w, double y, double tickLen) {
    sf::Vertex tick[] = {
        sf::Vertex(sf::Vector2f((float)-tickLen, (float)y), lineColors[currentTheme]),
        sf::Vertex(sf::Vector2f((float)+tickLen, (float)y), lineColors[currentTheme]),
    };
    w->draw(tick, 2, sf::PrimitiveType::Lines);
}

void deseneazaAsimptota(sf::RenderWindow* w, float x, float start, float end) {
    sf::Color color = { 255, 0, 0, 255 };
    sf::Vertex lines[6] = {
        {sf::Vector2f{x, start}, color},
        {sf::Vector2f{x, end}, color},
        {{x + 0.1f, start}, color},
        {{x + 0.1f, end }, color},
        {{x - 0.1f, start }, color},
        {{x - 0.1f, end }, color},
    };

    w->draw(lines, 6, sf::PrimitiveType::Lines);
}

void deseneazaGrafic(sf::RenderWindow* fereastra,
    ParseTree* parser,
    sf::VertexArray* array,
    double startX,
    double width,
    size_t vertices,
    sf::View* camera,
    sf::View* uiView,
    sf::Font* font,
    bool drawUnits)
{
    sf::Vector2f cf = camera->getCenter();
    sf::Vector2f cs = camera->getSize();

    double stanga = (double)cf.x - (double)cs.x / 2.0;
    double dreapta = (double)cf.x + (double)cs.x / 2.0;
    double sus = (double)cf.y - (double)cs.y / 2.0;
    double jos = (double)cf.y + (double)cs.y / 2.0;

    double dx = width / (double)(vertices - 1);

    double tickLen = (double)cs.y * 0.015;

    fereastra->setView(*camera);
    if (parser->tokenizer != NULL && parser->debug_buffer.len == 0) {

        for (size_t i = 0; i < vertices-1; i++) {
            double x1 = (double)i * dx + startX;
            double x2 = (double)(i + 1) * dx + startX;
            double y1 = -parser->execute(x1, parser->head);
            double y2 = -parser->execute(x2, parser->head);

            double threshold = camera->getSize().y * 1.0f;

            sf::Vector2f centru = camera->getCenter();
            sf::Vector2f marime = camera->getSize();
            float sus = centru.y - marime.y / 2;
            float jos = centru.y + marime.y / 2;

            if (!std::isfinite(y1) || !std::isfinite(y2)) {
                //deseneazaAsimptota(fereastra, (x1 + x2) / 2.0f, sus, jos);
                continue;
            }

            if (std::abs(y2 - y1) > threshold) {
               // deseneazaAsimptota(fereastra, (x1 + x2) / 2.0f, sus, jos);
                continue;
            }

            double big = camera->getSize().y * 5.0;
            if (std::abs(y1) > big || std::abs(y2) > big) {
               // deseneazaAsimptota(fereastra, (x1 + x2) / 2.0f, sus, jos);
                continue;
            }
            if (std::abs(y1) > big && std::abs(y2) > big && (y1 > 0) != (y2 > 0)) {
               // deseneazaAsimptota(fereastra, (x1 + x2) / 2.0f, sus, jos);
                continue;
            }

            array->append({ sf::Vector2f((float)x1, (float)y1),  lineColors[currentTheme] });
            array->append({ sf::Vector2f((float)x2, (float)y2),  lineColors[currentTheme] });
        }

        fereastra->draw(*array);
    }

    if (!drawUnits) return;

    double approx = (double)camera->getSize().x / 12.0;
    int step = niceStep(approx);

    int xStart = (int)std::ceil(std::max(stanga, startX));
    int xEnd = (int)std::floor(std::min(dreapta, startX + width));

    int yStart = (int)std::ceil(sus);
    int yEnd = (int)std::floor(jos);

    for (int xi = firstMultiple(xStart, step); xi <= xEnd; xi += step) {
        if (xi == 0) continue;
        if (std::abs(xi) < 1) continue;
        drawTickX(fereastra, (double)xi, tickLen);
    }

    for (int yi = firstMultiple(yStart, step); yi <= yEnd; yi += step) {
        if (yi == 0) continue;
        if (std::abs(yi) < 1) continue;
        drawTickY(fereastra, (double)yi, tickLen);
    }

    sf::Text t(*font);
    sf::Vector2u win = fereastra->getSize();
    unsigned int charSize = (unsigned int)std::clamp((int)std::llround(18.0 * ((double)win.y / 720.0)), 14, 30);
    t.setCharacterSize(charSize);
    t.setFillColor(textColors[currentTheme]);

    fereastra->setView(*uiView);

    for (int xi = firstMultiple(xStart, step); xi <= xEnd; xi += step) {
        if (xi == 0) continue;
        if (std::abs(xi) < 1) continue;

        sf::Vector2i px = fereastra->mapCoordsToPixel(sf::Vector2f((float)xi, 0.f), *camera);
        sf::Vector2f ui = fereastra->mapPixelToCoords(px, *uiView);

        t.setString(std::to_string(xi));
        sf::FloatRect b = t.getLocalBounds();
        t.setOrigin({ b.position.x + b.size.x / 2.f, 0.f });
        t.setPosition({ ui.x, ui.y + 8.f });
        fereastra->draw(t);
    }

    for (int yi = firstMultiple(yStart, step); yi <= yEnd; yi += step) {
        if (yi == 0) continue;
        if (std::abs(yi) < 1) continue;

        int labelVal = -yi;

        sf::Vector2i px = fereastra->mapCoordsToPixel(sf::Vector2f(0.f, (float)yi), *camera);
        sf::Vector2f ui = fereastra->mapPixelToCoords(px, *uiView);

        t.setString(std::to_string(labelVal));
        sf::FloatRect b = t.getLocalBounds();
        t.setOrigin({ 0.f, b.position.y + b.size.y / 2.f });
        t.setPosition({ ui.x + 12.f, ui.y });
        fereastra->draw(t);
    }

    fereastra->setView(*camera);
}

int main()
{

    Tokenizer tokenizer;
    ParseTree parser;

    sf::ContextSettings settings;

    settings.antiAliasingLevel = sf::RenderTexture::getMaximumAntiAliasingLevel();
    sf::RenderWindow fereastra(sf::VideoMode(sf::Vector2u(UI_BASE_SIZE)), "Sistem de Coordonate XOY", sf::Style::Default, sf::State::Windowed, settings);

    fereastra.setVerticalSyncEnabled(true);
    sf::Vector2f cameraStartPos = sf::Vector2f(0.0f, 0.0f);
    sf::Vector2f cameraStartSize = sf::Vector2f(10.0f, 5.0f);
    sf::View camera(cameraStartPos, cameraStartSize);

    bool tragDeEcran = false;

    sf::View default_view = fereastra.getDefaultView();

    Button resetButton = make_button("Reset", sf::Vector2f{ UI_BASE_SIZE.x-100, 10.0f},  
        {90, 30});
    Button themeButton = make_button("Theme", sf::Vector2f(UI_BASE_SIZE.x - 190, 10.0f),
        { 90, 30 });
    Button coordButton = make_button("XY", sf::Vector2f(UI_BASE_SIZE.x - 280, 10.0f),
        { 90, 30 });

    resetButton.setAnchor(UIAnchor::TopRight);
    themeButton.setAnchor(UIAnchor::TopRight, { 110.0f, 10.0f });
    coordButton.setAnchor(UIAnchor::TopRight, { 210.0f, 10.0f });

    StringBuffer buffer;
    StringBuffer coordBuffer;

    TextDisplay function_display = create_text_display(&buffer);
    TextDisplay debug_display = create_text_display(&parser.debug_buffer);
    TextDisplay coord_display = create_text_display(&coordBuffer);
    debug_display.text.setPosition({ 0, 70 });
    coord_display.text.setPosition({ 0, 100 });
    function_display.text.setCharacterSize(64);
    function_display.text.setStyle(sf::Text::Bold);




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
                if (resetButton.isHovered(mpos)) {
                    camera.setCenter(cameraStartPos);
                    camera.setSize(cameraStartSize);
                }
                else if (themeButton.isHovered(mpos)) {
                    if (currentTheme == themes - 1) {
                        currentTheme = 0;
                    }
                    else {
                        currentTheme++;
                    }
                }
                else if (coordButton.isHovered(mpos)) {
                    VIEW_COORDS = !VIEW_COORDS;
                }
                else {
                    pozitieVecheMouse = sf::Mouse::getPosition(fereastra);
                    tragDeEcran = true;
                }
                
            }
            if (event->is<sf::Event::MouseButtonReleased>())
            {
                
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
               

                //uiView.setViewport(sf::FloatRect({posX, posY}, {sizeX, sizeY}));

                default_view.setSize(sf::Vector2f{ resized->size });
                default_view.setCenter(sf::Vector2f{ resized->size } / 2.0f);
            }

            if(sf::Event::TextEntered* text = event->getIf<sf::Event::TextEntered>()) {
                char c = (char)text->unicode;
                if(c == '\r') {
                    parser.debug_buffer.reset();
                    if (buffer.len == 0) {
                        tokenizer.destroy();
                        parser.tokenizer = nullptr;
                    } else {
                        if (tokenizer.expr != nullptr) tokenizer.destroy();
                        tokenizer.tokenize(buffer.buf);
                        parser.init(&tokenizer);
                        parser.parse();
                    }
                } else if(c == '\b') {
                    buffer.remove_head();
                } else{
                    buffer.append_char(c);
                }
            }
            
        }
        

        fereastra.clear(backgroundColors[currentTheme]);
        
        
        fereastra.setView(camera);

        sf::Vector2f centru = camera.getCenter();
        sf::Vector2f marime = camera.getSize();
        float stanga = centru.x - marime.x / 2;
        float dreapta = centru.x + marime.x / 2;
        float sus = centru.y - marime.y / 2;
        float jos = centru.y + marime.y / 2;

        sf::Color lineColor = lineColors[currentTheme];

        sf::Vertex axaX[] =
        {
            
            sf::Vertex(sf::Vector2f(stanga, 0), lineColor),
            sf::Vertex(sf::Vector2f(dreapta, 0), lineColor)
        };
        sf::Vertex axaY[] =
        {
            sf::Vertex(sf::Vector2f(0, sus), lineColor),
            sf::Vertex(sf::Vector2f(0, jos), lineColor)
        };

        fereastra.draw(axaX, 2, sf::PrimitiveType::Lines);
        fereastra.draw(axaY, 2, sf::PrimitiveType::Lines);

        float marimeSageata = marime.x * 0.02f;
        deseneazaSageata(fereastra, sf::Vector2f(dreapta, 0), 0.0f, marimeSageata);
        deseneazaSageata(fereastra, sf::Vector2f(0, sus), -90.0f, marimeSageata);


        //printf("Drawing at distance: %f\n", distance);

        
            size_t vertices = std::clamp((int)fereastra.getSize().x * 2, 200, 8000);

            /*if (array.getVertexCount() < vertices) {
                array.resize(vertices);
            }*/
            sf::VertexArray array(sf::PrimitiveType::Lines);

            deseneazaGrafic(&fereastra, &parser, &array, stanga, marime.x, vertices, &camera, &default_view, &font, true);

            fereastra.setView(default_view);

            //printf("%f %f\n", camera.getCenter().x, camera.getSize().x);

            buffer.check_null_term();


            function_display.text.setFillColor(textColors[currentTheme]);
            debug_display.text.setFillColor(textColors[currentTheme]);

            resetButton.draw(&fereastra, &default_view);
            themeButton.draw(&fereastra, &default_view);
            coordButton.draw(&fereastra, &default_view);
            function_display.draw(&fereastra);
            debug_display.draw(&fereastra);


        if (VIEW_COORDS) {
            sf::Vector2f coords = fereastra.mapPixelToCoords(sf::Mouse::getPosition(fereastra), camera);
            coordBuffer.reset();

            coordBuffer.appendf("X=%f, Y=%f\n", coords.x, -coords.y);

            coord_display.text.setFillColor(textColors[currentTheme]);
            coord_display.draw(&fereastra);
        }
        
        fereastra.display();
    }
    return 0;
}
