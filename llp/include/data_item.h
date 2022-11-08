#ifndef LLP_DATA_ITEM_H
#define LLP_DATA_ITEM_H

typedef long long iid;

class data_item {
public:
    enum {
        INT32, DOUBLE, BOOL, STRING, NONE
    } type = NONE;
    iid id;
    void *data{};

    data_item() = default;

    explicit data_item(decltype(type) type, iid id, void *value = nullptr) : type(type), id(id), data(data) {};

};

#endif //LLP_DATA_ITEM_H
