#include <iostream>
#include <memory>
#include "ECS/ECS.h"

using namespace ECS;

struct Vec2
{
    double v[2];
};
struct Vec3
{
    double v[3];
};
struct Vec4
{
    double v[4];
};

double array[20];

class MySystem : public System
{
public:
    virtual std::string GetName() const override { return "MySystem"; }
    virtual void        Update(float dt) override
    {
        printf("My system: ");
        for (Entity e : View<std::string>())
        {
            std::cout << *e.Get<std::string>() << " ";
        }
        std::cout << std::endl;
    }
};

void Print(const std::vector<Entity>& v) { std::cout << v.size() << std::endl; }

template <typename... Components> void Print(Storage& storage)
{
    for (Entity d : storage.View<Components...>())
    {
        std::cout << *d.Get<std::string>() << " ";
    }
    std::cout << std::endl;
}

void Test1()
{
    Storage storage;
    Entity  e1 = storage.CreateEntity();
    Entity  e2 = storage.CreateEntity();
    Entity  e3 = storage.CreateEntity();
    Entity  e4 = storage.CreateEntity();

    e1.Add<std::string>("First");
    e1.Add<Vec2>();
    e1.Add<Vec3>();
    e2.Add<std::string>("Second");
    e2.Add<Vec3>();
    e2.Add<Vec4>();
    e3.Add<std::string>("Third");
    e3.Add<Vec4>();
    e3.Add<Vec2>();
    e3.Add<int>();
    e4.Add<std::string>("Fourth");
    e4.Add<Vec4>();
    e4.Add<Vec3>();
    e4.Add<Vec2>();
    e4.Add<int>();

    Print<double>(storage);
    Print<>(storage);
    Print<std::string>(storage);
    Print<std::string, Vec2>(storage);
    Print<std::string, Vec3>(storage);
    Print<std::string, Vec4>(storage);
    Print<std::string, Vec2, Vec4>(storage);
    Print<std::string, Vec2, Vec3, Vec4>(storage);
    Print<int>(storage);

    storage.RegisterSystem<MySystem>();
    storage.UpdateSystems(1);

    printf("Succeeded!\n");
}

int main()
{
    Test1();
    return 0;
}