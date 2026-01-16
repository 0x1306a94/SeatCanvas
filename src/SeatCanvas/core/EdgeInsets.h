//
//  EdgeInsets.h
//  SeatCanvas
//
//  Created by king on 2025/11/12.
//

#ifndef EdgeInsets_h
#define EdgeInsets_h

namespace kk {
struct EdgeInsets {
    float top;
    float left;
    float bottom;
    float right;

    EdgeInsets()
        : EdgeInsets(0.0f, 0.0f, 0.0f, 0.0f) {
    }

    EdgeInsets(float top, float left, float bottom, float right)
        : top(top)
        , left(left)
        , bottom(bottom)
        , right(right) {
    }

    // 统一设置所有边距
    explicit EdgeInsets(float all)
        : EdgeInsets(all, all, all, all) {
    }

    // 比较运算符
    bool operator==(const EdgeInsets &other) const {
        return top == other.top &&
            left == other.left &&
            bottom == other.bottom &&
            right == other.right;
    }

    bool operator!=(const EdgeInsets &other) const {
        return !(*this == other);
    }

    // 便利方法
    float horizontal() const {
        return left + right;
    }

    float vertical() const {
        return top + bottom;
    }

    bool isEmpty() const {
        return top == 0.0f && left == 0.0f &&
            bottom == 0.0f && right == 0.0f;
    }
};

}  // namespace kk

#endif /* EdgeInsets_h */
