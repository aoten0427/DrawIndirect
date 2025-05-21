// ================================================================ 
// ファイル名 : Singleton.h
// 作成者 : 景山碧天
// 説明 :  シングルトン基底クラス
// ================================================================

#pragma once


template <typename T>
class Singleton {
public:
    // シングルトンインスタンスの取得
    static T* GetInstance() {
        if (instance == nullptr)
        {
            instance.reset(new T());
        }
        return instance.get();
    }

    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
protected:
    Singleton() = default;
    virtual ~Singleton() = default;

private:
    //自身のインスタンス
    static std::unique_ptr<T> instance;
};

template <typename T>
std::unique_ptr<T> Singleton<T>::instance = nullptr;