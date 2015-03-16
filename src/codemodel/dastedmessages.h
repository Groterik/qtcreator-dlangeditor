#ifndef DASTEDMESSAGES_H
#define DASTEDMESSAGES_H

#include <vector>
#include <string>

#include <msgpack.hpp>

namespace Dasted {

const quint8 PROTOCOL_VERSION = 1;

struct DString
{
    std::string impl;
    void msgpack_unpack(const msgpack::object &o)
    {
        impl.clear();
        if (o.type != msgpack::type::NIL) {
            if (o.type != msgpack::type::STR) {
                throw std::runtime_error("bad msgpack type (string is expected)");
            }
            o >> impl;
        }
    }

    template <typename Packer>
    void msgpack_pack(Packer& pk) const
    {
        pk.pack(impl);
    }
};

template <class T>
struct DVector
{
    std::vector<T> impl;
    void msgpack_unpack(const msgpack::object &o)
    {
        impl.clear();
        if (o.type != msgpack::type::NIL) {
            if (o.type != msgpack::type::ARRAY) {
                throw std::runtime_error("bad msgpack type (array is expected)");
            }
            o >> impl;
        }
    }

    template <typename Packer>
    void msgpack_pack(Packer& pk) const
    {
        pk.pack(impl);
    }
};

struct Location
{
    DString filename;
    uint cursor;

    MSGPACK_DEFINE(filename, cursor)
};

typedef unsigned char SymbolType;

struct Symbol
{
    SymbolType type;
    Location location;
    DString name;
    DVector<DString> qualifiers;
    DVector<DString> parameters;
    DVector<DString> templateParameters;
    DString doc;

    MSGPACK_DEFINE(type, location, name, qualifiers, parameters, templateParameters, doc)
};

struct Scope
{
    Symbol name;
    DVector<Symbol> symbols;
    DVector<Scope> children;

    MSGPACK_DEFINE(name, symbols, children)
};

enum MessageType
{
    WRONG_TYPE = 0,
    COMPLETE,
    FIND_DECLARATION,
    ADD_IMPORT_PATHS,
    GET_DOC,
    OUTLINE,
};

template <MessageType T> struct Request;

template <>
struct Request<COMPLETE>
{
    enum {type = COMPLETE};
    DString src;
    uint cursor;

    MSGPACK_DEFINE(src, cursor)
};

template <>
struct Request<FIND_DECLARATION>
{
    enum {type = FIND_DECLARATION};
    DString src;
    uint cursor;

    MSGPACK_DEFINE(src, cursor)
};

template <>
struct Request<ADD_IMPORT_PATHS>
{
    enum {type = ADD_IMPORT_PATHS};
    DVector<DString> paths;

    MSGPACK_DEFINE(paths)
};

template <>
struct Request<GET_DOC>
{
    enum {type = GET_DOC};
    DString src;
    uint cursor;

    MSGPACK_DEFINE(src, cursor)
};

template <>
struct Request<OUTLINE>
{
    enum {type = OUTLINE};
    DString src;

    MSGPACK_DEFINE(src)
};

template <MessageType T> struct Reply;

template <>
struct Reply<COMPLETE>
{
    enum {type = COMPLETE};
    bool calltips;
    DVector<Symbol> symbols;

    MSGPACK_DEFINE(calltips, symbols)
};

template <>
struct Reply<FIND_DECLARATION>
{
    enum {type = FIND_DECLARATION};
    Symbol symbol;

    MSGPACK_DEFINE(symbol)
};


template <>
struct Reply<ADD_IMPORT_PATHS>
{
    enum {type = ADD_IMPORT_PATHS};
    unsigned char payload;

    MSGPACK_DEFINE(payload)
};

template <>
struct Reply<GET_DOC>
{
    enum {type = GET_DOC};
    DVector<Symbol> symbols;

    MSGPACK_DEFINE(symbols)
};

template <>
struct Reply<OUTLINE>
{
    enum {type = OUTLINE};
    Scope global;

    MSGPACK_DEFINE(global)
};

} // namespace Dasted

#endif // DASTEDMESSAGES_H
