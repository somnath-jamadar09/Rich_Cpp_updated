// Port of rich/style.py (core subset).
// Ported faithfully: bit-per-attribute model, _style_map SGR codes,
// STYLE_ATTRIBUTES word table, Style::parse("bold red on blue"),
// ansi code generation (render). Bit order and SGR codes below are copied
// directly from the real style.py source (bold=0 .. overline=12,
// _style_map = {0:"1",1:"2",...,12:"53"}).
// NOT ported yet: downgrade() dependency on _palettes (color.hpp doesn't
// have downgrade() either, so this generates full truecolor/8bit codes
// regardless of terminal color depth. Fine for modern truecolor terminals.
#pragma once
#include "color.hpp"
#include "errors.hpp"
#include <optional>
#include <string>
#include <unordered_map>
#include <sstream>
#include <cctype>
#include <algorithm>

namespace rich {

class Style {
public:
    Style() = default;

    // Named-argument-style construction via a small builder, since C++ has
    // no keyword arguments like Python's Style(bold=True, color="red").
    Style& set_color(const std::string& c) { color_ = Color::parse(c); return *this; }
    Style& set_bgcolor(const std::string& c) { bgcolor_ = Color::parse(c); return *this; }
    Style& set_bold(bool v = true) { bold_ = v; return *this; }
    Style& set_dim(bool v = true) { dim_ = v; return *this; }
    Style& set_italic(bool v = true) { italic_ = v; return *this; }
    Style& set_underline(bool v = true) { underline_ = v; return *this; }
    Style& set_blink(bool v = true) { blink_ = v; return *this; }
    Style& set_blink2(bool v = true) { blink2_ = v; return *this; }
    Style& set_reverse(bool v = true) { reverse_ = v; return *this; }
    Style& set_conceal(bool v = true) { conceal_ = v; return *this; }
    Style& set_strike(bool v = true) { strike_ = v; return *this; }
    Style& set_underline2(bool v = true) { underline2_ = v; return *this; }
    Style& set_frame(bool v = true) { frame_ = v; return *this; }
    Style& set_encircle(bool v = true) { encircle_ = v; return *this; }
    Style& set_overline(bool v = true) { overline_ = v; return *this; }

    /// Parse a style definition string, e.g. "bold red on blue", "italic underline u",
    /// "not bold", "b white on #030712".
    static Style parse(const std::string& style_definition) {
        std::string trimmed = style_definition;
        auto not_space = [](unsigned char c) { return !std::isspace(c); };
        trimmed.erase(trimmed.begin(), std::find_if(trimmed.begin(), trimmed.end(), not_space));
        trimmed.erase(std::find_if(trimmed.rbegin(), trimmed.rend(), not_space).base(), trimmed.end());
        if (trimmed.empty() || trimmed == "none") return Style{};

        static const std::unordered_map<std::string, std::string> kAttrWords = {
            {"dim", "dim"}, {"d", "dim"}, {"bold", "bold"}, {"b", "bold"},
            {"italic", "italic"}, {"i", "italic"}, {"underline", "underline"}, {"u", "underline"},
            {"blink", "blink"}, {"blink2", "blink2"}, {"reverse", "reverse"}, {"r", "reverse"},
            {"conceal", "conceal"}, {"c", "conceal"}, {"strike", "strike"}, {"s", "strike"},
            {"underline2", "underline2"}, {"uu", "underline2"}, {"frame", "frame"},
            {"encircle", "encircle"}, {"overline", "overline"}, {"o", "overline"},
        };

        Style style;
        std::istringstream ss(trimmed);
        std::string original_word;
        std::vector<std::string> words;
        while (ss >> original_word) words.push_back(original_word);

        for (size_t i = 0; i < words.size(); ++i) {
            std::string word = words[i];
            std::transform(word.begin(), word.end(), word.begin(),
                            [](unsigned char c) { return std::tolower(c); });

            if (word == "on") {
                if (++i >= words.size()) throw StyleSyntaxError("color expected after 'on'");
                try {
                    style.bgcolor_ = Color::parse(words[i]);
                } catch (ColorParseError& e) {
                    throw StyleSyntaxError("unable to parse '" + words[i] +
                                            "' as background color; " + e.what());
                }
            } else if (word == "not") {
                if (++i >= words.size()) throw StyleSyntaxError("expected style attribute after 'not'");
                std::string w = words[i];
                std::transform(w.begin(), w.end(), w.begin(), [](unsigned char c) { return std::tolower(c); });
                auto it = kAttrWords.find(w);
                if (it == kAttrWords.end())
                    throw StyleSyntaxError("expected style attribute after 'not', found '" + words[i] + "'");
                style.apply_attr(it->second, false);
            } else if (word == "link") {
                if (++i >= words.size()) throw StyleSyntaxError("URL expected after 'link'");
                style.link_ = words[i];
            } else if (kAttrWords.count(word)) {
                style.apply_attr(kAttrWords.at(word), true);
            } else {
                try {
                    style.color_ = Color::parse(word);
                } catch (ColorParseError& e) {
                    throw StyleSyntaxError("unable to parse '" + word + "' as color; " + e.what());
                }
            }
        }
        return style;
    }

    /// Combine this style with another; `other`'s explicit attributes win.
    Style operator+(const Style& other) const {
        Style result = *this;
        if (other.color_) result.color_ = other.color_;
        if (other.bgcolor_) result.bgcolor_ = other.bgcolor_;
        if (other.bold_) result.bold_ = other.bold_;
        if (other.dim_) result.dim_ = other.dim_;
        if (other.italic_) result.italic_ = other.italic_;
        if (other.underline_) result.underline_ = other.underline_;
        if (other.blink_) result.blink_ = other.blink_;
        if (other.blink2_) result.blink2_ = other.blink2_;
        if (other.reverse_) result.reverse_ = other.reverse_;
        if (other.conceal_) result.conceal_ = other.conceal_;
        if (other.strike_) result.strike_ = other.strike_;
        if (other.underline2_) result.underline2_ = other.underline2_;
        if (other.frame_) result.frame_ = other.frame_;
        if (other.encircle_) result.encircle_ = other.encircle_;
        if (other.overline_) result.overline_ = other.overline_;
        if (other.link_) result.link_ = other.link_;
        return result;
    }

    /// Generate the SGR (Select Graphic Rendition) code string, e.g. "1;38;2;255;0;0".
    /// Empty string means "no styling to apply".
    std::string ansi_codes() const {
        std::vector<std::string> sgr;
        static const char* style_map[13] = {"1", "2", "3", "4", "5", "6", "7",
                                             "8", "9", "21", "51", "52", "53"};
        bool flags[13] = {bold_, dim_, italic_, underline_, blink_, blink2_, reverse_,
                           conceal_, strike_, underline2_, frame_, encircle_, overline_};
        for (int bit = 0; bit < 13; ++bit)
            if (flags[bit]) sgr.push_back(style_map[bit]);

        if (color_) {
            for (auto& c : color_->get_ansi_codes(true)) sgr.push_back(c);
        }
        if (bgcolor_) {
            for (auto& c : bgcolor_->get_ansi_codes(false)) sgr.push_back(c);
        }

        std::string out;
        for (size_t i = 0; i < sgr.size(); ++i) {
            if (i) out += ";";
            out += sgr[i];
        }
        return out;
    }

    /// Wrap `text` with this style's ANSI escape codes + reset.
    std::string render(const std::string& text) const {
        std::string codes = ansi_codes();
        if (codes.empty()) return text;
        return "\x1b[" + codes + "m" + text + "\x1b[0m";
    }

    bool empty() const {
        return !color_ && !bgcolor_ && !bold_ && !dim_ && !italic_ && !underline_ &&
               !blink_ && !blink2_ && !reverse_ && !conceal_ && !strike_ &&
               !underline2_ && !frame_ && !encircle_ && !overline_ && !link_;
    }

private:
    void apply_attr(const std::string& name, bool value) {
        if (name == "bold") bold_ = value;
        else if (name == "dim") dim_ = value;
        else if (name == "italic") italic_ = value;
        else if (name == "underline") underline_ = value;
        else if (name == "blink") blink_ = value;
        else if (name == "blink2") blink2_ = value;
        else if (name == "reverse") reverse_ = value;
        else if (name == "conceal") conceal_ = value;
        else if (name == "strike") strike_ = value;
        else if (name == "underline2") underline2_ = value;
        else if (name == "frame") frame_ = value;
        else if (name == "encircle") encircle_ = value;
        else if (name == "overline") overline_ = value;
    }

    std::optional<Color> color_;
    std::optional<Color> bgcolor_;
    bool bold_ = false, dim_ = false, italic_ = false, underline_ = false;
    bool blink_ = false, blink2_ = false, reverse_ = false, conceal_ = false;
    bool strike_ = false, underline2_ = false, frame_ = false, encircle_ = false;
    bool overline_ = false;
    std::optional<std::string> link_;
};

} // namespace rich
