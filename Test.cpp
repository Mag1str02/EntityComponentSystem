#include <iostream>
#include <memory>
#include "Entity.hpp"

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

void Print(const std::vector<Entity>& v) { std::cout << v.size() << std::endl; }

int main()
{
    ECS::Storage storage;
    Entity       e1 = storage.CreateEntity();
    Entity       e2 = storage.CreateEntity();
    Entity       e3 = storage.CreateEntity();

    e1.Add<Vec2>();
    e1.Add<Vec3>();
    e2.Add<Vec3>();
    e2.Add<Vec4>();
    e3.Add<Vec4>();
    e3.Add<Vec2>();
    e3.Add<int>();

    Print(storage.View<>());
    Print(storage.View<int>());
    Print(storage.View<Vec2>());
    Print(storage.View<Vec3>());
    Print(storage.View<Vec4>());
    Print(storage.View<Vec2, Vec3>());
    Print(storage.View<Vec3, Vec4>());
    Print(storage.View<Vec4, Vec2>());
    Print(storage.View<Vec2, Vec4>());

    printf("Succeeded!\n");

    return 0;
}