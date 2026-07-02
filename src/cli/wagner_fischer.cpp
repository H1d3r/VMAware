#include "wagner_fischer.hpp"

#include <algorithm>
#include <iostream>

// https://en.wikipedia.org/wiki/Wagner%E2%80%93Fischer_algorithm
// this function is used for argument corrections in CLI tools.
// The performance might be questionable, but it's nothing critical.
// It's only used for finding unrecognised arguments anyway, it's
// a debug utility for crashes.
u8 wagner_fischer(const std::string& a_input, const std::string& b_input) {
    std::string a = a_input;
    std::string b = b_input;

    std::size_t a_length = a.length();
    std::size_t b_length = b.length();

    if (a_length > b_length) {
        std::swap(a, b);
        std::swap(a_length, b_length);
    }

    std::vector<std::size_t> curr_row(a_length + 1);
    std::vector<std::size_t> prev_row(a_length + 1);

    for (std::size_t j = 0; j <= a_length; ++j) {
        prev_row[j] = j;
    }

    for (std::size_t i = 1; i <= b_length; ++i) {
        curr_row[0] = i;

        for (std::size_t j = 1; j <= a_length; ++j) {
            const std::size_t add = prev_row[j] + 1;
            const std::size_t del = curr_row[j - 1] + 1;
            std::size_t change = prev_row[j - 1];

            if (a[j - 1] != b[i - 1]) {
                ++change;
            }

            curr_row[j] = std::min({ add, del, change });
        }

        std::swap(prev_row, curr_row);
    }

    if (prev_row[a_length] > std::numeric_limits<u8>::max()) {
        return std::numeric_limits<u8>::max();
    }
    return static_cast<u8>(prev_row[a_length]);
}

std::vector<std::string> suggest(const std::string& misspelled_word, const arg_table& dictionary) {
    std::vector<std::pair<u8, std::string>> candidates;
    candidates.reserve(dictionary.size());

    for (const auto& word : dictionary) {
        const u8 distance = wagner_fischer(word.first, misspelled_word);
        if (distance <= 2) {
            candidates.emplace_back(distance, word.first);
        }
    }

    std::sort(
        candidates.begin(),
        candidates.end(),
        [](const std::pair<u8, std::string>& a, const std::pair<u8, std::string>& b) {
            return a.first < b.first;
        }
    );

    std::vector<std::string> suggestions;
    suggestions.reserve(candidates.size());

    for (const auto& candidate : candidates) {
        suggestions.push_back(candidate.second);
    }

    return suggestions;
}

void manage_output(const std::vector<std::string>& suggestions) {
    if (suggestions.empty()) {
        return;
    }

    std::cerr << "Did you mean: \"";

    for (std::size_t i = 0; i < suggestions.size(); ++i) {
        if (i > 0) {
            std::cerr << ", ";
        }
        std::cerr << bold << suggestions.at(i) << ansi_exit;
    }

    std::cerr << "\"?\n";
}