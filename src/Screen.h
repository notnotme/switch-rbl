#pragma once

template <class Context>
class Screen
{

public:
    Screen();
    virtual ~Screen();

    virtual void enter(Context& context) = 0;
    virtual void exit(Context& context) = 0;
    virtual bool render(Context& context, float dt) = 0;

private:
    Screen(const Screen& copy);

};

template <class DemoContext>
Screen<DemoContext>::Screen()
{
}

template <class DemoContext>
Screen<DemoContext>::~Screen()
{
}
