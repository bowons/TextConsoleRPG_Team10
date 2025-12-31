#pragma once

template <typename T>
class Singleton
{
protected:
    static T* _Instance;

    Singleton() {};
    virtual ~Singleton() {};

public:
    static T* GetInstance()
    {
        if (_Instance == nullptr)
        {
            _Instance = new T();
        }
        return _Instance;
    }
};

template <typename T>
T* Singleton<T>::_Instance = nullptr;