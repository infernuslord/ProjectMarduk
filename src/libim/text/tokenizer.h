#ifndef LIBIM_TOKENIZER_H
#define LIBIM_TOKENIZER_H
#include "token.h"
#include "../io/stream.h"

#include <functional>
#include <memory>
#include <string>
#include <string_view>

namespace libim::text {

    class Tokenizer
    {
    public:
        Tokenizer(InputStream& s);
        virtual ~Tokenizer();

        // Explicitly delete copy and move ctors
        Tokenizer(const Tokenizer&) = delete;
        Tokenizer(Tokenizer&&) noexcept = delete;
        Tokenizer& operator=(const Tokenizer&) = delete;
        Tokenizer operator=(Tokenizer&&) noexcept = delete;

        Token getToken();
        void getToken(Token& out);

        template <typename T> T getNumber()
        {
            auto tkn = getToken();
            return tkn.getNumber<T>();
        }

        std::string getIdentifier();
        std::string getStringLiteral();
        std::string getSpaceDelimitedString();
        void getSpaceDelimitedString(Token& out);
        void getDelimitedString(Token& out, const std::function<bool(char)>& isDelim);
        void getString(Token& out, std::size_t len);

        void assertIdentifier(std::string_view id);
        void assertPunctuator(std::string_view punc);
        void assertEndOfFile();

        void skipToNextLine();

        void setReportEol(bool report);
        bool reportEol() const;

        const InputStream& istream() const;

    protected:
        Token cachedTkn_;
        class TokenizerPrivate;
        std::unique_ptr<
            TokenizerPrivate
        > tp_;
    };

}
#endif // LIBIM_TOKENIZER_H
