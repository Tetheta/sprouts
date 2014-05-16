#include "platform.h"
#include "guibutton.h"
#include "guilabel.h"
#include "guicanvas.h"
#include "text.h"
#include <cwchar>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <random>

using namespace std;

int main()
{
    startGraphics();
    Renderer renderer;
    Image background(L"background.png");
    Display::initFrame();
    vector<function<void()>> needRunFunctions;
    shared_ptr<GUIContainer> gui = make_shared<GUIContainer>(-Display::scaleX(), Display::scaleX(),
                                   -Display::scaleY(), Display::scaleY());
    shared_ptr<GUILabel> lastButtonLabel = make_shared<GUILabel>(L"", -0.5, 0.5, -0.35, -0.05);
    gui->add(make_shared<GUIButton>([lastButtonLabel]()
    {
        lastButtonLabel->text = L"button 1 pressed √π ≤ ½·e²";
        lastButtonLabel->textColor = Color::RGB(1, 0, 0);
    }, L"Button 1", -0.4, 0.4, 0.9, 1));
    gui->add(make_shared<GUIButton>([lastButtonLabel]()
    {
        lastButtonLabel->text = L"π\n⌠\n│ sin(x) dx = 2\n⌡\n0";
        lastButtonLabel->textColor = Color::RGB(1, 1, 0);
    }, L"Button 1½", -0.4, 0.4, 0.8, 0.9));
    gui->add(make_shared<GUIButton>([lastButtonLabel]()
    {
        lastButtonLabel->text = L"button 2 pressed";
        lastButtonLabel->textColor = Color::RGB(1, 1, 0);
    }, L"Button 2", -0.4, 0.4, 0.7, 0.8));
    shared_ptr<GUILabel> fpsLabel = make_shared<GUILabel>(L"", -0.5, 0.5, -0.05, 0.05);
    gui->add(make_shared<GUIContainer>(-0.5, 0.5, -0.4, 0.1)->add(fpsLabel)->add(lastButtonLabel));
    gui->add(make_shared<GUIButton>([&needRunFunctions, &gui]()
    {
        needRunFunctions.push_back([&gui]()
        {
            Display::setSize(640, 480);
            Display::initFrame();
            gui->reset();
        });
    }, L"Set 640x480 graphics", -0.4, 0.4, 0.3, 0.4));
    gui->add(make_shared<GUIButton>([&needRunFunctions, &gui]()
    {
        needRunFunctions.push_back([&gui]()
        {
            Display::setSize(1024, 768);
            Display::initFrame();
            gui->reset();
        });
    }, L"Set 1024x768 graphics", -0.4, 0.4, -0.4, -0.3, Color::RGB(1, 0, 0), Color::V(0), Color::RGB(1,
            0, 1)));
    gui->add(make_shared<GUIButton>([&needRunFunctions, &gui]()
    {
        needRunFunctions.push_back([&gui]()
        {
            Display::fullScreen(!Display::fullScreen());
            Display::initFrame();
            gui->reset();
        });
    }, L"Toggle fullscreen", -0.4, 0.4, -0.6, -0.5, Color::RGB(1, 0, 0), Color::V(0), Color::RGB(1,
            0, 1)));
    gui->add(make_shared<GUIButton>([&needRunFunctions, &gui]()
    {
        needRunFunctions.push_back([&gui]()
        {
            endGraphics();
            exit(0);
        });
    }, L"Quit", -0.4, 0.4, -0.7, -0.6, Color::RGB(1, 0, 0), Color::V(0), Color::RGB(1,
            0, 1)));
    gui->add(make_shared<GUICanvas>(-1, -0.5, -0.25, 0.25, []() -> Mesh
    {
        wstring str = L"Hello";
        Mesh mesh = Text::mesh(str, Color::V(1));
        mesh->add(invert(mesh));
        float width = Text::width(str);
        float height = Text::height(str);
        mesh = (Mesh)transform(Matrix::translate(-width / 2, -height / 2, 0)
        .concat(Matrix::scale(2 / width)), mesh);
        Mesh retval = Mesh(new Mesh_t());
        minstd_rand0 randomEngine;
        uniform_real_distribution<double> r(0, 2 * M_PI);
        uniform_real_distribution<double> r2(0, 1);

        for(int i = 0; i < 5; i++)
        {
            retval->add(scaleColors(Color::HSV(r2(randomEngine), 1, 1), transform(Matrix::rotateX(r(randomEngine)).concat(Matrix::rotateY(r(
                randomEngine))).concat(Matrix::rotateZ(r(randomEngine))), mesh)));
        }
        return (Mesh)transform(Matrix::rotateY(Display::timer() * M_PI / 3).concat(Matrix::translate(0, 0, -1)), retval);
    }));

    while(true)
    {
        Display::initFrame();
        gui->minX = -Display::scaleX();
        gui->maxX = Display::scaleX();
        gui->minY = -Display::scaleY();
        gui->maxY = Display::scaleY();
        Display::handleEvents(gui);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        TextureDescriptor td = TextureDescriptor(background, 0, Display::width() / background.width(), 0,
                               Display::height() / background.height());
        Mesh backgroundmesh = Generate::quadrilateral(td, VectorF(-Display::scaleX(), -Display::scaleY(),
                              -1), Color(1), VectorF(Display::scaleX(), -Display::scaleY(), -1), Color(1),
                              VectorF(Display::scaleX(), Display::scaleY(), -1), Color(1), VectorF(-Display::scaleX(),
                                      Display::scaleY(), -1), Color(1));
        renderer << backgroundmesh;
        Display::initOverlay();
        wostringstream os;
        os << L"FPS : " << setprecision(4) << setw(5) << setfill(L'0') << left << Display::averageFPS();
        fpsLabel->text = os.str();
        renderer << gui->render();
        Display::flip(60);
        vector<function<void()>> fns = std::move(needRunFunctions);
        needRunFunctions.clear();

        for(function<void()> fn : fns)
        {
            fn();
        }
    }
}
