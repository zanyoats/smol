// struct Node below is defined as a string | list<Node>. This allows us to build
// trees of strings instead of just flat lists. Makes for a less mundane example.
//
// Implementations match the spirit of the little schemer's occurs, rember, subst.
// Each rely on recursion and extra copying to make implementations more clear
// and simple. More efficient implementations would consider structural sharing.

#include <smol/persistent_list.hpp>

#include <iomanip>
#include <iostream>
#include <string_view>
#include <variant>
#include <string>
#include <string_view>
#include <iomanip>
#include <ostream>

struct Node {
    std::variant<std::string, smol::persistent_list<Node>> data;

    friend std::ostream& operator<<(std::ostream& out, const Node& node) {
        if (const auto* text = std::get_if<std::string>(&node.data)) {
            out << std::quoted(*text);
        } else {
            out << std::get<smol::persistent_list<Node>>(node.data);
        }
        return out;
    }
};

// occurs: count occurrences of y in xs

std::size_t occurs(std::string_view y, const smol::persistent_list<Node>& xs) {
    if (xs.is_empty()) return 0;

    const auto& head = xs.head();
    std::size_t head_count;

    if (auto* text = std::get_if<std::string>(&head.data)) {
        head_count = (*text == y) ? 1 : 0;
    } else {
        const auto& ys = std::get<smol::persistent_list<Node>>(head.data);
        head_count = occurs(y, ys);
    }
    return head_count + occurs(y, xs.tail());
}

// member: use occurs to see if y is a member in xs

inline bool member(std::string_view y, const smol::persistent_list<Node>& xs) {
    return occurs(y, xs) > 0;
}

// rember: remove member y from xs

smol::persistent_list<Node> rember(std::string_view y, const smol::persistent_list<Node>& xs) {
    if (xs.is_empty()) return smol::persistent_list<Node>(xs);

    const auto& head = xs.head();
    const auto tail = xs.tail();
    auto result_tail = rember(y, tail);

    if (const auto* text = std::get_if<std::string>(&head.data)) {
        if (*text == y) return result_tail;
        return head + result_tail;
    }

    const auto& children = std::get<smol::persistent_list<Node>>(head.data);
    return Node{rember(y, children)} + result_tail;
}

// subst: substitute all y with z in xs

smol::persistent_list<Node> subst(std::string_view y, std::string_view z, const smol::persistent_list<Node>& xs) {
    if (xs.is_empty() || y == z) return smol::persistent_list<Node>(xs);

    const auto& head = xs.head();
    const auto tail = xs.tail();
    auto result_tail = subst(y, z, tail);

    if (const auto* text = std::get_if<std::string>(&head.data)) {
        if (*text == y) return Node{std::string{z}} + result_tail;
        return head + result_tail;
    }

    const auto& children = std::get<smol::persistent_list<Node>>(head.data);
    return Node{subst(y, z, children)} + result_tail;
}

int main() {
    auto drink = Node{smol::persistent_list<Node>{
        Node{"milk"},
        Node{"cacao"},
    }};
    auto main = Node{smol::persistent_list<Node>{
        Node{"tofu"},
        Node{"broccoli"},
        Node{"carrots"},
        Node{"peas"},
    }};
    auto snack = Node{smol::persistent_list<Node>{
        Node{"blueberries"},
        Node{"strawberries"},
        Node{"cacao"},
    }};

    auto meal1 = smol::persistent_list<Node>{
        drink,
        main,
        snack,
    };

    const auto mea1_no_cacao = rember("cacao", meal1);

    auto meal2_lentils = subst("tofu", "lentils", meal1);

    std::cout << "Meal 1:\n" << meal1;
    std::cout << "\n\nMeal 1 (without \"cacao\"):\n" << mea1_no_cacao;
    std::cout << "\n\nMeal 2 (subst \"tofu\" with \"lentils\"):\n" << meal2_lentils << '\n';
}
