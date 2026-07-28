#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;

struct ParseState {
    bool in_block_comment = false;
    bool in_double_quote = false;
    bool in_single_quote = false;
    bool in_raw_string = false;
    std::string raw_delim;
};

enum class LineKind {
    NoComment,
    InlineComment,
    StandaloneComment
};

struct LineInfo {
    LineKind kind = LineKind::NoComment;
    std::size_t comment_pos = std::string::npos;
    bool excluded = false;
};

static bool is_space(unsigned char ch) {
    return std::isspace(ch) != 0;
}

static std::string ltrim_copy(std::string_view s) {
    std::size_t i = 0;
    while (i < s.size() && is_space(static_cast<unsigned char>(s[i]))) ++i;
    return std::string(s.substr(i));
}

static std::string rtrim_copy(std::string_view s) {
    std::size_t end = s.size();
    while (end > 0 && is_space(static_cast<unsigned char>(s[end - 1]))) --end;
    return std::string(s.substr(0, end));
}

static std::string trim_copy(std::string_view s) {
    return rtrim_copy(ltrim_copy(s));
}

static bool is_blank_line(std::string_view s) {
    for (unsigned char ch : s) {
        if (!is_space(ch)) return false;
    }
    return true;
}

static bool starts_with(std::string_view s, std::string_view prefix) {
    return s.substr(0, prefix.size()) == prefix;
}

static bool is_excluded_comment_text(std::string_view comment_text) {
    std::string t = ltrim_copy(comment_text);
    return starts_with(t, "START OF") || starts_with(t, "======");
}

static void capitalize_first_alpha(std::string& s) {
    for (char& ch : s) {
        unsigned char u = static_cast<unsigned char>(ch);
        if (std::isalpha(u)) {
            ch = static_cast<char>(std::toupper(u));
            break;
        }
        if (!is_space(u)) {
            break;
        }
    }
}

static bool try_parse_raw_string_start(const std::string& line, std::size_t pos,
    std::size_t& consume_len,
    std::string& delim) {
    std::size_t prefix_len = 0;
    if (line.compare(pos, 2, "u8") == 0) {
        prefix_len = 2;
    }
    else if (pos < line.size() && (line[pos] == 'u' || line[pos] == 'U' || line[pos] == 'L')) {
        prefix_len = 1;
    }

    if (pos + prefix_len + 1 >= line.size()) return false;
    if (line[pos + prefix_len] != 'R' || line[pos + prefix_len + 1] != '"') return false;

    std::size_t open_paren = line.find('(', pos + prefix_len + 2);
    if (open_paren == std::string::npos) return false;

    delim = line.substr(pos + prefix_len + 2, open_paren - (pos + prefix_len + 2));
    consume_len = open_paren - pos + 1;
    return true;
}

static LineInfo analyze_line(const std::string& line, ParseState& state) {
    std::size_t i = 0;
    while (i < line.size()) {
        if (state.in_block_comment) {
            if (i + 1 < line.size() && line[i] == '*' && line[i + 1] == '/') {
                state.in_block_comment = false;
                i += 2;
            }
            else {
                ++i;
            }
            continue;
        }

        if (state.in_raw_string) {
            if (line[i] == ')' &&
                i + 1 + state.raw_delim.size() < line.size() &&
                line.compare(i + 1, state.raw_delim.size(), state.raw_delim) == 0 &&
                line[i + 1 + state.raw_delim.size()] == '"') {
                std::size_t delim_len = state.raw_delim.size();
                state.in_raw_string = false;
                state.raw_delim.clear();
                i += delim_len + 2;
            }
            else {
                ++i;
            }
            continue;
        }

        if (state.in_double_quote) {
            if (line[i] == '\\' && i + 1 < line.size()) {
                i += 2;
            }
            else if (line[i] == '"') {
                state.in_double_quote = false;
                ++i;
            }
            else {
                ++i;
            }
            continue;
        }

        if (state.in_single_quote) {
            if (line[i] == '\\' && i + 1 < line.size()) {
                i += 2;
            }
            else if (line[i] == '\'') {
                state.in_single_quote = false;
                ++i;
            }
            else {
                ++i;
            }
            continue;
        }

        if (i + 1 < line.size() && line[i] == '/' && line[i + 1] == '/') {
            LineInfo info;
            info.comment_pos = i;
            std::string before = line.substr(0, i);
            info.kind = is_blank_line(before) ? LineKind::StandaloneComment : LineKind::InlineComment;
            info.excluded = is_excluded_comment_text(line.substr(i + 2));
            return info;
        }

        if (i + 1 < line.size() && line[i] == '/' && line[i + 1] == '*') {
            state.in_block_comment = true;
            i += 2;
            continue;
        }

        std::size_t consume_len = 0;
        std::string delim;
        if (try_parse_raw_string_start(line, i, consume_len, delim)) {
            state.in_raw_string = true;
            state.raw_delim = std::move(delim);
            i += consume_len;
            continue;
        }

        if (line[i] == '"') {
            state.in_double_quote = true;
            ++i;
            continue;
        }

        if (line[i] == '\'') {
            state.in_single_quote = true;
            ++i;
            continue;
        }

        ++i;
    }

    return {};
}

struct PhysicalLine {
    std::string text;
    std::string eol;
};

static std::vector<PhysicalLine> split_preserve_newlines(const std::string& content) {
    std::vector<PhysicalLine> lines;
    std::size_t i = 0;
    while (i < content.size()) {
        std::size_t start = i;
        while (i < content.size() && content[i] != '\n' && content[i] != '\r') ++i;
        std::string text = content.substr(start, i - start);
        std::string eol;
        if (i < content.size()) {
            if (content[i] == '\r' && i + 1 < content.size() && content[i + 1] == '\n') {
                eol = "\r\n";
                i += 2;
            }
            else {
                eol = std::string(1, content[i]);
                ++i;
            }
        }
        lines.push_back({ std::move(text), std::move(eol) });
    }
    return lines;
}

static std::string join_with_line_endings(const std::vector<std::string>& parts,
    const std::vector<std::string>& eols) {
    std::string out;
    for (std::size_t i = 0; i < parts.size(); ++i) {
        out += parts[i];
        if (i < eols.size()) out += eols[i];
    }
    return out;
}

static std::string make_inline_block(const std::string& before, std::string comment_text) {
    comment_text = trim_copy(comment_text);
    if (comment_text.empty()) {
        return before + "/* */";
    }
    return before + "/* " + comment_text + " */";
}

static std::string make_single_line_block(const std::string& indent, std::string comment_text) {
    comment_text = trim_copy(comment_text);
    capitalize_first_alpha(comment_text);
    if (comment_text.empty()) {
        return indent + "/* */";
    }
    return indent + "/* " + comment_text + " */";
}

static std::string make_multi_line_block(const std::string& indent,
    std::vector<std::string> comment_lines) {
    if (!comment_lines.empty()) {
        capitalize_first_alpha(comment_lines.front());
    }

    std::string out;
    out += indent + "/*";
    for (const auto& line : comment_lines) {
        out += "\n" + indent + " *";
        if (!line.empty()) {
            out += " " + line;
        }
    }
    out += "\n" + indent + " */";
    return out;
}

static bool rewrite_file(const fs::path& input_path) {
    std::ifstream in(input_path, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Could not open input file.");
    }

    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    // in.close(); 

    auto lines = split_preserve_newlines(content);

    ParseState state;
    std::vector<std::string> out_lines;
    std::vector<std::string> out_eols;

    for (std::size_t i = 0; i < lines.size();) {
        ParseState probe_state = state;
        LineInfo info = analyze_line(lines[i].text, probe_state);

        if (info.kind == LineKind::InlineComment && !info.excluded) {
            std::string before = lines[i].text.substr(0, info.comment_pos);
            std::string comment = lines[i].text.substr(info.comment_pos + 2);
            out_lines.push_back(make_inline_block(before, std::move(comment)));
            out_eols.push_back(lines[i].eol);
            analyze_line(lines[i].text, state);
            ++i;
            continue;
        }

        if (info.kind != LineKind::StandaloneComment || info.excluded) {
            analyze_line(lines[i].text, state);
            out_lines.push_back(lines[i].text);
            out_eols.push_back(lines[i].eol);
            ++i;
            continue;
        }

        std::vector<std::string> grouped_comment_lines;
        std::vector<std::size_t> grouped_line_indexes;
        std::string indent = lines[i].text.substr(0, info.comment_pos);

        std::size_t j = i;
        ParseState group_probe_state = state;
        while (j < lines.size()) {
            if (is_blank_line(lines[j].text)) {
                grouped_comment_lines.emplace_back();
                grouped_line_indexes.push_back(j);
                ++j;
                continue;
            }

            ParseState tmp = group_probe_state;
            LineInfo li = analyze_line(lines[j].text, tmp);
            if (li.kind == LineKind::StandaloneComment && !li.excluded) {
                std::string text = trim_copy(lines[j].text.substr(li.comment_pos + 2));
                grouped_comment_lines.push_back(std::move(text));
                grouped_line_indexes.push_back(j);
                ++j;
                continue;
            }
            break;
        }

        if (grouped_comment_lines.size() == 1 && !grouped_line_indexes.empty()) {
            out_lines.push_back(make_single_line_block(indent, grouped_comment_lines.front()));
            out_eols.push_back(lines[grouped_line_indexes.front()].eol);
        }
        else {
            out_lines.push_back(make_multi_line_block(indent, std::move(grouped_comment_lines)));
            out_eols.push_back(lines[grouped_line_indexes.back()].eol);
        }

        for (std::size_t k = i; k < j; ++k) {
            analyze_line(lines[k].text, state);
        }
        i = j;
    }

    std::string output = join_with_line_endings(out_lines, out_eols);

    fs::path temp_path = input_path;
    temp_path += ".tmp";
    {
        std::ofstream out(temp_path, std::ios::binary | std::ios::trunc);
        if (!out) {
            throw std::runtime_error("Could not create temp file.");
        }
        out << output;
    }

    fs::remove(input_path);
    fs::rename(temp_path, input_path);
    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file.cpp>\n";
        return 1;
    }

    try {
        const fs::path input_path = argv[1];
        rewrite_file(input_path);
        std::cout << "done.\n";
        return 0;
    }
    catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }
}