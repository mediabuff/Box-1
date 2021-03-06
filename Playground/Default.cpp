#include <Stick/FileUtilities.hpp>
#include <Box/BoxLua.hpp>
#include <Box/MouseEvents.hpp>
#include <Box/Render/GLRenderer.hpp>
#include <GLFW/glfw3.h>
#include <GLAnnotate/GLAnnotate.hpp>
#include <Crunch/Colors.hpp>
#include <Crunch/Randomizer.hpp>

#include <Luanatic/Luanatic.hpp>

#include <Stick/SystemClock.hpp>

using namespace stick;
using namespace box;
using namespace brick;
using namespace crunch;

// class TestInterface : public DocumentInterface
// {
// public:

//     void markDocumentForRendering()
//     {
//         printf("MARK DOCUMENT FOR RENDERING\n");
//     }

//     void markNodeForRendering(Entity _e)
//     {
//         printf("MARK NODE FOR RENDERING\n");
//     }
// };

namespace mycomps
{
    using BackgroundColor = brick::Component<ComponentName("BackgroundColor"), crunch::ColorRGBA>;
}

brick::Entity root;
lua_State * state;

inline void daFunc(brick::Entity * _ptr, const crunch::ColorRGBA & _col)
{
    _ptr->set<mycomps::BackgroundColor>(_col);
}

inline void daFunc(brick::Entity _vt, const crunch::ColorRGBA & _col)
{
    _vt.set<mycomps::BackgroundColor>(_col);
}

static void recursivelyDrawDocument(Entity _e, gla::GLAnnotate & _renderer)
{
    auto mc = _e.maybe<mycomps::BackgroundColor>();
    if (mc)
    {
        _renderer.setColor((*mc).r, (*mc).g, (*mc).b, (*mc).a);
        auto & cl = _e.get<comps::ComputedLayout>();
        _renderer.rect(cl.box.min().x, cl.box.min().y, cl.box.width(), cl.box.height());
    }
    auto mchildren = _e.maybe<comps::Children>();
    if (mchildren)
    {
        for (Entity & _e : (*mchildren))
        {
            recursivelyDrawDocument(_e, _renderer);
        }
    }
}

static void mouseButtonCallback(GLFWwindow * window, int button, int action, int mods)
{
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    if (action == GLFW_PRESS)
    {
        root.get<comps::EventHandler>()->publish(MouseDownEvent(MouseState(xpos, ypos, (UInt32)MouseButton::Left), MouseButton::Left), true);
    }
    else if (action == GLFW_RELEASE)
    {
        root.get<comps::EventHandler>()->publish(MouseUpEvent(MouseState(xpos, ypos, (UInt32)MouseButton::Left), MouseButton::Left), true);
    }
}

static void mouseMoveCallback(GLFWwindow * window, double xpos, double ypos)
{
    root.get<comps::EventHandler>()->publish(MouseMoveEvent(MouseState(xpos, ypos, 0)), true);
}

int main(int _argc, const char * _args[])
{
    stick::SystemClock clock;
    auto start = clock.now();

    Entity e = defaultHub().createEntity();
    for (int i = 0; i < 1000; ++i)
    {
        daFunc(&e, crunch::ColorRGBA(i, 1.0, i / 2.0, 1.0));
    }

    printf("IT TOOK %f MS\n", (clock.now() - start).milliseconds());

    start = clock.now();
    for (int i = 0; i < 1000; ++i)
    {
        daFunc(e, crunch::ColorRGBA(i, 1.0, i / 2.0, 1.0));
    }
    printf("IT TOOK %f MS\n", (clock.now() - start).milliseconds());

    // initialize glfw
    if (!glfwInit())
        return EXIT_FAILURE;

    // and set some hints to get the correct opengl versions/profiles
    glfwWindowHint(GLFW_SAMPLES, 8);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // state = luanatic::createLuaState();
    // luanatic::initialize(state);
    // luanatic::openStandardLibraries(state);
    // luanatic::addPackagePath(state, "../../Box/BML/?.lua");
    // registerBox(state, &defaultHub());

    // auto error = luanatic::execute(state, loadTextFile("../../Playground/BoxMLLexer.lua").ensure());
    // if (error)
    // {
    //     printf("%s\n", error.message().cString());
    //     return EXIT_FAILURE;
    // }

    // if(luanatic::HasValueTypeConverter<box::Value>::value)
    //     printf("GOT DA CONVERTER!\n");

    // return EXIT_SUCCESS;

    //create the window
    GLFWwindow * window = glfwCreateWindow(800, 600, "Hello Paper Example", NULL, NULL);
    if (window)
    {
        glfwSetMouseButtonCallback(window, mouseButtonCallback);
        glfwSetCursorPosCallback(window, mouseMoveCallback);

        glfwMakeContextCurrent(window);

        gla::GLAnnotate gla;

        bool res = gla.init();
        if (!res)
        {
            printf("%s\n", gla.error());
            return EXIT_FAILURE;
        }

        root = createDocument();
        render::GLRenderer renderer;

        auto err = renderer.initialize(root);
        if(err)
        {
            printf("%s\n", err.message().cString());
            return EXIT_FAILURE;
        }

        setSize(root, 800.0f, 600.0f);
        setPadding(root, 50.0f);

        root.set <comps::Background>(crunch::ColorRGBA(0.5f, 0.3f, 0.1f, 1.0f));
        auto a = createNode(root, "DAAA A");
        setHeight(a, 200.0f);
        a.set <comps::Background>(crunch::ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f));
        addChild(root, a);
        auto b = createNode(root, "DAAA FUUUUUCK B");
        setSize(b, 600.0f, 100.0f);
        // setMaxWidth(b, 500.0f);
        b.setAndApply<comps::MaxWidth>(box::markDirty, 500.0f);
        setMaxHeight(b, 200.0f);
        b.set <comps::Background>(crunch::ColorRGBA(0.3f, 0.3f, 0.3f, 1.0f));
        addChild(a, b);

        auto c = createNode(root, "DAAA C");
        setWidth(c, 200.0f);
        c.set<comps::Justify>(Justify::End);
        c.set <comps::Background>(crunch::ColorRGBA(0.9f, 0.3f, 0.3f, 1.0f));
        addChild(root, c);

        auto e = createNode(root, "E");
        setSize(e, 100.0f, 20.0f);
        e.set <comps::Background>(crunch::ColorRGBA(0.9f, 0.3f, 0.9f, 1.0f));
        addChild(c, e);

        auto f = createNode(root, "F");
        setMargin(f, 20);
        setSize(f, 50.0f, 50.0f);
        f.set <comps::Background>(crunch::ColorRGBA(0.9f, 0.9f, 0.3f, 1.0f));
        addChild(c, f);

        auto g = createNode(root, "G");
        setSize(g, 80.0f, 20.0f);
        g.set <comps::Background>(crunch::ColorRGBA(0.9f, 0.1f, 0.9f, 1.0f));
        addChild(c, g);

        auto h = createNode(root, "H");
        setSize(h, 50.0f, 50.0f);
        h.set <comps::Background>(crunch::ColorRGBA(0.9f, 0.1f, 0.4f, 1.0f));
        addChild(c, h);

        auto i = createNode(root, "I");
        setSize(i, 30.0f, 60.0f);
        i.set <comps::Background>(crunch::ColorRGBA(0.1f, 0.1f, 1.0f, 1.0f));
        addChild(c, i);
        setMargin(i, 10);
        setMarginTop(i, -10);

        addEventCallback(i, [](const MouseDownEvent & _e, brick::Entity _self)
        {
            printf("CLICKED BABY\n");
            _self.set<comps::Background>(crunch::ColorRGBA(0.9f, 0.9f, 0.0f, 1.0f));
        });

        addEventCallback(i, [](const MouseUpEvent & _e, brick::Entity _self)
        {
            printf("RELEASE BABY\n");
            _self.set<comps::Background>(crunch::ColorRGBA(0.9f, 0.1f, 0.0f, 1.0f));
        });

        addEventCallback(i, [](const MouseMoveEvent & _e, brick::Entity _self)
        {
            // printf("MOUSE MOVE ON I BABY!!!!!!!!!!!!\n");
        });

        addEventCallback(i, [](const MouseEnterEvent & _e, brick::Entity _self)
        {
            printf("MOUSE ENTER!!!!!!!!!!!!\n");
            _self.set<comps::Background>(crunch::ColorRGBA(0.9f, 0.1f, 0.0f, 1.0f));
            setWidth(_self, 90.0f);
        });

        addEventCallback(i, [](const MouseLeaveEvent & _e, brick::Entity _self)
        {
            printf("MOUSE LEAVE!!!!!!!!!!!!\n");
            _self.set<comps::Background>(crunch::ColorRGBA(0.1f, 0.1f, 1.0f, 1.0f));
            setWidth(_self, 30.0f);
        });

        // the main loop
        float angle = 0;
        Randomizer rnd;
        while (!glfwWindowShouldClose(window))
        {
            // printf("FRAME\n");

            // setWidth(root, rnd.randomf(400, 800));
            layout(root, 800, 600);
            // if(i.get<comps::Dirty>())
            //     printf("I AM DIRTY\n");
            // printf("POST LAYOUT\n");

            // clear the background to black
            glClearColor(0, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT);

            // gla.ortho(0, 800, 600, 0, -1, 1);
            // recursivelyDrawDocument(root, gla);

            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            glViewport(0, 0, width, height);
            renderer.render();

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }
    else
    {
        glfwTerminate();
        printf("Could not open GLFW window :(\n");
        return EXIT_FAILURE;
    }

    // clean up glfw
    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
