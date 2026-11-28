#include <ostream>
#include <smol/forward_list.hpp>

#include <iostream>
#include <charconv>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <variant>

namespace {

struct UserInput {
    std::string name;
    std::string argument;
};

struct ParseError {
    std::string message;
};

// Validated commands
struct Help {};
struct List {};
struct Quit {};
struct Add  { std::string text; };
struct Done { std::size_t number; };
struct Del  { std::size_t number; };

using ParseResult = std::variant<List, Add, Done, Del, Quit, Help, ParseError>;

constexpr std::string_view help_text =
    ".list           Show TODOs and their current numbers, starting at 1.\n"
    ".add [text]     Add a TODO. Enter more lines if needed, then type . on its own line to save.\n"
    ".done <number>  Mark the numbered TODO as done.\n"
    ".del <number>   Delete the numbered TODO.\n"
    ".help           Show this command list.\n"
    ".quit           Exit the app.\n";

std::string_view trim(std::string_view text) {
    constexpr std::string_view whitespace = " \t\r\n\f\v";

    const auto first = text.find_first_not_of(whitespace);
    if (first == std::string_view::npos) {
        return {};
    }

    const auto last = text.find_last_not_of(whitespace);
    return text.substr(first, last - first + 1);
}


std::optional<std::size_t> parse_item_number(std::string_view text) {
    if (text.empty()) {
        return std::nullopt;
    }

    std::size_t number{};
    const char* begin = text.data();
    const char* end = begin + text.size();
    const auto [next, error] = std::from_chars(begin, end, number);

    if (error != std::errc{} || next != end || number == 0) {
        return std::nullopt;
    }
    return number;
}

ParseResult parse_command(UserInput command) {
    const auto& name = command.name;
    const std::string_view argument = trim(command.argument);

    if (name == ".list" || name == ".quit" || name == ".help") {
        if (!argument.empty()) {
            return ParseError{name + " takes no arguments"};
        }
        if (name == ".quit") {
            return Quit{};
        }
        if (name == ".help") {
            return Help{};
        }
        return List{};
    }

    if (name == ".add") {
        if (argument.empty()) {
            return ParseError{".add requires some TODO text"};
        }
        return Add{std::move(command.argument)};
    }

    if (name == ".done" || name == ".del") {
        const auto number = parse_item_number(argument);
        if (!number) {
            return ParseError{name + " requires a positive decimal item number"};
        }
        if (name == ".done") {
            return Done{*number};
        }
        return Del{*number};
    }

    if (name.empty()) {
        return ParseError{"expected a command"};
    }
    return ParseError{"unknown command '" + name + "'"};
}


std::optional<UserInput> read_user_input(std::istream& in, std::ostream& out) {
    out << "> " << std::flush;

    std::string line;
    if (!std::getline(in, line)) {
        return std::nullopt;
    }

    // Split once so spaces inside the argument are preserved.
    const auto separator = line.find_first_of(" \t");
    UserInput command{line.substr(0, separator), {}};

    bool has_text_line = false;
    if (separator != std::string::npos) {
        command.argument = line.substr(separator + 1);
        has_text_line = !command.argument.empty();
    }

    if (command.name != ".add") {
        return command;
    }

    while (true) {
        out << "... " << std::flush;

        if (!std::getline(in, line)) {
            return std::nullopt;  // Discard an unfinished TODO on EOF.
        }
        if (line == ".") {
            return command;
        }

        if (has_text_line) {
            command.argument += '\n';
        }
        command.argument += line;
        has_text_line = true;
    }
}

} // namespace

struct Todo {
    std::string text;
    bool done = false;
};

class Todos {
private:
    smol::forward_list<Todo> todos_;
    smol::forward_list<Todo>::const_iterator last_;

public:
    Todos() :last_(todos_.before_begin()) {}

    Todos(const Todos&) = delete;
    Todos& operator=(const Todos&) = delete;

    void add_todo(std::string text) {
        last_ = todos_.insert_after(last_, Todo{std::move(text)});
    }

    bool mark_done(std::size_t number) {
        std::size_t position = 1;
        for (Todo& todo : todos_) {
            if (position == number) {
                todo.done = true;
                return true;
            }
            ++position;
        }
        return false;
    }

    bool delete_todo(std::size_t number) {
        auto previous = todos_.before_begin();
        std::size_t position = 1;

        for (auto current = todos_.begin(); current != todos_.end(); ++current) {
            if (position == number) {
                auto successor = todos_.erase_after(previous);
                if (successor == todos_.end()) {
                    last_ = previous;
                }
                return true;
            }

            previous = current;
            ++position;
        }
        return false;
    }

    friend std::ostream& operator<<(std::ostream& out, const Todos& todos) {
        if (todos.todos_.is_empty()) {
            return out << "No TODOs.\n";
        }

        std::size_t number = 1;
        for (const Todo& todo : todos.todos_) {
            const std::string prefix = "[" + std::to_string(number++)
                         + (todo.done ? "] [x] " : "] [ ] ");
            const std::string indent(prefix.size(), ' ');

            out << prefix;
            for (char ch : todo.text) {
                out.put(ch);
                if (ch == '\n') {
                    out << indent;
                }
            }
            out.put('\n');
        }
        return out;
    }
};

int main() {
    Todos todos;

    while (auto raw = read_user_input(std::cin, std::cout)) {
        auto result = parse_command(std::move(*raw));

        if (const auto* error = std::get_if<ParseError>(&result)) {
            std::cout << error->message << "\nType .help for usage.\n";
        } else if (std::holds_alternative<Help>(result)) {
            std::cout << help_text;
        } else if (std::holds_alternative<Quit>(result)) {
            break;
        } else if (std::holds_alternative<List>(result)) {
            std::cout << todos;
        } else if (auto* add = std::get_if<Add>(&result)) {
            todos.add_todo(std::move(add->text));
        } else if (const auto* done = std::get_if<Done>(&result)) {
            if (!todos.mark_done(done->number)) {
                std::cout << "No TODO numbered " << done->number << ".\n";
            }
        } else if (const auto* del = std::get_if<Del>(&result)) {
            if (!todos.delete_todo(del->number)) {
                std::cout << "No TODO numbered " << del->number << ".\n";
            }
        }
    }

    return 0;
}
