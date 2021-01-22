#pragma once

#include <unordered_map>
#include <typeindex>

#include "observer_ptr.h"

namespace tofu {
    // シンプルなサービスロケータ
    class ServiceLocator 
    {
    public:
        // 所有権を移譲してサービスを登録する
        template<class T>
        observer_ptr<T> Register(std::unique_ptr<T>&& ptr)
        {
            _container[get_id<T>()] = std::move(ptr);
            return Get<T>();
        }

        // サービスを取得
        template<class T>
        observer_ptr<T> Get() const
        {
            return reinterpret_cast<T*>(_container.at(get_id<T>()).get());
        }

        void clear() {
            _container.clear();
        }

    private:
        template<class T>
        std::type_index get_id() const
        {
            return std::type_index{ typeid(std::decay_t<T>) };
        }

        std::unordered_map<std::type_index, std::shared_ptr<void>> _container;
    };

}

